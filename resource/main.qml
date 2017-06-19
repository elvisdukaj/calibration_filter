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

    CalibrationFilter {
        id: calibrationFilter

        onChessBoardFound: {
            timer.start()
            calibrationFilter.showNegative = true;
        }

        onCalibrationFinished: {
            showUndistorted.enabled = true
        }

        showUnsistorted: showUndistorted.checked
    }

    VideoOutput {
        id: videoOutput
        source: camera
        anchors.fill: parent

        focus : visible // to receive focus and capture key events when visible
        filters: [calibrationFilter]
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

    Switch {
        id: showUndistorted

        anchors.top: parent.top
        anchors.right: parent.right

        text: qsTr("Show Undistorted")
        checked: false
        checkable: true
        enabled: false
    }
}
