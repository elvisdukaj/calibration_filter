import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

Item {
    property alias textField1: textField1
    property alias button1: button1

    RowLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 20
        anchors.top: parent.top

        TextField {
            id: textField1
            placeholderText: qsTr("Text Field")
        }

        Button {
            id: button1
            text: qsTr("Press Me")
        }
    }

    Slider {
        id: slider
        x: 157
        y: 207
        stepSize: 1
        value: 0.5
    }

    Switch {
        id: switch1
        x: 204
        y: 301
        text: qsTr("Show Undistort")
        autoExclusive: false
        checkable: false
        checked: true
    }
}
