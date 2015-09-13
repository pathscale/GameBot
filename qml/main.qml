import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2

import levelme 1.0

ApplicationWindow {
    title: qsTr("Hello World")
    width: 640
    height: 480
    visible: true

    CocBot {
        id: bot
        function copyQML(objects, destination, componentName, propNames) {
            for (var i = 0; i < objects.length; i++) {
                var o = objects[i];
                var component = Qt.createComponent(componentName);
                if (component.status == Component.Ready) {
                    var props = {}
                    for (var y = 0; y < propNames.length; y++) {
                        var propName = propNames[y];
                        props[propName] = o[propName];
                    }
                    var dynamicObject = component.createObject(destination, props);
                    if (dynamicObject == null) {
                        console.log("error creating block ");
                        console.log(component.errorString());
                        return
                    }
                } else {
                    console.log("error loading block component");
                    console.log(component.errorString());
                    return
                }
            }
        }

        onMatchesChanged: {
            console.log("Matches changed (FIXME: destroy objects)");
            copyQML(dmg, mainForm.matches_layer, "MatchBox.qml", ["x", "y", "width", "height", "fit", "scale", "tiles", "xAnchor", "yAnchor"]);
        }
        onHeatmapChanged: {
            console.log("Defense changed (FIXME: destroy objects)");
            copyQML(dmg, mainForm.heatmap_layer, "Defense.qml", ["x", "y", "scale", "range", "dmgType", "dmgValue", "targets"]);
        }

        onDebugChanged: {
            console.log("debug changed:" + url);
            mainForm.debug_layer.source = url;
        }
    }

    menuBar: MenuBar {
        Menu {
            title: qsTr("&File")
            MenuItem {
                text: qsTr("&Open")
                onTriggered: fileDialog.show(qsTr("Please choose a screenshot"));
            }
            MenuItem {
                text: qsTr("E&xit")
                onTriggered: Qt.quit();
            }
        }
    }

    MainForm {
        id: mainForm;
        z: 0
        anchors.rightMargin: 0
        anchors.bottomMargin: 0
        anchors.leftMargin: 0
        anchors.topMargin: 0
        anchors.fill: parent
    }

    FileDialog {
        id: fileDialog
        folder: shortcuts.home
        nameFilters: [ "Image files (*.jpg *.png)", "All files (*)" ]
        function show(caption) {
            fileDialog.title = caption;
            fileDialog.visible = true;
        }
        onAccepted: {
            console.log("loading " + fileDialog.fileUrl)
            mainForm.preview.source = fileDialog.fileUrl;
            bot.loadUrl(fileDialog.fileUrl)
        }
    }
}
