#include "cannyfilter.h"
#include <QDebug>
#include <stdexcept>
#include <opencv2/imgproc.hpp>

using namespace std;

QVideoFilterRunnable* CannyFilter::createFilterRunnable()
{
    return new CannyFilterRunnable{this};
}

CannyFilterRunnable::CannyFilterRunnable(CannyFilter* filter)
    : m_filter{filter}
{
}

QVideoFrame CannyFilterRunnable::run(QVideoFrame* frame, const QVideoSurfaceFormat& surfaceFormat, QVideoFilterRunnable::RunFlags flags)
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

    if (frame->map(QAbstractVideoBuffer::ReadWrite))
    {
        auto data = frame->bits();

        cv::Mat mat;
        cv::Mat grayscale;

        switch (frame->pixelFormat()) {
        case QVideoFrame::Format_RGB32:
            mat = cv::Mat{height, width, CV_8UC4, data};
            cv::cvtColor(mat, grayscale, cv::COLOR_RGBA2GRAY);
            break;

        case QVideoFrame::Format_RGB24:
            mat = cv::Mat{height, width, CV_8UC3, data};
            cv::cvtColor(mat, grayscale, cv::COLOR_RGBA2GRAY);
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

        cv::Canny(grayscale, grayscale, 3 * m_filter->threshold(), m_filter->threshold());

        switch (frame->pixelFormat()) {
        case QVideoFrame::Format_RGB32:
            cv::cvtColor(grayscale, mat, cv::COLOR_GRAY2RGBA);
            break;

        case QVideoFrame::Format_RGB24:
            cv::cvtColor(grayscale, mat, cv::COLOR_GRAY2RGB);
            break;
        }

        frame->unmap();
        return *frame;
    }
    else
    {
        return *frame;
    }
}

bool CannyFilterRunnable::isFrameValid(QVideoFrame* frame) const noexcept
{
    return frame->isValid() && frame->handleType() == QAbstractVideoBuffer::NoHandle;
}
