#include "markerdetectorfilter.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
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
        cv::Mat frameMat, grayscale, binarized;
        videoFrameInGrayScaleAndColor(frame, grayscale, frameMat);

        MarksDetector marksDetector(m_filter->threshold());

        cv::flip(grayscale, grayscale, 1);
        marksDetector.processFame(grayscale);

        grayscaleToVideoFrame(frame, binarized, frameMat);
    }
    catch(const std::exception& exc)
    {
        qDebug() << exc.what();
    }

    frame->unmap();


    return *frame;
}

MarksDetector::MarksDetector(int threshold, int minContournSize)
    : m_threshold{threshold}, m_minCountournSize{minContournSize}
{
}

void MarksDetector::processFame(cv::Mat& grayscale)
{
    m_contours.clear();
    binarize(grayscale);
    findContours();
}

void MarksDetector::binarize(cv::Mat &grayscale)
{
    cv::threshold(grayscale, m_binarized, m_threshold, 255.0, cv::THRESH_BINARY);
}

void MarksDetector::findContours()
{
    cv::findContours(m_binarized, m_contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);
    filterContours();

    cv::Mat contousImage(m_binarized.size(), CV_8UC1);
    cv::drawContours(contousImage, m_contours, -1, cv::Scalar{255});
    cv::imshow("Contours", contousImage);
}

void MarksDetector::findCandidates()
{
//    std::vector<
}

void MarksDetector::filterContours()
{
    qDebug() << "First " << m_contours.size();
    auto toRemove = remove_if(begin(m_contours), end(m_contours),
                              [&](const std::vector<cv::Point>& vec)
    {
        return vec.size() > m_minCountournSize;
    }
    );

    m_contours.erase(toRemove, end(m_contours));
    qDebug() << "After" << m_contours.size();
}

Mark::Mark(const cv::Mat& image)
{

}
