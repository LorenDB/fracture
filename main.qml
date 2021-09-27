import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.0
import fracture 1.0

ApplicationWindow {
    width: 1000
    height: 1000
    visible: true
    title: "fracture"

    header: ToolBar {
        Label {
            anchors.centerIn: parent
            text: "fracture"
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        ButtonGroup {
            buttons: fractalTypeButtons.children
            exclusive: true
        }

        ColumnLayout {
            id: fractalTypeButtons

            spacing: 10

            RadioButton {
                text: qsTr("Mandelbrot")
                onClicked: fractalView.type = FractalView.Mandelbrot
                checked: fractalView.type === FractalView.Mandelbrot
            }

            RadioButton {
                text: qsTr("Julia")
                onClicked: fractalView.type = FractalView.Julia
                checked: fractalView.type === FractalView.Julia
            }

            RadioButton {
                text: qsTr("Burning ship")
                onClicked: fractalView.type = FractalView.BurningShip
                checked: fractalView.type === FractalView.BurningShip
            }
        }

        FractalView {
            id: fractalView

            Layout.fillWidth: true
            Layout.fillHeight: true
            focus: true

            Shortcut {
                sequence: "A"
                onActivated: fractalView.zoomIn()
            }

            Shortcut {
                sequence: "Z"
                onActivated: fractalView.zoomOut()
            }

            Shortcut {
                sequence: "F5"
                onActivated: fractalView.scheduleRender()
            }

            Shortcut {
                sequence: "Return"
                onActivated: {
                    fractalView.zoomInTo(zoomBox.zoomFactor)
                    zoomBox.zoomFactor = 1
                }
            }

            Shortcut {
                sequence: "Ctrl+Return"
                onActivated: {
                    fractalView.zoomOutTo(zoomBox.zoomFactor)
                    zoomBox.zoomFactor = 1
                }
            }

            Shortcut {
                sequence: "PgUp"
                onActivated: zoomBox.zoomFactor -= 0.05
            }

            Shortcut {
                sequence: "PgDown"
                onActivated: zoomBox.zoomFactor += 0.05
            }

            TapHandler {
                acceptedButtons: Qt.LeftButton
                onTapped: {
                    if (fractalView.type === FractalView.Mandelbrot)
                    {
                        fractalView.juliaPoint = eventPoint.position
                        fractalView.type = FractalView.Julia
                    }
                    else if (fractalView.type === FractalView.Julia)
                        fractalView.type = FractalView.Mandelbrot
                }
            }

            Rectangle {
                id: zoomBox

                property double zoomFactor: 1

                color: "transparent"
                border.color: "white"
                border.width: 1
                radius: 2
                visible: zoomFactor !== 1
                anchors.centerIn: parent
                width: parent.width * zoomFactor
                height: parent.height * zoomFactor
            }

            Rectangle {
                id: loadingBubble

                anchors.centerIn: parent
                width: loadingMsg.implicitWidth + 10
                height: loadingMsg.implicitHeight + 10
                radius: 5
                visible: fractalView.isLoading

                Text {
                    id: loadingMsg

                    anchors.centerIn: parent
                    font.pointSize: 18
                    text: "Loading..."
                    visible: fractalView.isLoading
                }
            }
        }

//        ShaderEffectSource {
//            id: ses

//            samples: 64
//            sourceItem: fractalView
//            sourceRect: Qt.rect(loadingBubble.x, loadingBubble.y, loadingBubble.width, loadingBubble.height)
//            anchors.fill: loadingBubble
//        }

//        GaussianBlur {
//            anchors.fill: loadingBubble
//            source: ses
//            radius: 10
//        }
    }
}
