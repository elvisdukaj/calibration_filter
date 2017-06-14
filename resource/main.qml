import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import QtMultimedia 5.5

import qubicaamf.vision 1.0

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
        }

        Page {
            Camera {
                id: camera

                imageCapture {
                    onImageCaptured: {
                        photoPreview.source = preview
                    }
                }
            }

            ThresholdFilter {
                id: thresholdFilter
                threshold: thresholdSlider.value
            }

            VideoOutput {
                source: camera
                anchors.fill: parent
<<<<<<< HEAD
                focus : visible // to receive focus and capture key events when visible
                filters: [thresholdFilter]
=======
                focus: visible
                filters: [filter]
>>>>>>> 01233423ed3355f95c1253afa0a27309744ef0bc
            }

            Image {
                id: photoPreview
            }

            Slider {
                id: thresholdSlider
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
