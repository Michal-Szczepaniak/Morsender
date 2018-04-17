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
import Sailfish.Email 1.1
import Sailfish.TransferEngine 1.0
import org.nemomobile.dbus 2.0

ShareDialog {
    id: root

    allowedOrientations: Orientation.All


    property bool isLink: root.content
                                && ('type') in root.content
                                && root.content.type === "text/x-url"
    readonly property bool isPlainText: root.content
                                        &&('type' in content) && content.type === "text/plain"

    DBusInterface {
        id: g_dbusInterface
        service: "com.mistermagister.morsender"
        path: "/"
        iface: "com.mistermagister.morsender"
    }

    onAccepted: {
        var description = content
                && content.type
                && ('status' in content) ? content.status : "";

        if ((source == '') && content && ('data' in content) && content.data) {
            description = tempWriter.writeToFile(content.data, content.name || 'data', '');
        }

        shareItem.userData = {
            "title": isLink ? root.content.linkTitle : root.content ? content.name : "Empty",
            "description": description
        }

        g_dbusInterface.call("share",{
                                 "title": isLink ? root.content.linkTitle : root.content ? content.name : "Empty",
                                 "description": description
                             })
        shareItem.start()
    }

    SailfishShare {
        id: shareItem
        source: root.source
        metadataStripped: true
        serviceId: root.methodId
    }

    DialogHeader {
        id: dialogHeader
        acceptText: qsTrId("Share")
    }

    SilicaFlickable {
        anchors {
            top: dialogHeader.bottom
            topMargin: Theme.paddingLarge
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        contentHeight: column.height

        Column {
            id: column

            width: parent.width
            spacing: Theme.paddingLarge

            Label {
                width: parent.width
                text: "Target chat choose window will open after accepting."
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            }
    }

    TempFileWriter {
        id: tempWriter
    }
}
