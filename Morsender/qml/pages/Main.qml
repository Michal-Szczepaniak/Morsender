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

import QtQuick 2.4
import Sailfish.Silica 1.0
import org.nemomobile.dbus 2.0
import "../components"

Page {
    id: testPage

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.All

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

    SlideshowView {
        id: swipeView
        itemWidth: width
        itemHeight: height
        clip: true

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: mainPageHeader.top

        model: VisualItemModel {
            BuddyList { }
            Chat { id: chatPage }
            Settings { }
        }
    }

    TabHeader {
        id: mainPageHeader
        listView: swipeView
        iconArray: [ "image://theme/icon-m-events", "image://theme/icon-s-chat", "image://theme/icon-m-developer-mode", ]
    }
}
