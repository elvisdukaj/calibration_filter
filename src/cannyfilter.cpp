#include "cannyfilter.h"
#include <opencv2/imgproc.hpp>
#include <QDebug>
#include <stdexcept>
using namespace std;

QVideoFilterRunnable* CannyFilter::createFilterRunnable()
{
    return new CannyFilterRunnable{this};
}

CannyFilterRunnable::CannyFilterRunnable(CannyFilter* filter)
    : m_filter{filter}
{
}

QVideoFrame CannyFilterRunnable::run(QVideoFrame* frame, const QVideoSurfaceFormat&, QVideoFilterRunnable::RunFlags)
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

        cv::Canny(grayscale, grayscale, 3 * m_filter->threshold(), m_filter->threshold());

        grayscaleToVideoFrame(frame, grayscale, frameMat);
    }
    catch(const std::exception& exc)
    {
        qDebug() << exc.what();
    }

    frame->unmap();

    return *frame;
}
