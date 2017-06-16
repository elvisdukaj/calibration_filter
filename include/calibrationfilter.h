#pragma once

#include "abstractopencvrunnablefilter.h"
#include <opencv2/core.hpp>
#include <vector>

class CalibrationFilter : public QAbstractVideoFilter {
    Q_OBJECT

    Q_PROPERTY(int threshold READ threshold WRITE threshold NOTIFY thresholdChanged)
    Q_PROPERTY(bool chessBoardFound READ isChessBoardFound WRITE setChessBoardFound NOTIFY chessBoardFound)
    Q_PROPERTY(bool showNegative READ showNegative WRITE setShowNegative)
    Q_PROPERTY(bool calibrate READ isCalibrated WRITE calibrate NOTIFY calibrationFinished)
    Q_PROPERTY(QImage unwraped READ getUnrwappedImage WRITE setUnrwappedImage    )


public:
    QVideoFilterRunnable* createFilterRunnable() override;

    int threshold() const { return m_threshold; }
    void threshold(int thr) { m_threshold = thr; }

    int isCalibrated() const { return m_calibrated; }
    void calibrate(bool cal) { m_calibrated = cal; }

    bool isChessBoardFound() const { return m_chessboardFound; }
    void setChessBoardFound(bool f) { m_chessboardFound = f; }

    void setUnrwappedImage(QImage image) { m_unwrappedImage = image.copy(); }
    QImage getUnrwappedImage() const { return m_unwrappedImage; }

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
    bool m_calibrated = false;
    bool m_cornerFound = false;
    bool m_chessboardFound = false;
    bool m_showNegative = false;
    QImage m_unwrappedImage;
};



class CameraCalibrator {
public:
    CameraCalibrator();

    void processFrame(cv::Mat& grayscale);

private:
    std::vector<cv::Point2f> findChessboard(const cv::Mat& mat) const noexcept;

    // Open chessboard images and extract corner points
    bool addChessboardPoints(const cv::Mat& grayscale, const std::vector<cv::Point2f>& corners );
    bool addChessboardPoints(const std::vector<std::string>& filelist, cv::Size& boardSize);

    // Add scene points and corresponding image points
    void addPoints(const std::vector<cv::Point2f>& imageCorners, const std::vector<cv::Point3f>& objectCorners);

    // Calibrate the camera
    // returns the re-projection error
    double calibrate(cv::Size& imageSize);

    // remove distortion in an image (after calibration)
    cv::Mat remap(const cv::Mat& image);

private:
    // input points:
    // the points in world coordinates
    std::vector<std::vector<cv::Point3f>> m_objectPoints;
    // the point positions in pixels
    std::vector<std::vector<cv::Point2f>> m_imagePoints;
    // output Matrices
    cv::Mat m_cameraMatrix;
    cv::Mat m_distCoeffs;
    // flag to specify how calibration is done
    int m_flag;
    // used in image undistortion
    cv::Mat m_map1,m_map2;
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
};
