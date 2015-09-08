import QtQuick 2.0


Rectangle {
    property double scale: 1 // TODO: make use of this after scaling is supported
    property int xcenter
    property int ycenter
    property int range
    id: defense
    x: xcenter// - 10 // tile height
    y: ycenter// - 15 // tile width
    width: 1
    height: 1
    Rectangle {
        id: range
        width: defense.range * 30 * defense.scale // 30 ~= tile side length when projected as square
        height: width
        x: -height/2
        y: -width/2
        //opacity: 0.1
        color: "transparent"
        border.width: 1
        border.color: "red"
        radius: width/2
        transform: Scale { yScale: 0.75
                           origin.x: 0
                           origin.y: range.height/2
        }
    }
}

