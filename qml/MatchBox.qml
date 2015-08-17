import QtQuick 2.0

Rectangle {
    property alias fit: label.text
    id: matchArea
    border.width: 1
    border.color: "red"
    color: "transparent"
    Text {
        id: label
        visible: false
        color: "white"
    }
    MouseArea {
        id: mouseArea
        hoverEnabled: true
        anchors.fill: parent
    }

    states: State {
        name: "details"
        when: mouseArea.pressed | mouseArea.entered // entered doesn't work with QtQuick 2.0
        PropertyChanges { target: matchArea; color: "black"}
        PropertyChanges { target: label; visible: true}
    }
}

