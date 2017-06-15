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

    SwipeView {
        id: swipeView
        anchors.fill: parent
        currentIndex: tabBar.currentIndex

        Page1 {
            Image {
                id: unwrapped
            }
        }

        Page {
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

                onCalibrationFinished: {

                }
            }

            VideoOutput {
                source: camera
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                anchors.top: parent.top
                width: parent.width / 2

                focus : visible // to receive focus and capture key events when visible
                filters: [calibrationFilter]
            }

            Slider {
                id: thresholdSlider
                value: 127
                from: 0
                to: 255
            }
        }
    }

    footer: TabBar {
        id: tabBar
        currentIndex: swipeView.currentIndex
        TabButton {
            text: qsTr("First")
        }
        TabButton {
            text: qsTr("Second")
        }
    }
}
