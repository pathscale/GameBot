import QtQuick 2.0


Rectangle {
    property double scale: 1 // TODO: make use of this after scaling is supported
    property int range
    id: defense
    width: 2
    height: 2
    // XXX: is there a way to define the below globally?
    property double horzTile: 42
    property double vertTile: 31.5
    Rectangle {
        id: reach
        width: defense.range * defense.horzTile * defense.scale // 30 ~= tile side length when projected as square
        height: width
        x: -height / 2
        y: -width / 2
        radius: width / 2 // this makes it a circle
        //opacity: 0.1
        color: "transparent"
        border.width: 1
        border.color: "red"
        // this makes an ellipse
        transform: Scale { yScale: defense.vertTile / defense.horzTile
                           origin.x: 0
                           origin.y: reach.height / 2
        }
    }
}

