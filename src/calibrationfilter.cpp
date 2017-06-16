#include "calibrationfilter.h"
#include <opencv2/core.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>
#include <QDebug>
#include <stdexcept>
using namespace std;

QVideoFilterRunnable* CalibrationFilter::createFilterRunnable()
{
    return new CalibrationFilterRunnable(this);
}

CalibrationFilterRunnable::CalibrationFilterRunnable(CalibrationFilter* filter)
    : m_filter{filter}
{
}

QVideoFrame CalibrationFilterRunnable::run(QVideoFrame* frame, const QVideoSurfaceFormat&, QVideoFilterRunnable::RunFlags)
{
    if (!isFrameValid(frame))
    {
        qDebug() << "Frame is NOT valid";
        return QVideoFrame{};
    }

    if (!frame->map(QAbstractVideoBuffer::ReadWrite))
    {
        qDebug() << "Unable to map the videoframe in memory" << endl;
        return *frame;
    }

    try
    {
        cv::Mat frameMat, grayscale;

        if (m_filter->showNegative())
        {
            cv::Mat frameMat;
            convertToCvMat(frame, frameMat);
            m_lastFrameWithChessBoard.copyTo(frameMat);
        }
        else
        {
            videoframeToGrayscale(frame, grayscale, frameMat);

            cv::flip(grayscale, grayscale, 1);
            cv::threshold(grayscale, grayscale, m_filter->threshold(), 255.0, cv::THRESH_BINARY);

            vector<cv::Point2f> corners;
            auto res = cv::findChessboardCorners(grayscale, cv::Size(9, 6), corners);

            grayscaleToVideoFrame(frame, grayscale, frameMat);

            if (res)
            {
                cv::cornerSubPix(grayscale, corners, cv::Size(11, 11), cv::Size(-1, -1),
                                 cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));

                cv::drawChessboardCorners(frameMat, cv::Size(9, 6), cv::Mat(corners), res);
                m_lastFrameWithChessBoard = frameMat.clone();
                m_filter->setChessBoardFound(true);
                emit m_filter->chessBoardFound();
            }
        }
    }
    catch(const std::exception& exc)
    {
        qDebug() << exc.what();
    }

    frame->unmap();
    return *frame;
}

void CalibrationFilterRunnable::convertToCvMat(QVideoFrame *frame, cv::Mat& frameMat)
{
    auto width = frame->width();
    auto height = frame->height();
    auto data = frame->bits();

    switch (frame->pixelFormat()) {
    case QVideoFrame::Format_RGB32:
        frameMat = cv::Mat{height, width, CV_8UC4, data};
        return;

    case QVideoFrame::Format_RGB24:
        frameMat = cv::Mat{height, width, CV_8UC3, data};
        return;

    case QVideoFrame::Format_YUV420P:
        frameMat = cv::Mat{height, width, CV_8UC1, data};
        return;

    default:
        throw std::runtime_error{"Unknown video frame type"};
    }
}

CameraCalibrator::CameraCalibrator()
    : m_flag(0), m_mustInitUndistort(true)
{
}

void CameraCalibrator::processFrame(cv::Mat& grayscale)
{
    auto corners = findChessboard(grayscale);

    if (!corners.empty())
    {

        addChessboardPoints(grayscale, corners)
    }
}

vector<cv::Point2f> CameraCalibrator::findChessboard(const cv::Mat &mat) const noexcept
{
    vector<cv::Point2f> corners;
    cv::findChessboardCorners(mat, cv::Size(9, 6), corners);
    return corners;
}

bool CameraCalibrator::addChessboardPoints(const cv::Mat& grayscale, const std::vector<cv::Point2f>& corners)
{
    cv::cornerSubPix(grayscale, corners, cv::Size(11, 11), cv::Size(-1, -1),
                     cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
}

int CameraCalibrator::addChessboardPoints(const std::vector<string>& filelist, cv::Size& boardSize)
{
    // the points on the chessboard
    std::vector<cv::Point2f> imageCorners;
    std::vector<cv::Point3f> objectCorners;
    // 3D Scene Points:
    // Initialize the chessboard corners
    // in the chessboard reference frame
    // The corners are at 3D location (X,Y,Z)= (i,j,0)
    for (int i=0; i<boardSize.height; i++) {
        for (int j=0; j<boardSize.width; j++) {
            objectCorners.push_back(cv::Point3f(i, j, 0.0f));
        }
    }
    // 2D Image points:
    cv::Mat image; // to contain chessboard image
    int successes = 0;
    // for all viewpoints
    for (int i=0; i<filelist.size(); i++) {
        // Open the image
        image = cv::imread(filelist[i],0);
        // Get the chessboard corners
        bool found = cv::findChessboardCorners(
                         image, boardSize, imageCorners);
        // Get subpixel accuracy on the corners
        cv::cornerSubPix(image, imageCorners,
                         cv::Size(5,5),
                         cv::Size(-1,-1),
                         cv::TermCriteria(cv::TermCriteria::MAX_ITER +
                                          cv::TermCriteria::EPS,
                                          30, // max number of iterations
                                          0.1)); // min accuracy
        //If we have a good board, add it to our data
        if (imageCorners.size() == boardSize.area()) {
            // Add image and scene points from one view
            addPoints(imageCorners, objectCorners);
            successes++;
        }
    }
    return successes;
}

void CameraCalibrator::CameraCalibrator::addPoints(
        const std::vector<cv::Point2f>& imageCorners,
        const std::vector<cv::Point3f> &objectCorners)
{
    // 2D image points from one view
    m_imagePoints.push_back(imageCorners);
    // corresponding 3D scene points
    m_objectPoints.push_back(objectCorners);
}

double CameraCalibrator::calibrate(cv::Size& imageSize)
{
    // undistorter must be reinitialized
    m_mustInitUndistort= true;
    //Output rotations and translations
    std::vector<cv::Mat> rvecs, tvecs;
    // start calibration
    return
            calibrateCamera(m_objectPoints, // the 3D points
                            m_imagePoints, // the image points
                            imageSize, // image size
                            m_cameraMatrix,// output camera matrix
                            m_distCoeffs, // output distortion matrix
                            rvecs, tvecs,// Rs, Ts
                            m_flag); // set options
}

cv::Mat CameraCalibrator::CameraCalibrator::remap(const cv::Mat& image)
{
    cv::Mat undistorted;
    if (m_mustInitUndistort) { // called once per calibration
        cv::initUndistortRectifyMap(
                    m_cameraMatrix, // computed camera matrix
                    m_distCoeffs, // computed distortion matrix
                    cv::Mat(), // optional rectification (none)
                    cv::Mat(), // camera matrix to generate undistorted
                    image.size(), // size of undistorted
                    CV_32FC1, // type of output map
                    m_map1, m_map2); // the x and y mapping functions
        m_mustInitUndistort= false;
    }
    // Apply mapping functions
    cv::remap(image, undistorted, m_map1, m_map2,
              cv::INTER_LINEAR); // interpolation type
    return undistorted;
}
