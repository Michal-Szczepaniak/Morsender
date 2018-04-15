/*
    Copyright (C) 2018 Michał Szczepaniak

    This file is part of Morsender.

    Morsender is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Morsender is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Morsender.  If not, see <http://www.gnu.org/licenses/>.
*/

import QtQuick 2.6
import Sailfish.Silica 1.0
import QtGraphicalEffects 1.0
import QtMultimedia 5.0
import org.nemomobile.notifications 1.0
import org.nemomobile.configuration 1.0
import Sailfish.Pickers 1.0

Page {
    id: chatPage
    property string userName: ""
    property string name: ""
    property string pluginName: ""
    property int type: 0

    function addZero(i) {
        if (i < 10) {
            i = "0" + i;
        }
        return i;
    }

    ConfigurationValue {
         id: theme
         key: "/apps/morsender/theme"
         defaultValue: 0
    }

    ConfigurationValue {
        id: cleanAvatars
        key: "/apps/morsender/cleanAvatars"
        defaultValue: true
    }

    Component {
        id: users
        Page {
            SilicaListView {
                clip: true
                spacing: 10
                anchors.fill: parent
                header: PageHeader {
                    id: pageHeader
                    title: userName
                    description: type === 2 ? chatModel.status ? "Online" : "Offline" : chatModel.topic
                }
                model: chatModel.chatUsers
                delegate: BackgroundItem {
                    Label {
                        text: modelData[0]
                        x: Theme.horizontalPageMargin
                        verticalAlignment: "AlignVCenter"
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                    }

                    onClicked: {
                        buddyModel.chatSwitched(modelData[1], false, chatModel.getActiveConvAccount());
                        pageStack.replace(Qt.resolvedUrl("Chat.qml"), {
                                           userName: modelData[0],
                                           type: 2,
                                           name: modelData[1],
                                           pluginName: pluginName
                                       })
                    }
                }
            }
        }
    }

    Connections {
        target: chatPage
        onStatusChanged: {
            if(chatPage.status == PageStatus.Active && type == 3) {
                pageStack.pushAttached(users)
            }
        }
    }


    SilicaFlickable {
        anchors.fill: parent
        contentHeight: parent.height - Theme.paddingLarge
        clip: true
        PageHeader {
            id: pageHeader
            title: userName
            description: type === 2 ? chatModel.status ? "Online" : "Offline" : chatModel.topic
        }

        SilicaListView {
            id: chat
            spacing: 10
            anchors.fill: parent
            anchors.topMargin: pageHeader.height
            anchors.bottomMargin: textInput.height/2
            anchors.rightMargin: Theme.paddingLarge
            anchors.leftMargin: Theme.paddingLarge
            verticalLayoutDirection: ListView.BottomToTop
            clip: true
            model: chatModel
            delegate: Item {
                width: parent.width
                height: theme.value === 1 ? column.height : bubbleColumn.height
                Item{
                    id: clearDelegate
                    width: theme.value === 1 ? parent.width : 0
                    height: theme.value === 1 ? column.height : 0
                    visible: theme.value === 1 ? true : false

                    Image {
                        id: clearImageAvatar
                        source: avatar
                        width: type == 2 ? Theme.paddingLarge*2 : 0
                        height: Theme.paddingLarge*2
                        anchors.left: parent.left
                        visible: cleanAvatars.value && recived && type == 2 ? true : false
                        fillMode: Image.PreserveAspectCrop
                        layer.enabled: true
                        layer.effect: OpacityMask {
                            maskSource: clearMask
                        }
                    }

                    Rectangle {
                        id: clearMask
                        width: Theme.paddingLarge*2
                        height: Theme.paddingLarge*2
                        radius: 50
                        visible: cleanAvatars.value && !recived && type == 2
                        color: Theme.highlightColor
                    }

                    Column {
                        id: column
                        spacing: Theme.paddingSmall
                        anchors.leftMargin: cleanAvatars.value ? Theme.paddingLarge : Theme.paddingSmall
                        anchors.right: parent.right
                        anchors.left: cleanAvatars.value ? clearImageAvatar.right : parent.left
                        Image {
                            id: img
                            source: image
                            visible: image
                            fillMode: Image.PreserveAspectFit
                            width: parent.width - Theme.paddingMedium
                            height: parent.width - Theme.paddingMedium
                            sourceSize.width: 2048
                            sourceSize.height: 2048
                        }

                        VideoOutput {
                            id: cleanVideo
                            fillMode: VideoOutput.PreserveAspectFit
                            visible: video
                            width: parent.width - Theme.paddingMedium
                            height: parent.width - Theme.paddingMedium
                            source: MediaPlayer {
                                id: cleanVideoPlayer
                                autoPlay: true
                                source: video
                                loops: MediaPlayer.Infinite
                                onError: {
                                    cleanVideo.visible = false;
                                    console.error("Videoplayer can't load video: " + errorString)
                                }
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    if (cleanVideoPlayer.playbackState === MediaPlayer.PlayingState) {
                                        cleanVideoPlayer.pause();
                                    } else {
                                        cleanVideoPlayer.play();
                                    }
                                }
                            }
                        }

                        LinkedLabel {
                            id: umessage
                            plainText: message.trim()
                            font.pixelSize: Theme.fontSizeMedium
                            horizontalAlignment: Text.AlignLeft
                            width: parent.width - Theme.paddingMedium
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            color: recived ? Theme.primaryColor : Theme.highlightColor
                        }

                        Text {
                            id: time
                            text: name + "  " + timestamp
                            horizontalAlignment: Text.AlignLeft
                            font.pixelSize: Theme.fontSizeTiny
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            color: Theme.secondaryColor
                        }
                    }
                }

                Item{
                    id: bubblesDelegate
                    width: theme.value === 0 ? parent.width : 0
                    height: theme.value === 0 ? bubbleColumn.height : 0
                    visible: theme.value === 0 ? true : false
                    Image {
                        id: bubblesImageAvatar
                        source: avatar
                        width: type == 2 ? Theme.paddingLarge*3 : 0
                        height: Theme.paddingLarge*3
                        anchors.left: parent.left
                        visible: recived && type == 2 ? true : false
                        fillMode: Image.PreserveAspectCrop
                        layer.enabled: true
                        layer.effect: OpacityMask {
                            maskSource: bubbleMask
                        }
                    }

                    Rectangle {
                        id: bubbleMask
                        width: Theme.paddingLarge*3
                        height: Theme.paddingLarge*3
                        radius: 10
                        visible: false
                    }

                    Rectangle {
                        anchors.fill: bubbleColumn
                        radius: 10
                        color: Theme.rgba(Theme.highlightBackgroundColor, Theme.highlightBackgroundOpacity)
                    }
                    Column {
                        id: bubbleColumn
                        spacing: Theme.paddingSmall
                        width: parent.width/1.5
                        anchors.leftMargin: Theme.paddingSmall
                        anchors.right: !recived ? parent.right : undefined
                        anchors.left: recived ? bubblesImageAvatar.right : undefined
                        leftPadding: Theme.paddingMedium
                        rightPadding: Theme.paddingMedium
                        topPadding: Theme.paddingMedium
                        bottomPadding: Theme.paddingMedium

                        Image {
                            id: bubbleImg
                            source: image
                            visible: image
                            width: parent.width - Theme.paddingMedium*2
                            height: parent.width - Theme.paddingMedium*2
                            fillMode: Image.PreserveAspectFit
                            sourceSize.width: 2048
                            sourceSize.height: 2048
                        }

                        VideoOutput {
                            id: bubbleVideo
                            fillMode: VideoOutput.PreserveAspectFit
                            visible: video
                            width: parent.width - Theme.paddingMedium*2
                            height: parent.width - Theme.paddingMedium*2
                            source: MediaPlayer {
                                id: bubbleVideoPlayer
                                autoPlay: true
                                source: video
                                loops: MediaPlayer.Infinite
                                onError: {
                                    bubbleVideo.visible = false;
                                    console.error("Videoplayer can't load video: " + errorString)
                                }
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    if (bubbleVideoPlayer.playbackState === MediaPlayer.PlayingState) {
                                        bubbleVideoPlayer.pause();
                                    } else {
                                        bubbleVideoPlayer.play();
                                    }
                                }
                            }
                        }

                        LinkedLabel {
                            id: bubbleMessage
                            plainText: message.trim()
                            width: parent.width - Theme.paddingMedium*2
                            font.pixelSize: Theme.fontSizeMedium
                            horizontalAlignment: recived ? Text.AlignLeft : Text.AlignRight
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            color: Theme.primaryColor
                        }

                        Text {
                            id: bubbleTime
                            text: recived ? timestamp + "  " + name : name + "  " + timestamp
                            width: parent.width - Theme.paddingMedium*2
                            horizontalAlignment: recived ? Text.AlignLeft : Text.AlignRight
                            font.pixelSize: Theme.fontSizeTiny
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            color: Theme.secondaryColor
                        }
                    }
                }
            }

            VerticalScrollDecorator {}
        }

        TextField {
            id: textInput
            anchors.bottomMargin: -textInput.height/2
            anchors.bottom: parent.bottom
            anchors.right: pluginIndicator.left
            anchors.left: parent.left
            placeholderText: "Say something..."
            color: Theme.primaryColor
            focus: true
            focusOutBehavior: FocusBehavior.KeepFocus
            Keys.onReturnPressed: {
                if(textInput.text.length != 0) {
                    chatModel.sendMessage(textInput.text)
                    textInput.text = ""
                    textInput.focus = false
                    textInput.focus = true
                }
            }
        }

        Label {
           id: pluginIndicator
           text: pluginName
           anchors.bottom: parent.bottom
           anchors.right: parent.right
           anchors.rightMargin: Theme.paddingLarge
           color: Theme.highlightColor
           font.pixelSize: Theme.fontSizeSmall
        }

        PushUpMenu {
            id: pushUpMenu
            spacing: Theme.paddingLarge
            MenuItem {
                text: "Send file"
                enabled: chatModel.fileSendingEnabled
                onClicked: pageStack.push(contentPickerPage)
            }
        }
    }

    Component {
         id: contentPickerPage
         ContentPickerPage {
             title: "Select file"
             onSelectedContentPropertiesChanged: {
                 chatModel.sendFile(selectedContentProperties.filePath)
             }
         }
     }
}
