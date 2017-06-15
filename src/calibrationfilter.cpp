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
        }
    }
    catch(const std::exception& exc)
    {
        qDebug() << exc.what();
    }

    frame->unmap();
    return *frame;
}
