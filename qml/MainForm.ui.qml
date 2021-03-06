import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0

Item {
    id: item1
    width: 640
    height: 480

    property alias preview: preview
    property alias debug_layer: debug_layer
    property alias heatmap_layer: heatmap_layer
    property alias matches_layer: matches_layer
    Column {
        RowLayout {
            width: 350
            height: 30
            CheckBox {
                id: matches
                checked: true
                text: "Show matches"
            }
            CheckBox {
                id: heatmap
                checked: true
                text: "Show range"
            }
            CheckBox {
                id: dmgvalues
                checked: true
                text: "Show damage"
            }

            Button {
                id: ss
                text: "Screenshot"
                onClicked: mainForm.preview.grabToImage(function(result) {
                    result.saveToFile("something.png");
                });
            }
        }

        RowLayout {
            Label {
                text: "DPS under pointer: "
            }
            Label {
                id: damageValue
                text: "0"
            }
        }
    }

    ScrollView {
        anchors.rightMargin: 0
        anchors.bottomMargin: 0
        anchors.leftMargin: 0
        anchors.topMargin: 87
        anchors.fill: parent
        Image {
            id: preview
            fillMode: Image.Pad
            source: ""
            property bool dmgLabelsVisible: dmgvalues.checked
            Item {
                id: matches_layer
                visible: matches.checked
            }
            Item {
                id: heatmap_layer
                visible: heatmap.checked
            }

            Item {
                Image {
                    id: debug_layer
                    fillMode: Image.Pad
                    source: ""
                    visible: false
                }
                GreyMaskThreshold {
                    id: debug_gray2a
                    anchors.fill: debug_layer
                    src: debug_layer
                }
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onPositionChanged: damageValue.text = bot.getDamageText(mouse.x, mouse.y)
            }
        }
    }

    Item {
        id: debug_ctrls
        SpinBox {
            id: thr_spin
            x: 267
            y: 53
            width: 64
            height: 24
            value: 256
            maximumValue: 256
            onValueChanged: debug_gray2a.thr = value / maximumValue
        }

        Label {
            id: thr_label
            x: 353
            y: 56
            text: qsTr("min_match * 256")
        }

        Slider {
            id: thr_slider
            x: 8
            y: 54
            width: 243
            height: 22
            value: thr_spin.value
            maximumValue: thr_spin.maximumValue
            onValueChanged: thr_spin.value = value;
        }
    }
}
