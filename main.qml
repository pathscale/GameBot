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
        onHeatmapChanged: {
            console.log("heatmap changed");
            var component = Qt.createComponent("DamageArea.qml");
            if (component.status == Component.Ready) {
                 var dynamicObject = component.createObject(mainForm.preview, {"x": dmg.x,
                                                                               "y": dmg.y,
                                                                               "width": dmg.width,
                                                                               "height": dmg.height});
                 if (dynamicObject == null) {
                     console.log("error creating block");
                     console.log(component.errorString());
                     return
                 }
             } else {
                 console.log("error loading block component");
                 console.log(component.errorString());
                 return
             }
            //mainForm.preview.
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
        button1.onClicked: messageDialog.show(qsTr("Button 1 pressed"))
        button2.onClicked: messageDialog.show(qsTr("Button 2 pressed"))
        button3.onClicked: messageDialog.show(qsTr("Button 3 pressed"))
    }

    MessageDialog {
        id: messageDialog
        title: qsTr("May I have your attention, please?")

        function show(caption) {
            messageDialog.text = caption;
            messageDialog.open();
        }
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
