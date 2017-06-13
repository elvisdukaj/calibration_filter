#include "thresholdfilter.h"
#include <QPixmap>
#include <QDebug>
#include <stdexcept>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

QVideoFilterRunnable* ThresholdFilter::createFilterRunnable()
{
    return new ThresholdFilterRunnable(this);
}

ThresholdFilterRunnable::ThresholdFilterRunnable(ThresholdFilter* filter)
    : m_filter{filter}
{
}

QVideoFrame ThresholdFilterRunnable::run(QVideoFrame* frame, const QVideoSurfaceFormat& surfaceFormat, QVideoFilterRunnable::RunFlags flags)
{
    Q_UNUSED(surfaceFormat);
    Q_UNUSED(flags);

    if (!isFrameValid(frame))
    {
        qDebug() << "frame not valid";
        return *frame;
    }

    QVideoFrame newFrame = *frame;
    auto width = newFrame.width();
    auto height = newFrame.height();

    newFrame.map(QAbstractVideoBuffer::ReadOnly);
    auto data = newFrame.bits();
    std::fill(data + (width * height), data + newFrame.mappedBytes(), 127);
    cv::Mat grayscale(height, width, CV_8UC1, data);
    cv::threshold(grayscale, grayscale, m_filter->threshold(), 255.0, cv::THRESH_BINARY);

    if (!newFrame.isValid())
        qDebug() << "new frame is not valid";

    newFrame.unmap();

    return newFrame;
}

bool ThresholdFilterRunnable::isFrameValid(QVideoFrame* frame) const noexcept
{
    return frame->isValid() && frame->handleType() == QAbstractVideoBuffer::NoHandle
            && frame->pixelFormat() == QVideoFrame::Format_YUV420P;
}
