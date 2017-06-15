#pragma once

#include "abstractopencvrunnablefilter.h"
#include <opencv2/core.hpp>
#include <vector>

class CalibrationFilter : public QAbstractVideoFilter {
    Q_OBJECT

    Q_PROPERTY(int threshold READ threshold WRITE threshold NOTIFY thresholdChanged)
    Q_PROPERTY(bool calibrate READ isCalibrated WRITE calibrate NOTIFY calibrationFinished)
    Q_PROPERTY(QImage unwraped READ getUnrwappedImage WRITE setUnrwappedImage    )


public:
    QVideoFilterRunnable* createFilterRunnable() override;

    int threshold() const { return m_threshold; }
    void threshold(int thr) { m_threshold = thr; }

    int isCalibrated() const { return m_calibrated; }
    void calibrate(bool cal) { m_calibrated = cal; }

    void setUnrwappedImage(QImage image) { m_unwrappedImage = image.copy(); }
    QImage getUnrwappedImage() const { return m_unwrappedImage; }

signals:
    void thresholdChanged();
    void calibrationFinished();

private:
    friend class ThresholdFilterRunnable;

private:
    int m_threshold = 128;
    bool m_calibrated = false;
    bool m_cornerFound = false;
    QImage m_unwrappedImage;
};

class CalibrationFilterRunnable : public AbstractVideoFilterRunnable {
public:
    CalibrationFilterRunnable(CalibrationFilter* filter);
    QVideoFrame run(QVideoFrame* input, const QVideoSurfaceFormat &surfaceFormat, RunFlags flags) override;

private:
    CalibrationFilter* m_filter;
    std::vector<cv::Point3d> m_wordPoints;
    std::vector<cv::Point2d> m_imagePoints;
    bool m_finished = false;
};
