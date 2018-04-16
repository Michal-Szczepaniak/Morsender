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
import SortFilterProxyModel 0.2
import QtGraphicalEffects 1.0
import org.nemomobile.notifications 1.0
import com.mistermagister.accounts 1.0
import org.nemomobile.configuration 1.0
import org.nemomobile.dbus 2.0

Page {
    id: buddyListPage
    property bool searching: false

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

    DBusAdaptor {
        id: shareDBusInterface
        service: "com.mistermagister.morsender"
        path: "/"
        iface: "com.mistermagister.morsender"
        xml: '<interface name="com.mistermagister.morsender">
                          <method name="share"> <arg type="s" name="title" direction="in"/> <arg type="s" name="description" direction="in"/> </method>
                          <method name="openApp"> </method>
                      </interface>'

        function share(args) {
            console.log(args.title, args.description);
        }

        function openApp() {
            console.log("n0otif");
            buddyModel.activate();
            app.activate()
        }
    }

    Connections {
        target: chatModel
        onMessageRecived: {
            messageNotification.summary = name;
            messageNotification.body = message;
            messageNotification.previewSummary = name;
            messageNotification.previewBody = message;
            if(!mutedNotifications) {
                if(type) {
                    if(username === chatModel.chatName && Qt.application.state === Qt.ApplicationInactive)
                        messageNotification.publish();
                    if(username !== chatModel.chatName)
                        messageNotification.publish();
                } else if(!type && settings.chatNotifications) {
                    if(username === chatModel.chatName && Qt.application.state === Qt.ApplicationInactive)
                        messageNotification.publish();
                    if(username !== chatModel.chatName)
                        messageNotification.publish();
                }
            }
        }
    }

    Connections {
        target: buddyListPage
        onStatusChanged: {
            if(buddyListPage.status == PageStatus.Active) {
                var name = chatModel.chatName
//                console.log(chatModel.chatName)
                chatModel.chatName = ""
            }
        }
    }

    Connections {
        target: accountsModel
        onErrorAdded: {
            errorPanel.visible = true
            errorLabel.text = message
            accountLabel.text = account
            notification.body = message
            if(settings.errorNotifications && !mutedNotifications)
                notification.publish()
        }
        onErrorUpdated: {
            errorLabel.text = message
            notification.body = message
            if(settings.errorNotifications && !mutedNotifications)
                notification.publish()
        }

        onErrorRemoved: {
            errorPanel.visible = false
            errorLabel.text = ""
        }
    }

    SilicaFlickable {
        anchors.fill: parent

        PullDownMenu {
            MenuItem {
                text: qsTr("Quit")
                onClicked: Qt.quit()
            }
            MenuItem {
                text: qsTr("About")
                onClicked: pageStack.push(Qt.resolvedUrl("About.qml"))
            }
            MenuItem {
                text: qsTr("Settings")
                onClicked: pageStack.push(Qt.resolvedUrl("Settings.qml"))
            }

            MenuItem {
                text: qsTr("Accounts")
                onClicked: pageStack.push(Qt.resolvedUrl("AccountsList.qml"))
            }

            MenuItem {
                text: qsTr("New…")
                onClicked: pageStack.push(Qt.resolvedUrl("NewChat.qml"))
            }
        }

        PageHeader {
            id: search
            title: qsTr("Buddy list")
        }

        // Tell SilicaFlickable the height of its content.
        contentHeight: parent.height

        SilicaListView {
            id: buddyList
            model: buddyProxyModel
            currentIndex: -1
            anchors.top: search.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            header: SearchField {
                id: searchField
                width: parent.width
                placeholderText: "Search"
                onTextChanged: searchField.text.length > 0 ? searching = true : searching = false
            }
            delegate: ListItem {
                id: delegate

                menu: contextMenuComponent

                Component {
                    id: contextMenuComponent
                    ContextMenu {
                        MenuItem {
                            text: "Delete"
                            onClicked: buddyModel.removeNode(id)
                        }
                    }
                }

                Rectangle {
                    visible: settings.statusIndicator === 1
                    anchors.fill: parent
                    gradient: Gradient {
                        GradientStop {
                            id: cgradient
                            position: 0.0
                            color: Theme.rgba( (status == 1 ? "green" : "red") , 0.3)
                        }
                        GradientStop {
                            position: 1.0
                            color: "transparent"
                        }
                    }
                }

                Image {
                    id: buddyAvatar
                    anchors.top: buddyName.top
                    anchors.bottom: buddyName.bottom
                    anchors.rightMargin: settings.listAvatars ? Theme.paddingLarge : undefined
                    anchors.left: parent.left
                    anchors.leftMargin: settings.listAvatars ? Theme.paddingLarge : undefined
                    verticalAlignment: Image.AlignVCenter
                    horizontalAlignment: Image.AlignHCenter
                    fillMode: Image.PreserveAspectFit
                    visible: settings.listAvatars
                    source: avatar
                    width: buddyName.height
                    height: buddyName.height
                    layer.enabled: true
                    layer.effect: OpacityMask {
                        maskSource: avatarMask
                    }
                }

                Rectangle {
                    id: avatarMask
                    width: buddyAvatar.width
                    height: buddyAvatar.height
                    radius: settings.roundListAvatars ? 90 : 10
                    visible: false
                }

                Label {
                    id: buddyName
                    x: Theme.horizontalPageMargin
                    text: name
                    anchors.leftMargin: Theme.paddingLarge
                    anchors.left: settings.listAvatars ? buddyAvatar.right : parent.left
                    anchors.right: settings.statusIndicator === 2 ? indicatorIcon.left : pluginIcons.left
                    anchors.verticalCenter: parent.verticalCenter
                    color: delegate.highlighted ? Theme.highlightColor : Theme.primaryColor
                }

                Image {
                    id: pluginIcons
                    anchors.top: buddyName.top
                    anchors.bottom: buddyName.bottom
                    anchors.right: parent.right
                    anchors.rightMargin: Theme.paddingLarge
                    verticalAlignment: Image.AlignVCenter
                    horizontalAlignment: Image.AlignHCenter
                    fillMode: Image.PreserveAspectFit
                    visible: settings.pluginIcons
                    source: pluginIcon
                    width: buddyName.height
                    height: buddyName.height
                }

                GlassItem {
                    id: indicatorIcon
                    visible: settings.statusIndicator === 2
                    color: status == 1 ? "green" : "red"
                    cache: false
                    anchors.right: settings.statusIndicator === 2 ? pluginIcons.left : parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    NumberAnimation on falloffRadius {
                        from: 0.05
                        to: 0.051
                        duration: 1000
                        running: Qt.application.active && false
                        loops: Animation.Infinite
                        easing {type: Easing.OutBack; overshoot: 1000}
                    }
                }

                onClicked: {
                    buddyModel.chatSwitched(node, account);
                    pageStack.push(Qt.resolvedUrl("Chat.qml"), {
                                       userName: name,
                                       type: type,
                                       name: convName,
                                       node: node,
                                       pluginName: pluginName
                                   })
                }
            }
            VerticalScrollDecorator {}
            section.property: "group"
            section.criteria: ViewSection.FullString
            section.delegate: SectionHeader {
                text: section
            }
        }
    }

    SortFilterProxyModel {
        id: buddyProxyModel
        sourceModel: buddyModel
        filters: [
            ValueFilter {
                enabled: true
                roleName: "type"
                value: 0
                inverted: true
            },
            ValueFilter {
                enabled: !settings.offlineUsers && !searching
                roleName: "status"
                value: 2
                inverted: true
            },
            RegExpFilter {
                roleName: "name"
                pattern: "^" + buddyList.headerItem.text
                caseSensitivity: Qt.CaseInsensitive
            }
        ]
        sorters: [
            RoleSorter { roleName: "priority"; ascendingOrder: false },
            RoleSorter { roleName: "group"; ascendingOrder: true },
            RoleSorter { roleName: "status"; ascendingOrder: true },
            RoleSorter { roleName: "name"; ascendingOrder: true }
        ]
    }

    Notification {
       id: messageNotification
       category: "x-nemo.messaging.sms"
       appIcon: "/usr/share/icons/hicolor/256x256/apps/Morsender.png"
       appName: "Morsender"
       summary: "Morsender Account Error"
       previewSummary: "Morsender Account Error"
       previewBody: "Error in account connection"
       expireTimeout: 0
       remoteActions: [ {
                    "name": "default",
                    "displayName": "Open App",
                    "icon": "icon-s-do-it",
                    "service": "com.mistermagister.morsender",
                    "path": "/",
                    "iface": "com.mistermagister.morsender",
                    "method": "openApp"
                } ]
    }

    Notification {
       id: notification
       category: "x-nemo.messaging.sms"
       appIcon: "/usr/share/icons/hicolor/256x256/apps/Morsender.png"
       appName: "Morsender"
       summary: "Morsender Account Error"
       previewSummary: "Morsender Account Error"
       previewBody: "Error in account connection"
       expireTimeout: 0
       onClicked: console.log("Clicked")
       onClosed: console.log("Closed, reason: " + reason)
    }

    Rectangle {
        id: errorPanel

        color: Theme.rgba(Theme.highlightBackgroundColor, Theme.highlightBackgroundOpacity)
        visible: false
        width: parent.width
        height: Theme.paddingLarge*10
        anchors.bottom: parent.bottom

        Column {
            id: errorColumn
            anchors.fill: parent
            anchors.topMargin: Theme.paddingLarge
            spacing: Theme.paddingLarge

            Label {
                id: accountLabel
                text: ""
                horizontalAlignment: Text.AlignHCenter
                width: parent.width
            }

            Label {
                id: errorLabel
                text: ""
                horizontalAlignment: Text.AlignHCenter
                width: parent.width
            }

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: Theme.paddingSmall
                Button {
                    text: "Modify Account"
                    onClicked: {
                        accountsModel.modifyAccount();
                        pageStack.push(Qt.resolvedUrl("AccountsList.qml"))
                    }
                }
                Button {
                    text: "Reconnect"
                    onClicked: accountsModel.reconnectAccount()
                }
            }
        }
    }
}
