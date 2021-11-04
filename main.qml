import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.15


Window {
    id: window
    width: 730
    height: 860
    minimumWidth: videoElemRect.width
    visible: true
    title: qsTr("Macroblock analyzer")



    Connections{
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
        anchors.fill: parent

        spacing: 5
        Rectangle {
            id: videoElemRect
            x: 0
            anchors.horizontalCenter: col.horizontalCenter
            //anchors.left: parent.left
            //anchors.leftMargin: ((videoElem.width)*0.15)
            //anchors.leftMargin: 5
            width: (videoElem.width+12)*0.8
            height: (videoElem.height+12)*0.8
            border.color: "#FF0000"
            border.width: 1
            //anchors.horizontalCenterOffset: 0
            transform: Scale {
                id: videoElemScale
                xScale: 0.8;
                yScale: 0.8}


            Image {
                property bool counter: false
                //property alias frameCounter: frameCount

                id: videoElem
                source: "image://live/image"
                anchors.topMargin: 5
                anchors.bottomMargin: 5
                anchors.leftMargin: 5
                anchors.rightMargin: 5
                //transformOrigin: Item.Center

                fillMode: Image.PreserveAspectFit
                visible: true
                anchors.left: parent.left
                //anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: videoSlider.top

                width: sourceSize.width > 0 ? sourceSize.width : 690
                height: sourceSize.height > 0 ? sourceSize.height : 250
              //  transform: Scale { origin.x: 25; origin.y: 25; xScale: 0.85; yScale: 0.85}
                function reload() {
                    counter = !counter
                    source = "image://live/image?=id=" + counter
                    //slider.value = frameCount
                }
                MouseArea {
                    anchors.fill: videoElem
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

        Slider {
            id: videoSlider
            width: groupBox.width
            height: 22
            anchors.top: videoElemRect.bottom
            anchors.topMargin: videoElemRect.height*0.10
            anchors.horizontalCenter: groupBox.horizontalCenter
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

        GroupBox {
            id: groupBox
            width: 700
            height: 350
            anchors.top: videoSlider.bottom
            anchors.topMargin: 5
            anchors.horizontalCenter: parent.horizontalCenter
            title: qsTr("")

            Button {
                id: btnPlay
                x: 374
                y: 0
                width: 32
                height: 32
                text: qsTr("▶")
                anchors.left: btnPrevFrame.right
                anchors.right: frameNumView.left
                anchors.rightMargin: -414
                anchors.leftMargin: 5
                onClicked: {
                    FrameStream.play()
                }

            }

            Switch {
                id: switchFrameView
                x: 0
                y: 29
                text: qsTr("Show frame number")
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
                y: 0
                width: 84
                height: 32
                text: qsTr("GoToFrame")
                anchors.left: frameNumView.right
                anchors.leftMargin: 60
                autoExclusive: true
            }


            Flickable {
                id: flickable

                anchors.fill: parent
                anchors.topMargin: 38
                anchors.leftMargin: 185

                TextArea.flickable: TextArea {
                    id: textArea
                    text: "TextArea\n...\n...\n...\n...\n...\n...\n"
                    anchors.fill: parent
                    wrapMode: TextArea.Wrap
                    smooth: false
                    hoverEnabled: true
                    placeholderText: "Text Area"
                    font.hintingPreference: Font.PreferNoHinting
                    font.pointSize: 10
                }

                ScrollBar.vertical: ScrollBar { }
            }

            //            TextArea {
            //                id: textArea
            //                x: 257
            //                y: 38
            //                width: 425
            //                height: 142
            //                smooth: false
            //                hoverEnabled: true
            //                placeholderText: "Text Area"
            //                font.hintingPreference: Font.PreferNoHinting
            //                font.pointSize: 7
            //            }

            Text {
                id: text1
                x: 0
                y: 64
                text: qsTr("Frame type:")
                font.pixelSize: 20
            }

            Text {
                id: frameType
                x: 117
                y: 64
                text: qsTr("-")
                font.pixelSize: 20
            }

            Text {
                id: text3
                x: 0
                y: 93
                text: qsTr("Text")
                font.pixelSize: 20
            }

            Button {
                id: btnNextFrame
                width: 32
                height: 32
                text: qsTr("⏩️")
                anchors.left: btnPause.right
                anchors.leftMargin: 5
                onClicked: {
                    FrameStream.getNextFrame()
                }
            }

            Button {
                id: btnPrevFrame
                width: 33
                height: 32
                text: qsTr("⏪")
                anchors.left: btnOpenFile.right
                anchors.leftMargin: 5
            }

            Button {
                id: btnPause
                width: 32
                height: 32
                text: qsTr("⏸")
                anchors.left: btnPlay.right
                anchors.leftMargin: 5
                onClicked: {
                    FrameStream.pause()
                }
            }

            Button {
                id: btnOpenFile
                width: 33
                height: 32
                text: qsTr("📂")
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.leftMargin: 304
                anchors.topMargin: 0
                onClicked: {
                    FrameStream.openVideoCamera("/home/user/Видео/video/out.mp4")
                }
            }

            Switch {
                id: switchMbSkipView
                x: 0
                y: 127
                text: qsTr("Show Mb SKIP")
            }


        }
    }


}

/*##^##
Designer {
    D{i:0;formeditorZoom:0.9}D{i:8}D{i:22}D{i:23}D{i:24}D{i:10}
}
##^##*/
