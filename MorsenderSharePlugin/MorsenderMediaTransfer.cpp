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

#include <QDebug>
#include "MorsenderMediaTransfer.h"
#include <QStringList>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusInterface>

MorsenderMediaTransfer::MorsenderMediaTransfer(QObject *parent) :
    MediaTransferInterface(parent)
{
//    flickrUploadInterface = new FlickrUploadInterface(this);
//    connect(flickrUploadInterface, SIGNAL(uploadMediaStarted()), this, SLOT(uploadMediaStarted()));
//    connect(flickrUploadInterface, SIGNAL(uploadMediaProgress(qint64,qint64)), this, SLOT(uploadMediaProgress(qint64,qint64)));
//    connect(flickrUploadInterface, SIGNAL(uploadMediaFinished(bool)), this,SLOT(uploadMediaFinished(bool)));
}

MorsenderMediaTransfer::~MorsenderMediaTransfer()
{
}

QString MorsenderMediaTransfer::displayName() const
{
    return QString("Morsender");
}

QUrl MorsenderMediaTransfer::serviceIcon() const
{
    return QUrl::fromLocalFile("/usr/share/icons/hicolor/256x256/apps/Morsender.png");
}

bool MorsenderMediaTransfer::cancelEnabled() const
{
    return false;
}

bool MorsenderMediaTransfer::restartEnabled() const
{
    return false;
}

void MorsenderMediaTransfer::start()
{
    QString url = mediaItem()->value(MediaItem::Url).toString();
    qDebug() << "url: " << url;
    QString tittle = mediaItem()->value(MediaItem::Title).toString();
    qDebug() << "tittle: " << tittle;
    QString description = mediaItem()->value(MediaItem::Description).toString();
    qDebug() << "description: " << description;

    QDBusConnection bus = QDBusConnection::sessionBus();
    QDBusInterface dbus_iface("pl.morsender.share", "/share",
                              "pl.morsender.share", bus);
    dbus_iface.call("share", description);
}

void MorsenderMediaTransfer::uploadMediaStarted() {
    setStatus(MediaTransferInterface::TransferStarted);
}

void MorsenderMediaTransfer::cancel() {
    setStatus(MediaTransferInterface::TransferCanceled);
}

void MorsenderMediaTransfer::uploadMediaProgress(qint64 bytesSent, qint64 bytesTotal) {
    qreal percent;
    if (bytesTotal != -1) {
        percent = qRound((qreal)bytesSent / (qreal)bytesTotal);
        setProgress(percent);
    } else {
        setStatus(MediaTransferInterface::NotStarted);
    }
}

void MorsenderMediaTransfer::uploadMediaFinished(bool ok) {
    if (ok) {
        setStatus(MediaTransferInterface::TransferFinished);
    } else {
        setStatus(MediaTransferInterface::TransferInterrupted);
    }
}
