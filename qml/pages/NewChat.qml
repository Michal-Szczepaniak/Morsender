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

Dialog {
    id: page

    acceptDestinationAction: PageStackAction.Pop

    onAccepted: {
        buddyModel.chatSwitched(nickname.text, convType.currentIndex, accountsModel.data(accountsModel.index(accounts.currentIndex,0,this), 262));
    }

    SilicaFlickable {
        anchors.fill: parent

        contentHeight: parent.height

        PageHeader {
            id: header
            title: qsTr("New…")
        }

        Column {
            spacing: Theme.paddingLarge
            anchors.topMargin: header.height
            anchors.fill: parent

            ComboBox {
               id: convType
               width: parent.width
               label: "Type"
               currentIndex: 0

               menu: ContextMenu {
                   MenuItem { text: "Private Message" }
                   MenuItem { text: "Chat" }
               }
            }

            TextField {
                id: nickname
                placeholderText: "Nickname"
                width: parent.width
            }

            ComboBox {
                id: accounts
                width: page.width
                label: "Account Type"
                currentIndex: 0
                menu: ContextMenu {
                    Repeater {
                        model: accountsModel
                        delegate: MenuItem { text: accountsModel.data(accountsModel.index(index,0,this), 258) }
                    }
                }
                onCurrentItemChanged:  console.log(accounts.currentIndex, accountsModel.data(accountsModel.index(accounts.currentIndex,0,this), 258))
//                onCurrentItemChanged: account.option = accountsModel.newAccount(plugins.currentIndex)
            }
        }
    }
}
