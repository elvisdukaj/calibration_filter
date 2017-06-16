import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import QtMultimedia 5.5

import com.qubicaamf.vision 1.0

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: qsTr("Hello World")

    Camera {
        id: camera
    }

    ThresholdFilter {
        id: thresholdFilter
        threshold: thresholdSlider.value
    }

    CannyFilter{
        id: cannyFilter
        threshold: thresholdSlider.value
    }

    CalibrationFilter {
        id: calibrationFilter
        threshold: thresholdSlider.value

        onChessBoardFound: {
            timer.start()
            calibrationFilter.showNegative = true;
        }
    }

    Timer {
        id: timer
        interval: 2000
        running: false
        repeat: false

        onTriggered: {
            calibrationFilter.showNegative = false;
        }
    }

    RowLayout {
        VideoOutput {
            id: videoOutput
            source: camera

            focus : visible // to receive focus and capture key events when visible
            filters: [calibrationFilter]
        }

        ColumnLayout {
            anchors.top: parent.top

            RowLayout {
                Label {
                    text: qsTr("Threshold value: ")
                }

                Label {
                    text: Math.floor(thresholdSlider.value)
                }
            }

            Slider {
                id: thresholdSlider

                value: 127
                from: 0
                to: 255
                stepSize: 1
            }
        }
    }

}
