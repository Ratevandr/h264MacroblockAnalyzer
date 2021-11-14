import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.15
import QtQuick.Dialogs 1.2
import Qt.labs.platform 1.0


Window {
    id: window
    width: 800
    height: 500
    minimumWidth: videoElemRect.width
    visible: true
    title: qsTr("Macroblock analyzer")

    Connections {
        target: cvImageProv

        function onImageChanged() {
            videoElem.reload()
        }
    }

    Connections{
        target: FrameStream

        function onUpdateFrameNum(frameNumStr, curFrameNum) {
            frameNumView.text = frameNumStr
            videoSlider.value = curFrameNum
        }
        function onInitSlider(maxFrameCount) {
            videoSlider.to = maxFrameCount

        }
        function onUpdateTextView(mbInfo) {
            textArea.text = mbInfo

        }
        function onUpdateFrameTypeInfo(frameTypeStr) {
            frameType.text = frameTypeStr;
        }
    }
    Column {
        id: col
        x: 0
        y: 0
        width: window.width
        height: window.height



        Row {
            id: rown
            width:  window.width
            height: 64
            layoutDirection: Qt.LeftToRight
            spacing: 6


            Button {
                id: btnOpenFile
                width: 33
                height: 32
                text: qsTr("📂")
                onClicked: {
                    fileDialog.open();

                }
            }


            Button {
                id: btnPrevFrame
                width: 33
                height: 32
                text: qsTr("⏪")
            }

            Button {
                id: btnPlay
                width: 32
                height: 32
                text: qsTr("▶")
                onClicked: {
                    FrameStream.play()
                }
            }


            Button {
                id: btnNextFrame
                width: 32
                height: 32
                text: qsTr("⏩️")
                onClicked: {
                    FrameStream.getNextFrame()
                }
            }

            Button {
                id: btnPause
                width: 32
                height: 32
                text: qsTr("⏸")
                onClicked: {
                    FrameStream.pause()
                }
            }

            TextEdit {
                id: frameNumView
                x: 0
                y: 0
                width: 111
                height: 32
                text: qsTr("00:00:01:20")
                font.pixelSize: 20
                horizontalAlignment: Text.AlignLeft
                selectByMouse: true
                Connections {
                    target: frameNumView
                    onEditingFinished: {
                        FrameStream.pause()
                        FrameStream.seekToTimecode(frameNumView.text)
                    }
                }
            }

            Button {
                id: button
                x: 0
                y: 0
                width: 84
                height: 32
                text: qsTr("GoToFrame")
                autoExclusive: true
            }

            Switch {
                id: switchFrameView
                text: qsTr("Show frame number")
            }

            Switch {
                id: switchMbSkipView
                text: qsTr("Show Mb SKIP")
            }


        }

        Row {
            id: row
            width: window.width
            height: window.height-rown.height
            layoutDirection: Qt.LeftToRight

            spacing: 0

            Column {
                id: column
                width: 200
                height: 400

                Rectangle {
                    id: videoElemRect
                    width: window.width - groupBox.width
                    height: window.height- rown.height - videoSlider.height - 5
                    border.color: "#FF0000"
                    border.width: 1

                    ScrollView {
                        width: videoElemRect.width
                        height: videoElemRect.height
                        clip: true

                        Image {
                            property bool counter: false
                            id: videoElem
                            source: "image://live/image"
                            fillMode: Image.PreserveAspectFit
                            visible: true
                            width: sourceSize.width
                            height: sourceSize.height
                            function reload() {
                                counter = !counter
                                source = "image://live/image?=id=" + counter
                            }
                            MouseArea {
                                width: videoElem.width
                                height: videoElem.height
                                hoverEnabled: true
                                acceptedButtons: Qt.LeftButton
                                onClicked: {
                                    var positionInPopup = mapToItem(videoElem, mouse.x, mouse.y)
                                    textArea.text = positionInPopup.x+" "+positionInPopup.y
                                    FrameStream.selectMacroblock(positionInPopup.x,positionInPopup.y)

                                }
                            }

                        }
                    }
                }


                Slider {
                    id: videoSlider
                    y: 0
                    height: 22
                    width: videoElemRect.width
                    stepSize: 1
                    to: 10
                    topPadding: 0
                    transformOrigin: Item.Top
                    value: 100

                    Connections {
                        target: videoSlider
                        onPressedChanged: {

                            if (videoSlider.pressed) {
                                FrameStream.pause()
                            } else {
                                FrameStream.seekToFrame(videoSlider.value)
                            }
                        }
                    }
                }
            }

            GroupBox {
                id: groupBox
                x: 0
                width: 318
                height: window.height - rown.height
                anchors.right: parent.right
                anchors.rightMargin: 0
                transformOrigin: Item.Center
                title: qsTr("")

                Column {
                    id: col1
                    x: 0
                    y: 0
                    width: 306
                    height: 400

                    Flickable {
                        id: flickable
                        width: 280
                        height: 298
                        TextArea.flickable: TextArea {
                            id: textArea
                            text: ""
                            anchors.fill: parent
                            wrapMode: TextArea.Wrap
                            smooth: false
                            hoverEnabled: true
                            placeholderText: "Macroblock information"
                            font.hintingPreference: Font.PreferNoHinting
                            font.pointSize: 10
                        }

                        ScrollBar.vertical: ScrollBar { }
                    }

                    Text {
                        id: frameTypeLabel
                        text: qsTr("Frame type:")
                        font.pixelSize: 20
                    }

                    Text {
                        id: frameType
                        text: qsTr("-")
                        font.pixelSize: 20
                    }
                }

                FileDialog {
                    id: fileDialog;
                    title: "Please choose a file";
                    nameFilters: ["File with h264 codec (*.*)"];
                    fileMode: FileDialog.OpenFile
                    onAccepted: {
                        if (fileDialog.file.toString().length > 0) {
                            console.log("User has selected " + fileDialog.file.toString());
                            FrameStream.openVideoCamera(fileDialog.file.toString())
                        }
                        fileDialog.close()
                    }
                }

            }

        }

    }


}


