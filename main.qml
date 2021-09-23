import QtQuick 2.15
import QtQuick.Window 2.15
import fracture 1.0

Window {
    width: 700
    height: 700
    visible: true
    title: "fracture"

    FractalView {
        id: fractalView

        anchors.fill: parent
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

        TapHandler {
            acceptedButtons: Qt.LeftButton
            onTapped: {
                if (fractalView.type === FractalView.Mandelbrot)
                    fractalView.switchToJulia(eventPoint.position.x, eventPoint.position.y)
                else
                    fractalView.switchToMandelbrot()
            }
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
}
