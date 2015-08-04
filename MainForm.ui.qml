import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1

Item {
    width: 640
    height: 480

    property alias button3: button3
    property alias button2: button2
    property alias button1: button1

    RowLayout {
        width: 350
        height: 30
        anchors.verticalCenterOffset: -217
        anchors.horizontalCenterOffset: -137
        anchors.centerIn: parent

        Button {
            id: button1
            text: qsTr("Press Me 1")
        }

        Button {
            id: button2
            text: qsTr("Press Me 2")
        }

        Button {
            id: button3
            text: qsTr("Press Me 3")
        }
    }

    Image {
        id: preview
        x: 8
        y: 44
        width: 624
        height: 428
        source: "qrc:/qtquickplugin/images/template_image.png"
    }

    CheckBox {
        id: damage
        x: 355
        y: 15
        text: qsTr("Show damage")
    }
}
