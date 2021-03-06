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
import com.mistermagister.options 1.0

Page {
    id: page

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.All

    Label {
        id: emptyText
        anchors.fill: parent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        text: "There is no account configured. \nTo start with Morsender configure account first from pull down menu."
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        visible: !accountsModel.rowCount()
    }

    SilicaFlickable {
        anchors.fill: parent

        PullDownMenu {
            MenuItem {
                text: qsTr("Add account")
                onClicked: {
                    accountPage.newAccount = true
                    pageStack.push(accountPage, {option: accountsModel.newAccount(0)})
                }
            }
        }

        // Tell SilicaFlickable the height of its content.
        contentHeight: parent.height

        SilicaListView {
            id: listView
            model: accountsModel
            anchors.fill: parent
            clip: true
            header: PageHeader {
                title: qsTr("Accounts")
            }
            delegate: ListItem {
                id: delegate
                ListView.onRemove: animateRemoval()
                menu: contextMenuComponent
                function remove() {
                    remorseAction("Deleting", function() { accountsModel.removeAccount(index) })
                }

                Label {
                    x: Theme.horizontalPageMargin
                    text: username
                    anchors.verticalCenter: parent.verticalCenter
                    color: delegate.highlighted ? Theme.highlightColor : Theme.primaryColor
                }
                onClicked: {
                    plugins.currentIndex = protocolID
                    accountPage.option = options
                    accountPage.newAccount = false
                    pageStack.push(accountPage)
                }

                Component {
                    id: contextMenuComponent
                    ContextMenu {
                        MenuItem {
                            text: "Delete"
                            onClicked: remove()
                        }
                    }
                }
            }
            VerticalScrollDecorator {}
        }
    }

    Dialog {
        id: accountPage
        // The effective value will be restricted by ApplicationWindow.allowedOrientations
        allowedOrientations: Orientation.All
        acceptDestination: page
        acceptDestinationAction: PageStackAction.Pop
        property var option: null
        property bool newAccount: false

        PageHeader {
            id: header
            title: accountPage.newAccount ? "Add Account" : "Account settings"
        }


        ComboBox {
            id: plugins
            width: page.width
            label: "Account Type"
            anchors.top: header.bottom
            enabled: !accountPage.option.enabled
            menu: ContextMenu {
                Repeater {
                    model: accountsModel.getPluginList();
                    delegate: MenuItem { text: accountsModel.getPluginList()[index] }
                }
            }
            onCurrentItemChanged: accountPage.option = accountsModel.newAccount(plugins.currentIndex)
        }

        SilicaListView {
            id: optionsListView
            model: accountPage.option
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: plugins.bottom
            anchors.bottom: parent.bottom
            clip: true
            delegate: Column {
                id: optionsDelegate
                x: Theme.horizontalPageMargin
                width: accountPage.width - Theme.horizontalPageMargin*2
                spacing: Theme.paddingSmall
                    Label {
                        id: stringName
                        width: parent.width
                        text: optionName
                        wrapMode: Text.WordWrap
                        color: optionsDelegate.highlighted ? Theme.highlightColor : Theme.primaryColor
                    }

                    TextField {
                        id: stringValue
                        text: optionValue
                        width: parent.width
                        readOnly: optionDisabled
                        visible: optionType === "string"
                        EnterKey.onClicked: optionValue = stringValue.text
                        onFocusChanged: optionValue = stringValue.text
                    }

                    PasswordField {
                        id: passwordValue
                        text: optionValue
                        width: parent.width
                        readOnly: optionDisabled
                        visible: optionType === "password"
                        EnterKey.onClicked: optionValue = passwordValue.text
                        onFocusChanged: optionValue = passwordValue.text
                    }

                    TextField {
                        id: intValue
                        text: optionValue
                        readOnly: optionDisabled
                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                        width: parent.width
                        visible: optionType === "int"
                        EnterKey.onClicked: optionValue = intValue.text
                        onFocusChanged: optionValue = intValue.text
                    }

                    TextSwitch {
                        id: boolValue
                        checked: optionValue === "1"
                        visible: optionType === "bool"
                        width: parent.width
                        enabled: !optionDisabled
                        onClicked: optionValue = boolValue.checked
                    }

                    ComboBox {
                        id: stringListValue
                        width: page.width
                        visible: optionType === "stringList"
                        currentIndex: optionType === "stringList" ? optionIndex : -1
                        enabled: !optionDisabled
                        menu: ContextMenu {
                            Repeater {
                                model: optionListValue
                                delegate: MenuItem { text: modelData }
                            }
                        }
                        onCurrentItemChanged: optionValue = stringListValue.currentIndex
                    }
            }
            VerticalScrollDecorator {}
        }

        onAccepted: {
            if(accountPage.newAccount)
                accountsModel.addAccount(accountPage.option, true)
        }
    }
}
