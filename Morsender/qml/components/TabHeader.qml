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

import QtQuick 2.1
import Sailfish.Silica 1.0

Item {
    id: tabPageHeader

    property SlideshowView listView: null
    property variant iconArray: []
    property int visibleHeight: flickable.contentY + height

    anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
    height: Theme.itemSizeMedium

    SilicaFlickable {
        id: flickable
        anchors.fill: parent
        contentHeight: parent.height

        Row {
            anchors.fill: parent

            Repeater {
                id: sectionRepeater
                model: iconArray
                delegate: BackgroundItem {

                    width: tabPageHeader.width / sectionRepeater.count
                    height: tabPageHeader.height

                    Image {
                        id: icon
                        height: Theme.iconSizeSmall * 1.4
                        width: Theme.iconSizeSmall * 1.4
                        anchors.centerIn: parent
                        source: modelData
                    }

                   Label {
                        anchors {
                            top: parent.top; topMargin: Theme.paddingSmall
                            left: icon.right; leftMargin: -Theme.paddingMedium
                        }
                        visible: listView.model.children[index].unreadCount > 0
                        font.pixelSize: Theme.fontSizeSmall
                        color: Theme.highlightColor

                        text: listView.model.children[index].unreadCount ? listView.model.children[index].unreadCount : ""
                    }

                    Loader {
                        anchors.fill: parent
                        sourceComponent: listView.model.children[index].busy
                                         ? busyIndicator : undefined
                        Component {
                            id: busyIndicator

                            Rectangle {
                                anchors.fill: parent
                                color: "black"
                                opacity: 0

                                Behavior on opacity { NumberAnimation { duration: 250 } }

                                BusyIndicator {
                                    opacity: 1
                                    anchors.centerIn: parent
                                    running: true
                                    height: tabPageHeader.height - Theme.paddingLarge
                                    width: height
                                }

                                Component.onCompleted: opacity = 0.75
                            }
                        }

                    }

                    onClicked: listView.currentIndex === index ? listView.currentItem.positionAtTop()
                                                                   : listView.currentIndex = index

                }
            }
        }

        Rectangle {
            id: currentSectionIndicator
            anchors.top: parent.top
            color: Theme.highlightColor
            height: Theme.paddingSmall
            width: tabPageHeader.width / sectionRepeater.count
            x: listView.currentIndex * width

            Behavior on x {
                NumberAnimation {
                    duration: 200
                }
            }
        }
    }
}
