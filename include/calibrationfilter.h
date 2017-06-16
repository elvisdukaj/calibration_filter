#pragma once

#include "abstractopencvrunnablefilter.h"
#include <opencv2/core.hpp>
#include <vector>

class CalibrationFilter : public QAbstractVideoFilter {
    Q_OBJECT

    Q_PROPERTY(int chessBoardWidth READ chessBoardWidth WRITE chessBoardWidth)
    Q_PROPERTY(int chessBoardHeight READ chessBoardHeight WRITE chessBoardHeight)

    Q_PROPERTY(int threshold READ threshold WRITE threshold NOTIFY thresholdChanged)

    Q_PROPERTY(bool chessBoardFound READ isChessBoardFound WRITE setChessBoardFound NOTIFY chessBoardFound)
    Q_PROPERTY(bool showNegative READ showNegative WRITE setShowNegative)
    Q_PROPERTY(bool setCalibrated READ isCalibrated WRITE setCalibrated NOTIFY calibrationFinished)

public:
    QVideoFilterRunnable* createFilterRunnable() override;

    int chessBoardWidth() const noexcept{ return m_chessBoardWidth; }
    void chessBoardWidth(int width) noexcept { m_chessBoardWidth = width; }

    int chessBoardHeight() const noexcept { return m_chessBoardHeight; }
    void chessBoardHeight(int height) noexcept { m_chessBoardHeight = height; }

    int threshold() const { return m_threshold; }
    void threshold(int thr) { m_threshold = thr; }

    int isCalibrated() const { return m_calibrated; }
    void setCalibrated(bool cal = true) { m_calibrated = cal; }

    bool isChessBoardFound() const { return m_chessboardFound; }
    void setChessBoardFound(bool f) { m_chessboardFound = f; }

    bool showNegative() const noexcept { return m_showNegative; }
    void setShowNegative(bool s) noexcept { m_showNegative = s; }

signals:
    void thresholdChanged();
    void calibrationFinished();
    void chessBoardFound();

private:
    friend class ThresholdFilterRunnable;

private:
    int m_threshold = 128;

    int m_chessBoardWidth = 11;
    int m_chessBoardHeight = 9;

    bool m_calibrated = false;
    bool m_cornerFound = false;
    bool m_chessboardFound = false;
    bool m_showNegative = false;

    int m_goodFrames = 0;

};

class CameraCalibrator {
public:
    CameraCalibrator(const cv::Size boardSize = cv::Size{9, 6})
        : m_boardSize{boardSize} {}

    std::vector<cv::Point2f> findChessboard(const cv::Mat& mat);
    double calibrate(cv::Size& imageSize);
    cv::Mat remap(const cv::Mat& image);
    const cv::Size boardSize() const noexcept { return m_boardSize; }

private:
    void addPoints(const std::vector<cv::Point2f>& imageCorners);

private:
    cv::Size m_boardSize;

    std::vector<std::vector<cv::Point3f>> m_worldObjectPoints;
    std::vector<std::vector<cv::Point2f>> m_imagePoints;

    cv::Mat m_foundChessBoard;

    cv::Mat m_cameraMatrix;
    cv::Mat m_distCoeffs;
    std::vector<cv::Mat> m_rotationVecs;
    std::vector<cv::Mat> m_translationtVecs;
    cv::Mat m_mapX, m_mapY;
    bool m_mustInitUndistort;
};

class CalibrationFilterRunnable : public AbstractVideoFilterRunnable {
public:
    CalibrationFilterRunnable(CalibrationFilter* filter);
    QVideoFrame run(QVideoFrame* input, const QVideoSurfaceFormat &surfaceFormat, RunFlags flags) override;

private:
    void convertToCvMat(QVideoFrame* frame, cv::Mat& mat);

private:
    CalibrationFilter* m_filter;
    std::vector<cv::Point3d> m_wordPoints;
    std::vector<cv::Point2d> m_imagePoints;
    cv::Mat m_lastFrameWithChessBoard;
    bool m_finished = false;
    CameraCalibrator m_calibrator;
    int m_goodFrames = 0;
    cv::Mat m_unwrapped;
};
