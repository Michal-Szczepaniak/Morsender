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

#include "MorsenderSharePlugin.h"
#include "MorsenderPluginInfo.h"
#include "MorsenderMediaTransfer.h"

#include <QtPlugin>

MorsenderSharePlugin::MorsenderSharePlugin()
{

}

MorsenderSharePlugin::~MorsenderSharePlugin()
{

}

QString MorsenderSharePlugin::pluginId() const
{
    return QLatin1String("MorsenderSharePlugin");
}

bool MorsenderSharePlugin::enabled() const
{
    return true;
}

TransferPluginInfo *MorsenderSharePlugin::infoObject()
{
    return new MorsenderPluginInfo;
}

MediaTransferInterface *MorsenderSharePlugin::transferObject()
{
    return new MorsenderMediaTransfer;
}
