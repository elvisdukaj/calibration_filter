#include "thresholdfilter.h"
#include <QDebug>
#include <stdexcept>
#include <opencv2/imgproc.hpp>

using namespace std;

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
        qDebug() << "Frame is NOT valid";
        return QVideoFrame{};
    }

    auto width = frame->width();
    auto height = frame->height();

    cv::Mat mat;
    cv::Mat grayscale;

    if (frame->map(QAbstractVideoBuffer::ReadWrite))
    {
        auto data = frame->bits();

        switch (frame->pixelFormat()) {
        case QVideoFrame::Format_RGB32:
            mat = cv::Mat{height, width, CV_8UC4, data};
            cv::cvtColor(mat, grayscale, cv::COLOR_RGBA2GRAY);
            break;

        case QVideoFrame::Format_RGB24:
            mat = cv::Mat{height, width, CV_8UC3, data};
            cv::cvtColor(mat, grayscale, cv::COLOR_RGB2GRAY);
            break;

        case QVideoFrame::Format_YUV420P:
            grayscale = cv::Mat{height, width, CV_8UC1, data};
            fill(data + (width * height), data + frame->mappedBytes(), 127);
            break;

        default:
            qDebug() << "Unknown format";
            frame->unmap();
            return *frame;
        }

        cv::flip(grayscale, grayscale, 1);
        cv::threshold(grayscale, grayscale, m_filter->threshold(), 255.0, cv::THRESH_BINARY);

        switch (frame->pixelFormat()) {
        case QVideoFrame::Format_RGB32:
            cv::cvtColor(grayscale, mat, cv::COLOR_GRAY2RGBA);
            break;

        case QVideoFrame::Format_RGB24:
            cv::cvtColor(grayscale, mat, cv::COLOR_GRAY2RGB);
            break;

        case QVideoFrame::Format_YUV420P:
            // nothing to do
            break;
        }

        frame->unmap();
        return *frame;
    }

    return *frame;
}

bool ThresholdFilterRunnable::isFrameValid(QVideoFrame* frame) const noexcept
{
    return frame->isValid() && frame->handleType() == QAbstractVideoBuffer::NoHandle;
}
