#pragma once

#include <QAbstractVideoFilter>
#include <opencv2/core.hpp>

class ThresholdFilter : public QAbstractVideoFilter {
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

class ThresholdFilterRunnable : public QVideoFilterRunnable {
public:
    ThresholdFilterRunnable(ThresholdFilter* filter);
    QVideoFrame run(QVideoFrame* input, const QVideoSurfaceFormat &surfaceFormat, RunFlags flags) override;

private:
    bool isFrameValid(QVideoFrame* frame) const noexcept;

private:
    ThresholdFilter* m_filter;
};
