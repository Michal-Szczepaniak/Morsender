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

//    if (flickrUploadInterface->connStatus() == FlickrUploadInterface::Logged) {
//        QString url = mediaItem()->value(MediaItem::Url).toString().replace("file://", "");
//        QString title = mediaItem()->value(MediaItem::Title).toString();
//        QStringList descriptions = mediaItem()->value(MediaItem::Description).toString().split(",");
//        QString photosetId = "no";

//        if (!descriptions.isEmpty()) {
//            photosetId = descriptions.takeLast();
//        }

//        descriptions.insert(0, title);
//        QStringList::const_iterator constIterator;
//        for (constIterator = descriptions.constBegin(); constIterator != descriptions.constEnd(); ++constIterator) {
//            qDebug() << " - " << (*constIterator).toLatin1().constData();
//        }

//        QString mymeType = mediaItem()->value(MediaItem::MimeType).toString();
//        qDebug() << "mymeType: " << mymeType;

//        QString params = mediaItem()->value(MediaItem::UserData).toString();
//        qDebug() << "userData: " << params;

//        flickrUploadInterface->send(url, descriptions, photosetId);
//    }
//    else {
//        qDebug() << "Error: flickrUploadInterface->connStatus() == " << flickrUploadInterface->connStatus();
//    }
    qDebug() << "hui";
}

void MorsenderMediaTransfer::uploadMediaStarted() {
    printf("Eyyy morender 2\n");
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
