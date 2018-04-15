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

#ifndef ACCOUNTSMODEL_H
#define ACCOUNTSMODEL_H

#include <QAbstractListModel>
#include "accountsoptionsmodel.h"
#include <libpurple/purple.h>
#include "defines.h"

class AccountsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString message READ getMessage NOTIFY messageChanged)
    Q_PROPERTY(PurpleAccount* account READ getAccount NOTIFY accountChanged)
public:
    AccountsModel();
    QString getMessage();
    PurpleAccount* getAccount();

    enum AccountsRoles {
        Enabled = Qt::UserRole + 1,
        Username,
        Protocol,
        ProtocolID,
        Options,
        Account
    };

    Q_INVOKABLE QStringList getPluginList();

    Q_INVOKABLE void removeAccount(int index);

    Q_INVOKABLE void addAccount(AccountsOptionsModel *account, int newAccount);

    Q_INVOKABLE AccountsOptionsModel* newAccount(int index);

    int rowCount(const QModelIndex & parent = QModelIndex()) const;

    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

signals:
    void messageChanged(QString message);
    void accountChanged(PurpleAccount* account);
    void errorAdded(QString message, QString account);
    void errorUpdated(QString message, QString account);
    void errorRemoved(QString message, QString account);
    void accountStatusChangedSignal(PurpleAccount *account, void *data);

public slots:
    Q_INVOKABLE void reconnectAccount();
    Q_INVOKABLE void modifyAccount();
    void accountStatusChanged(PurpleAccount *account, void *data);
    void accountErrorChanged(PurpleAccount *account, const PurpleConnectionErrorInfo *infoOld, const PurpleConnectionErrorInfo *infoNew, void *data);

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    QString message;
    PurpleAccount* account;
    QList<AccountsOptionsModel*> m_options;
    void* getAccountHandle(void);
    void addError(PurpleAccount *account, const PurpleConnectionErrorInfo *err);
    void updateError(PurpleAccount *account, const PurpleConnectionErrorInfo *err);
    void removeError(PurpleAccount *account);
    static void accountErrorChangedCb(
            PurpleAccount *account,
            const PurpleConnectionErrorInfo *infoOld,
            const PurpleConnectionErrorInfo *infoNew,
            void *data);
    static void accountStatusChangedCb(PurpleAccount *account, void *data);
};

#endif // ACCOUNTSMODEL_H
