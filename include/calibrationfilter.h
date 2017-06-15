#pragma once

#include "abstractopencvrunnablefilter.h"
#include <opencv2/core.hpp>

class CalibrationFilter : public QAbstractVideoFilter {
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

class CalibrationFilterRunnable : public AbstractVideoFilterRunnable {
public:
    CalibrationFilterRunnable(CalibrationFilter* filter);
    QVideoFrame run(QVideoFrame* input, const QVideoSurfaceFormat &surfaceFormat, RunFlags flags) override;

private:
    CalibrationFilter* m_filter;
};
