import QtQuick 2.0

Rectangle {
    property alias fit: label.text
    property int tiles
    property double scale
    property alias xAnchor: footprint.x
    property alias yAnchor: footprint.y
    // XXX: is there a way to define the below globally?
    property double horzTile: 42
    property double vertTile: 31.5
    property double fakeSquare: horzTile * Math.SQRT1_2 // tile side when stretched vertically to square
    id: matchArea
    border.width: 1
    border.color: "red"
    color: "transparent"
    Text {
        id: label
        visible: false
        color: "white"
    }
    Text {
        id: offset
        y: 100
        color: "white"
        visible: false
        text: footprint.x + ',' + footprint.y
        states: State {
            name: "dragged"
            when: mouseArea.pressed
            PropertyChanges { target: offset; visible: true}
        }
    }

    MouseArea {
        id: mouseArea
        hoverEnabled: true
        anchors.fill: parent
        property int dragStartX
        property int dragStartY
        onPressed: {
            dragStartX = mouse.x;
            dragStartY = mouse.y;
        }
        onPositionChanged: {
            if (pressed) {
                footprint.x = mouse.x - dragStartX;
                footprint.y = mouse.y - dragStartY;
            }
        }
    }

    Rectangle {
        id: footprint
        width: matchArea.tiles * fakeSquare * matchArea.scale // 30 ~= tile side length when projected as square
        height: matchArea.tiles * fakeSquare * matchArea.scale
        border.width: 1
        border.color: "blue"
        color: "transparent"
        transformOrigin: Item.TopLeft
        rotation: 45
        transform: Scale { yScale: vertTile / horzTile }
    }

    states: State {
        name: "details"
        when: mouseArea.pressed | mouseArea.entered // entered doesn't work with QtQuick 2.0
        PropertyChanges { target: matchArea; color: "black"}
        PropertyChanges { target: label; visible: true}
    }
}
