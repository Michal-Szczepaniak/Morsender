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
    Q_PROPERTY(int status READ getStatus NOTIFY statusChanged)
    Q_PROPERTY(bool enabled READ getEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QString username READ getUsername NOTIFY usernameChanged)
    Q_PROPERTY(QString prtocol READ getProtocol NOTIFY protocolChanged)
    Q_PROPERTY(int protocolID READ getProtocolID NOTIFY protocolIDChanged)
public:
    AccountsOptionsModel(int protocolID = NULL);

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

    void addOption(const Option &option);
    void addOptions(PurpleAccount* account, bool newAccount = false);

    int rowCount(const QModelIndex & parent = QModelIndex()) const;

    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

    bool setData(const QModelIndex &item, const QVariant &value, int role);

    Qt::ItemFlags flags(const QModelIndex &index) const;

    PurpleAccount* account;

    /* Getters */
    int getStatus();
    bool getEnabled();
    QString getUsername();
    QString getProtocol();
    int getProtocolID();

signals:
    void accountOptionsChanged(PurpleAccount* account);
    void statusChanged();
    void enabledChanged();
    void usernameChanged();
    void protocolChanged();
    void protocolIDChanged();

public slots:
    void accountsOptionsAdded();
    void accountStatusChanged(PurpleAccount *account, void *data);

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    QList<Option>   m_options;
    void*   getAccountHandle(void);
    int     m_status = 0;
    bool    m_enabled = 0;
    QString m_username = "";
    QString m_protocol = "";
    int     m_protocolID = 0;
};

Q_DECLARE_METATYPE(AccountsOptionsModel*)

#endif // ACCOUNTSOPTIONSMODEL_H
