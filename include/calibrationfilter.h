#pragma once

#include "abstractopencvrunnablefilter.h"
#include <opencv2/core.hpp>
#include <vector>

class CalibrationFilter : public QAbstractVideoFilter {
    Q_OBJECT

    Q_PROPERTY(QSize chessBoardSize READ chessBoardSize WRITE chessBoardSize)

    Q_PROPERTY(bool chessBoardFound READ isChessBoardFound WRITE setChessBoardFound NOTIFY chessBoardFound)
    Q_PROPERTY(bool calibrated READ isCalibrated WRITE setCalibrated NOTIFY calibrationFinished)

    Q_PROPERTY(bool showNegative READ hasToShowNegative WRITE setShowNegative)
    Q_PROPERTY(bool showUnsistorted READ showUnsistorted WRITE showUnsistorted)

public:
    QVideoFilterRunnable* createFilterRunnable() override;

    QSize chessBoardSize() const noexcept { return m_chessboardSize; }
    void chessBoardSize(const QSize& size) { m_chessboardSize = size; }

    bool isChessBoardFound() const { return m_chessboardFound; }
    void setChessBoardFound(bool f) { m_chessboardFound = f; }

    int isCalibrated() const { return m_calibrated; }
    void setCalibrated(bool cal = true) { m_calibrated = cal; }

    bool hasToShowNegative() const noexcept { return m_showNegative; }
    void setShowNegative(bool s) noexcept { m_showNegative = s; }

    bool showUnsistorted() const noexcept { return m_showUnsistorted; }
    void showUnsistorted(bool show) noexcept { m_showUnsistorted = show; }

signals:
    void chessBoardFound(bool);
    void calibrationFinished();

private:
    friend class ThresholdFilterRunnable;

private:
    int m_threshold = 128;

    QSize m_chessboardSize = QSize{9, 6};

    bool m_calibrated = false;
    bool m_cornerFound = false;
    bool m_chessboardFound = false;
    bool m_showNegative = false;
    bool m_showUnsistorted = true;

    int m_goodFrames = 0;
};

class CameraCalibrator {
public:
    CameraCalibrator(const cv::Size boardSize = cv::Size{9, 6})
        : m_boardSize{boardSize} {}

    std::vector<cv::Point2f> findChessboard(const cv::Mat& mat);
    double calibrate(cv::Size& imageSize);
    void remap(const cv::Mat& image, cv::Mat& outImage);
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
    void showFlipped(QVideoFrame* frame);
    void showNegative(QVideoFrame* frame);
    void showUndistorted(QVideoFrame* frame);
    void acquireFrame(QVideoFrame* frame);
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
