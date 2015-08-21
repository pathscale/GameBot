import QtQuick 2.0

Rectangle {
    property alias fit: label.text
    property int tiles
    property double scale
    property alias xAnchor: footprint.x
    property alias yAnchor: footprint.y
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

    Rectangle {
        id: footprint
        width: matchArea.tiles * 30 * matchArea.scale // 30 ~= tile side length when projected as square
        height: matchArea.tiles * 30 * matchArea.scale
        border.width: 1
        border.color: "blue"
        color: "transparent"
        transformOrigin: Item.TopLeft
        rotation: 45
        transform: Scale { yScale: 0.75 }
    }

    states: State {
        name: "details"
        when: mouseArea.pressed | mouseArea.entered // entered doesn't work with QtQuick 2.0
        PropertyChanges { target: matchArea; color: "black"}
        PropertyChanges { target: label; visible: true}
    }
}

