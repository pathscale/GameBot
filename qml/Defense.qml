import QtQuick 2.0


Rectangle {
    property double scale // TODO: make use of this after scaling is supported
    property alias xAnchor: footprint.x
    property alias yAnchor: footprint.y
    property int range
    id: defense
    color: "red"

    Rectangle {
        id: footprint
        width: defense.range * 30 * defense.scale // 30 ~= tile side length when projected as square
        height: width
        opacity: 0
        color: "red"
        transformOrigin: Item.TopLeft
        rotation: 45
        transform: Scale { yScale: 0.75 }
    }
}

