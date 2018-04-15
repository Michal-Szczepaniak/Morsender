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

import QtQuick 2.0
import Sailfish.Silica 1.0
import org.nemomobile.configuration 1.0

Page {
    id: page

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.All

    ConfigurationGroup {
        id: settings
        path: "/apps/morsender"

        property int theme: 0
        property bool chatNotifications: false
        property bool errorNotifications: true
        property bool offlineUsers: false

        // Clean theme settings
        property bool cleanAvatars: true

        // Buddy list theme
        property int statusIndicator: 1
        property bool pluginIcons: true
        property bool listAvatars: true
        property bool roundListAvatars: false
    }

    SilicaFlickable {
        anchors.fill: parent

        contentHeight: parent.height

        PageHeader {
            id: header
            title: qsTr("Settings")
        }

        Column {
            spacing: Theme.paddingLarge
            anchors.topMargin: header.height
            anchors.fill: parent
            ComboBox {
               id: themes
               width: parent.width
               label: "Theme"
               currentIndex: settings.theme

               menu: ContextMenu {
                   MenuItem { text: "Bubble" }
                   MenuItem { text: "Clear" }
               }

               onCurrentItemChanged: settings.theme = themes.currentIndex
            }

            TextSwitch {
               id: cleanThemeAvatarsSwitch
               visible: settings.theme === 1
               checked: settings.cleanAvatars
               width: parent.width
               text: qsTr("Show avatars in clean theme")
               onClicked: settings.cleanAvatars = !settings.cleanAvatars
            }

            TextSwitch {
               id: chatNotificationsSwitch
               checked: settings.chatNotifications
               width: parent.width
               text: qsTr("Show notifications from chat groups")
               onClicked: settings.chatNotifications = !settings.chatNotifications
            }

            TextSwitch {
               id: errorNotificationsSwitch
               checked: settings.errorNotifications
               width: parent.width
               text: qsTr("Show error notifications")
               onClicked: settings.errorNotifications = !settings.errorNotifications
            }

            TextSwitch {
               id: offlineUsersSwitch
               checked: settings.offlineUsers
               width: parent.width
               text: qsTr("Show offline users on the list")
               onClicked: settings.offlineUsers = !settings.offlineUsers
            }

            TextSwitch {
               id: pluginIconsSwitch
               checked: settings.pluginIcons
               width: parent.width
               text: qsTr("Show plugin icons on the list")
               onClicked: settings.pluginIcons = !settings.pluginIcons
            }

            TextSwitch {
               id: listAvatarsSwitch
               checked: settings.listAvatars
               width: parent.width
               text: qsTr("Show avatars on the list")
               onClicked: settings.listAvatars = !settings.listAvatars
            }

            TextSwitch {
               id: roundListAvatarsSwitch
               checked: settings.roundListAvatars
               width: parent.width
               text: qsTr("Rounded avatars on the list")
               onClicked: settings.roundListAvatars = !settings.roundListAvatars
            }

            ComboBox {
               id: blistThemes
               width: parent.width
               label: qsTr("Buddy list online indicator")
               currentIndex: settings.statusIndicator

               menu: ContextMenu {
                   MenuItem { text: "None" }
                   MenuItem { text: "Backgroud grandient" }
                   MenuItem { text: "Icon status" }
               }

               onCurrentItemChanged: settings.statusIndicator = blistThemes.currentIndex
            }
        }
        VerticalScrollDecorator {}
    }
}
