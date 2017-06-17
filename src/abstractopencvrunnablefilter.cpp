#include "abstractopencvrunnablefilter.h"
#include <opencv2/imgproc.hpp>
#include <QDebug>
#include <stdexcept>
using namespace std;

bool AbstractVideoFilterRunnable::isFrameValid(QVideoFrame* frame) const noexcept
{
    return frame->isValid() && frame->handleType() == QAbstractVideoBuffer::NoHandle;
}

void AbstractVideoFilterRunnable::videoFrameInGrayScaleAndColor(QVideoFrame* frame, cv::Mat& grayscale, cv::Mat& frameMat)
{
    auto width = frame->width();
    auto height = frame->height();
    auto data = frame->bits();

    switch (frame->pixelFormat()) {
    case QVideoFrame::Format_RGB32:
        frameMat = cv::Mat{height, width, CV_8UC4, data};
        cv::cvtColor(frameMat, grayscale, cv::COLOR_RGBA2GRAY);
        return;

    case QVideoFrame::Format_RGB24:
        frameMat = cv::Mat{height, width, CV_8UC3, data};
        cv::cvtColor(frameMat, grayscale, cv::COLOR_RGB2GRAY);
        return;

    case QVideoFrame::Format_YUV420P:
        frameMat = cv::Mat{height, width, CV_8UC1, data};
        grayscale = cv::Mat{height, width, CV_8UC1, data};
        fill(data + (width * height), data + frame->mappedBytes(), 127);
        return;

    default:
        throw std::runtime_error{"Unknown video frame type"};
    }
}

void AbstractVideoFilterRunnable::grayscaleToVideoFrame(QVideoFrame *frame, const cv::Mat& grayscale, cv::Mat& frameMat) const
{
    switch (frame->pixelFormat()) {
    case QVideoFrame::Format_RGB32:
        cv::cvtColor(grayscale, frameMat, cv::COLOR_GRAY2RGBA);
        break;

    case QVideoFrame::Format_RGB24:
        cv::cvtColor(grayscale, frameMat, cv::COLOR_GRAY2RGB);
        break;

    case QVideoFrame::Format_YUV420P:
        // nothing to do
        break;
    }
}
