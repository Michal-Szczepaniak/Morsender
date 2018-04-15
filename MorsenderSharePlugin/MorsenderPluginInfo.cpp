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

#include "MorsenderPluginInfo.h"
#include <QDebug>

MorsenderPluginInfo::MorsenderPluginInfo(): m_ready(false)
{
}

MorsenderPluginInfo::~MorsenderPluginInfo()
{

}

QList<TransferMethodInfo> MorsenderPluginInfo::info() const
{
    return m_infoList;
}

void MorsenderPluginInfo::query()
{
    TransferMethodInfo info;

    QStringList capabilities;
    capabilities << QLatin1String("*");

    info.displayName     = QLatin1String("Morsender");
    info.methodId        = QLatin1String("MorsenderSharePlugin");
    info.shareUIPath     = QLatin1String("/usr/share/Morsender/qml/pages/Share.qml");
    info.capabilitities  = capabilities;
    m_infoList << info;
    m_ready = true;
    emit infoReady();
}

bool MorsenderPluginInfo::ready() const
{
    return m_ready;
}
