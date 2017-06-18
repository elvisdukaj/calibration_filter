#pragma once

#include "abstractopencvrunnablefilter.h"
#include <opencv2/core.hpp>

class MarkerDetectorFilter : public QAbstractVideoFilter {
    Q_OBJECT

    Q_PROPERTY(int threshold READ threshold WRITE threshold NOTIFY thresholdChanged)

public:
    QVideoFilterRunnable* createFilterRunnable() override;

    int threshold() const { return m_threshold; }
    void threshold(int thr) { m_threshold = thr; }

signals:
    void thresholdChanged();

private:
    friend class ThresholdFilterRunnable;

private:
    int m_threshold = 128;
};

class MarkerDetectorFilterRunnable : public AbstractVideoFilterRunnable {
public:
    MarkerDetectorFilterRunnable(MarkerDetectorFilter* filter);
    QVideoFrame run(QVideoFrame* input, const QVideoSurfaceFormat &surfaceFormat, RunFlags flags) override;

private:
    MarkerDetectorFilter* m_filter;
};

class Mark {
public:
    Mark(const cv::Mat& image);
    uint64_t id() const;
private:
    uint64_t m_id;
};

class MarksDetector {
public:
    MarksDetector(int threshold, int minContournSize = 30);

    void processFame(cv::Mat& grayscale);
    uint64_t encode() const;

private:
    void binarize(cv::Mat& grayscale);
    void findContours();
    void findCandidates();

private:
    int m_threshold;
    const int m_minCountournSize;
    uint64_t m_id;
    cv::Mat m_binarized;
    std::vector<std::vector<cv::Point>> m_contours;
    void filterContours();
};

