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

#ifndef MORSENDERTRANSFERIFACE_H
#define MORSENDERTRANSFERIFACE_H

#include <TransferEngine-qt5/transferplugininterface.h>
#include <TransferEngine-qt5/transferplugininfo.h>
#include <TransferEngine-qt5/transfermethodinfo.h>
#include <TransferEngine-qt5/mediatransferinterface.h>

class Q_DECL_EXPORT MorsenderSharePlugin : public QObject, public TransferPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "pl.morsender.transfer.plugin")
    Q_INTERFACES(TransferPluginInterface)
public:
    MorsenderSharePlugin();
    ~MorsenderSharePlugin();

    MediaTransferInterface *transferObject();
    TransferPluginInfo *infoObject();
    QString pluginId() const;
    bool enabled() const;
};

#endif // MORSENDERTRANSFERIFACE_H
