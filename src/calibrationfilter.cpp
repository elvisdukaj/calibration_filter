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
            convertToCvMat(frame, frameMat);
            cv::flip(frameMat, frameMat, 1);
            m_lastFrameWithChessBoard.copyTo(
                        frameMat(cv::Rect{
                                     0, 0,
                                     m_lastFrameWithChessBoard.cols,
                                     m_lastFrameWithChessBoard.rows
                                 }
                                 )
                        );
        }
        else if(m_filter->isCalibrated())
        {
            convertToCvMat(frame, frameMat);
            m_unwrapped.copyTo(frameMat);
        }
        else
        {
            videoframeToGrayscale(frame, grayscale, frameMat);

            cv::flip(grayscale, grayscale, 1);

            auto corners = m_calibrator.findChessboard(grayscale);

            grayscaleToVideoFrame(frame, grayscale, frameMat);

            if (!corners.empty())
            {
                cv::drawChessboardCorners(
                            frameMat,
                            m_calibrator.boardSize(),
                            corners,
                            true
                            );

                emit m_filter->chessBoardFound();
                cv::bitwise_not(frameMat, m_lastFrameWithChessBoard);
                cv::resize(m_lastFrameWithChessBoard, m_lastFrameWithChessBoard,
                           cv::Size{0, 0}, 0.3, 0.3
                           );

                ++m_goodFrames;
                qDebug() << "Frame number " << m_goodFrames;

                if (m_goodFrames == 15)
                {
                    auto error = m_calibrator.calibrate(grayscale.size());
                    qDebug() << "calibration done! Error: " << error;

                    m_unwrapped = m_calibrator.remap(frameMat);
                    m_unwrapped.copyTo(frameMat);

                    m_filter->setCalibrated();
                    emit m_filter->calibrationFinished();
                }
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

vector<cv::Point2f> CameraCalibrator::findChessboard(const cv::Mat& grayscale)
{
    vector<cv::Point2f> imageCorners;
    if (cv::findChessboardCorners(grayscale, m_boardSize, imageCorners, cv::CALIB_CB_FAST_CHECK))
    {
        cv::cornerSubPix(
                    grayscale, imageCorners, m_boardSize,
                    cv::Size(-1, -1),
                    cv::TermCriteria(
                        CV_TERMCRIT_EPS + CV_TERMCRIT_ITER,
                        300, 0.01)
                    );

        if (imageCorners.size() == m_boardSize.area())
            addPoints(imageCorners);
        else
            imageCorners.clear();
    }

    return imageCorners;
}

void CameraCalibrator::addPoints(const std::vector<cv::Point2f> &imageCorners)
{
    vector<cv::Point3f> objectCorners;

    for (auto y = 0; y < m_boardSize.height; ++y)
        for (auto x = 0; x < m_boardSize.width; ++x)
            objectCorners.push_back(cv::Point3f(y, x, 0.0f));

    m_worldObjectPoints.push_back(objectCorners);
    m_imagePoints.push_back(imageCorners);
}


double CameraCalibrator::calibrate(cv::Size& imageSize)
{
    m_mustInitUndistort = true;

    // start calibration
    return cv::calibrateCamera(
                m_worldObjectPoints,
                m_imagePoints,
                imageSize,
                m_cameraMatrix,
                m_distCoeffs,
                m_rotationVecs,
                m_translationtVecs
                );
}

cv::Mat CameraCalibrator::CameraCalibrator::remap(const cv::Mat& image)
{
    cv::Mat undistorted;
    if (m_mustInitUndistort) { // called once per calibration
        cv::undistort(image, undistorted, m_cameraMatrix, m_distCoeffs);

//        cv::initUndistortRectifyMap(
//                    m_cameraMatrix, // computed camera matrix
//                    m_distCoeffs,   // computed distortion matrix
//                    cv::Mat(),      // optional rectification (none)
//                    cv::Mat(),      // camera matrix to generate undistorted
//                    image.size(),   // size of undistorted
//                    CV_32FC1,       // type of output map
//                    m_mapX, m_mapY  // the x and y mapping functions
//                    );
        m_mustInitUndistort= false;
    }

    // Apply mapping functions
//    cv::remap(image, undistorted, m_mapX, m_mapY, cv::INTER_LINEAR);
    return undistorted;
}


//int CameraCalibrator::addChessboardPoints(const std::vector<std::string>& filelist,
//                                          cv::Size & boardSize) {
//    // the points on the chessboard
//    std::vector<cv::Point2f> imageCorners;
//    std::vector<cv::Point3f> objectCorners;
//    // 3D Scene Points:
//    // Initialize the chessboard corners
//    // in the chessboard reference frame
//    // The corners are at 3D location (X,Y,Z)= (i,j,0)
//    for (int i=0; i<boardSize.height; i++) {
//        for (int j=0; j<boardSize.width; j++) {
//            objectCorners.push_back(cv::Point3f(i, j, 0.0f));
//        }
//    }
//    // 2D Image points:
//    cv::Mat image; // to contain chessboard image
//    int successes = 0;
//    // for all viewpoints
//    for (int i=0; i<filelist.size(); i++) {
//        // Open the image
//        image = cv::imread(filelist[i],0);
//        // Get the chessboard corners
//        bool found = cv::findChessboardCorners(
//                    image, boardSize, imageCorners);
//        // Get subpixel accuracy on the corners
//        cv::cornerSubPix(image, imageCorners,
//                         cv::Size(5,5),
//                         cv::Size(-1,-1),
//                         cv::TermCriteria(cv::TermCriteria::MAX_ITER +
//                                          cv::TermCriteria::EPS,
//                                          30, // max number of iterations
//                                          0.1)); // min accuracy
//        //If we have a good board, add it to our data
//        if (imageCorners.size() == boardSize.area()) {
//            // Add image and scene points from one view
//            addPoints(imageCorners, objectCorners);
//            successes++;
//        }
//    }
//    return successes;
//}

//void CameraCalibrator::CameraCalibrator::addPoints(
//        const std::vector<cv::Point2f>& imageCorners,
//        const std::vector<cv::Point3f> &objectCorners)
//{
//    // 2D image points from one view
//    m_imagePoints.push_back(imageCorners);
//    // corresponding 3D scene points
//    m_objectPoints.push_back(objectCorners);
//}

//double CameraCalibrator::calibrate(cv::Size& imageSize)
//{
//    // undistorter must be reinitialized
//    m_mustInitUndistort= true;
//    //Output rotations and translations
//    std::vector<cv::Mat> rvecs, tvecs;
//    // start calibration
//    return
//            calibrateCamera(m_objectPoints, // the 3D points
//                            m_imagePoints, // the image points
//                            imageSize, // image size
//                            m_cameraMatrix,// output camera matrix
//                            m_distCoeffs, // output distortion matrix
//                            rvecs, tvecs,// Rs, Ts
//                            m_flag); // set options
//}
