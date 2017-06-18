#include "markerdetectorfilter.h"
#include <opencv2/imgproc.hpp>
#include <QDebug>
#include <stdexcept>
using namespace std;

QVideoFilterRunnable* MarkerDetectorFilter::createFilterRunnable()
{
    return new MarkerDetectorFilterRunnable(this);
}

MarkerDetectorFilterRunnable::MarkerDetectorFilterRunnable(MarkerDetectorFilter* filter)
    : m_filter{filter}
{
}

QVideoFrame MarkerDetectorFilterRunnable::run(QVideoFrame* frame, const QVideoSurfaceFormat&, QVideoFilterRunnable::RunFlags)
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
        videoFrameInGrayScaleAndColor(frame, grayscale, frameMat);

        cv::flip(grayscale, grayscale, 1);
        cv::threshold(grayscale, grayscale, m_filter->threshold(), 255.0, cv::THRESH_BINARY);

        grayscaleToVideoFrame(frame, grayscale, frameMat);
    }
    catch(const std::exception& exc)
    {
        qDebug() << exc.what();
    }

    frame->unmap();


    return *frame;
}
