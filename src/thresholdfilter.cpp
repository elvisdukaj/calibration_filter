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
    else
    {
        return *frame;
//        frame->map(QAbstractVideoBuffer::ReadOnly);
//        auto data = frame->bits();

//        switch (frame->pixelFormat()) {
//        case QVideoFrame::Format_RGB32:
//            mat = cv::Mat{height, width, CV_8UC4, data};
//            cv::cvtColor(mat, grayscale, cv::COLOR_RGBA2GRAY);
//            break;

//        case QVideoFrame::Format_YUV420P:
//            mat = cv::Mat{height, width, CV_8UC1, data};
//            grayscale = mat.clone();
//            break;

//        default:
//            qDebug() << "Unknown format";
//            frame->unmap();
//            return *frame;
//        }

//        frame->unmap();

//        cv::flip(grayscale, grayscale, 1);
//        cv::threshold(grayscale, grayscale, m_filter->threshold(), 255.0, cv::THRESH_BINARY_INV);

//        cv::cvtColor(grayscale, m_frameResultMat, cv::COLOR_GRAY2RGBA);

//        m_image = QImage{m_frameResultMat.ptr<uchar>(), width, height, QImage::Format_RGB32};
//        m_frameResult = QVideoFrame{m_image};

//        if (!m_frameResult.isValid())
//        {
//            qDebug() << "new frame is not valid";
//            return *frame;
//        }

//        return m_frameResult;
    }
}

bool ThresholdFilterRunnable::isFrameValid(QVideoFrame* frame) const noexcept
{
    return frame->isValid() && frame->handleType() == QAbstractVideoBuffer::NoHandle;
}

#if 0
    switch (frame->handleType()) {
    case QAbstractVideoBuffer::NoHandle: qDebug() << "Handle Type: NoHandle"; break;
    case QAbstractVideoBuffer::GLTextureHandle: qDebug() << "Handle Type: GLTextureHandle"; break;
    case QAbstractVideoBuffer::XvShmImageHandle: qDebug() << "Handle Type: XvShmImageHandle"; break;
    case QAbstractVideoBuffer::CoreImageHandle: qDebug() << "Handle Type: NoHandle"; break;
    case QAbstractVideoBuffer::QPixmapHandle: qDebug() << "Handle Type: QPixmapHandle"; break;
    case QAbstractVideoBuffer::EGLImageHandle: qDebug() << "Handle Type: EGLImageHandle"; break;
    case QAbstractVideoBuffer::UserHandle: qDebug() << "Handle Type: UserHandle"; break;
    }

    switch (frame->pixelFormat()) {
    case QVideoFrame::Format_Invalid: qDebug() << "Pixel Format: Format_Invalid"; break;
    case QVideoFrame::Format_ARGB32: qDebug() << "Pixel Format: Format_ARGB32"; break;
    case QVideoFrame::Format_ARGB32_Premultiplied: qDebug() << "Pixel Format: Format_ARGB32_Premultiplied"; break;
    case QVideoFrame::Format_RGB32: qDebug() << "Pixel Format: Format_RGB32"; break;
    case QVideoFrame::Format_RGB24: qDebug() << "Pixel Format: Format_RGB24"; break;
    case QVideoFrame::Format_RGB565: qDebug() << "Pixel Format: Format_RGB565"; break;
    case QVideoFrame::Format_RGB555: qDebug() << "Pixel Format: Format_RGB555"; break;
    case QVideoFrame::Format_ARGB8565_Premultiplied: qDebug() << "Pixel Format: Format_ARGB8565_Premultiplied"; break;
    case QVideoFrame::Format_BGRA32: qDebug() << "Pixel Format: Format_BGRA32"; break;
    case QVideoFrame::Format_BGRA32_Premultiplied: qDebug() << "Pixel Format: Format_BGRA32_Premultiplied"; break;
    case QVideoFrame::Format_BGR32: qDebug() << "Pixel Format: Format_BGR32"; break;
    case QVideoFrame::Format_BGR24: qDebug() << "Pixel Format: Format_BGR24"; break;
    case QVideoFrame::Format_BGR565: qDebug() << "Pixel Format: Format_BGR565"; break;
    case QVideoFrame::Format_BGR555: qDebug() << "Pixel Format: Format_BGR555"; break;
    case QVideoFrame::Format_BGRA5658_Premultiplied: qDebug() << "Pixel Format: Format_BGRA5658_Premultiplied"; break;
    case QVideoFrame::Format_AYUV444_Premultiplied: qDebug() << "Pixel Format: Format_AYUV444_Premultiplied"; break;
    case QVideoFrame::Format_YUV444: qDebug() << "Pixel Format: Format_YUV444"; break;
    case QVideoFrame::Format_YUV420P: qDebug() << "Pixel Format: Format_YUV420P"; break;
    case QVideoFrame::Format_YV12: qDebug() << "Pixel Format: Format_YV12"; break;
    case QVideoFrame::Format_UYVY: qDebug() << "Pixel Format: Format_UYVY"; break;
    case QVideoFrame::Format_YUYV: qDebug() << "Pixel Format: Format_YUYV"; break;
    case QVideoFrame::Format_NV12: qDebug() << "Pixel Format: Format_NV12"; break;
    case QVideoFrame::Format_NV21: qDebug() << "Pixel Format: Format_NV21"; break;
    case QVideoFrame::Format_IMC1: qDebug() << "Pixel Format: Format_IMC1"; break;
    case QVideoFrame::Format_IMC2: qDebug() << "Pixel Format: Format_IMC2"; break;
    case QVideoFrame::Format_IMC3: qDebug() << "Pixel Format: Format_IMC3"; break;
    case QVideoFrame::Format_IMC4: qDebug() << "Pixel Format: Format_IMC4"; break;
    case QVideoFrame::Format_Y8: qDebug() << "Pixel Format: Format_Y8"; break;
    case QVideoFrame::Format_Y16: qDebug() << "Pixel Format: Format_Y16"; break;
    case QVideoFrame::Format_Jpeg: qDebug() << "Pixel Format: Format_Jpeg"; break;
    case QVideoFrame::Format_CameraRaw: qDebug() << "Pixel Format: Format_CameraRaw"; break;
    case QVideoFrame::Format_AdobeDng: qDebug() << "Pixel Format: Format_AdobeDng"; break;
    }
#endif
