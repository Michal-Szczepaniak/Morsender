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

#ifndef MORSENDERPLUGININFO_H
#define MORSENDERPLUGININFO_H

#include <TransferEngine-qt5/transferplugininfo.h>
#include <TransferEngine-qt5/transfermethodinfo.h>

class MorsenderPluginInfo : public TransferPluginInfo
{
    Q_OBJECT
public:
    MorsenderPluginInfo();
    ~MorsenderPluginInfo();

    QList<TransferMethodInfo> info() const;
    void query();
    bool ready() const;

private:
    QList<TransferMethodInfo> m_infoList;
    bool m_ready;

};

#endif // MORSENDERPLUGININFO_H
