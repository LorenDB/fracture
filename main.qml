import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.0
import fracture 1.0

ApplicationWindow {
    width: 1440
    height: 900
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

        ColumnLayout {
            spacing: 10

            Button {
                text: qsTr("Refresh")
                onClicked: fractalView.rerender()
            }

            Button {
                text: qsTr("Stop")
                onClicked: fractalView.cancelRender()
                enabled: fractalView.isLoading
            }

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
        }

        FractalView {
            id: fractalView

            Layout.fillWidth: true
            Layout.fillHeight: true
            focus: true

            Shortcut {
                sequence: "F5"
                onActivated: fractalView.rerender()
            }

            Shortcut {
                sequence: "Return"
                onActivated: fractalView.applyZoomIn()
            }

            Shortcut {
                sequence: "Ctrl+Return"
                onActivated: fractalView.applyZoomOut()
            }

            Shortcut {
                sequence: "PgUp"
                onActivated: fractalView.zoomFactor -= 0.05
            }

            Shortcut {
                sequence: "PgDown"
                onActivated: fractalView.zoomFactor += 0.05
            }

            Shortcut {
                property double increment: 1 - (fractalView.width - 2) / fractalView.width

                sequence: "Shift+PgUp"
                onActivated: fractalView.zoomFactor -= increment
            }

            Shortcut {
                property double increment: 1 - (fractalView.width - 2) / fractalView.width

                sequence: "Shift+PgDown"
                onActivated: fractalView.zoomFactor += increment
            }

            Shortcut {
                sequence: "Left"
                onActivated: fractalView.xOffset -= 20
            }

            Shortcut {
                sequence: "Right"
                onActivated: fractalView.xOffset += 20
            }

            Shortcut {
                sequence: "Up"
                onActivated: fractalView.yOffset -= 20
            }

            Shortcut {
                sequence: "Down"
                onActivated: fractalView.yOffset += 20
            }

            Shortcut {
                sequence: "Shift+Left"
                onActivated: fractalView.xOffset -= 1
            }

            Shortcut {
                sequence: "Shift+Right"
                onActivated: fractalView.xOffset += 1
            }

            Shortcut {
                sequence: "Shift+Up"
                onActivated: fractalView.yOffset -= 1
            }

            Shortcut {
                sequence: "Shift+Down"
                onActivated: fractalView.yOffset += 1
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

                x: parent.width / 2 - width / 2 + fractalView.xOffset
                y: parent.height / 2 - height / 2 + fractalView.yOffset
                color: "transparent"
                border.color: "white"
                border.width: 1
                radius: 2
                visible: fractalView.zoomFactor !== 1
                width: parent.width * fractalView.zoomFactor
                height: parent.height * fractalView.zoomFactor
            }

            Rectangle {
                id: loadingBubble

                anchors.centerIn: parent
                width: loadingMsg.implicitWidth + 10
                height: loadingMsg.implicitHeight + 10
                radius: 5
                visible: fractalView.isLoading
                opacity: 0.9
            }

            Text {
                id: loadingMsg

                anchors.centerIn: loadingBubble
                font.pointSize: 18
                text: "Loading..."
                visible: fractalView.isLoading
            }
        }
    }
}
