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

#ifndef ACCOUNTSOPTIONSMODEL_H
#define ACCOUNTSOPTIONSMODEL_H

#include <QAbstractListModel>
#include <libpurple/purple.h>

class AccountsOptionsModel : public QAbstractListModel
{
    Q_OBJECT
public:
    AccountsOptionsModel();

    enum OptionRoles {
        Name = Qt::UserRole + 1,
        Value,
        DefaultValue,
        ListValue,
        Type,
        Index,
        Disabled
    };

    struct Option {
       QString name;
       QString value;
       QString defaultValue;
       QStringList listName;
       QStringList listValue;
       int listIndex;
       QString type;
       const char* setting;
       QChar split;
       bool disabled = false;
    };

    QString username;
    QString protocol;
    int     protocolID = 0;
    bool    enabled;

    void addOption(const Option &option);
    void addOptions(PurpleAccount* account, bool newAccount = false);
    Q_INVOKABLE bool getEnabled();

    int rowCount(const QModelIndex & parent = QModelIndex()) const;

    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

    bool setData(const QModelIndex &item, const QVariant &value, int role);

    Qt::ItemFlags flags(const QModelIndex &index) const;

    PurpleAccount* account;
protected:
    QHash<int, QByteArray> roleNames() const;

private:
    QList<Option> m_options;
    void sendModifiedSignal();
    void* getAccountHandle(void);
};

Q_DECLARE_METATYPE(AccountsOptionsModel*)

#endif // ACCOUNTSOPTIONSMODEL_H
