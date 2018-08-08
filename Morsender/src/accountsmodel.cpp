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

#include "accountsmodel.h"
#include <purple.h>
#include <QDebug>
#include "chatmodel.h"

AccountsModel::AccountsModel()
{
    GList* accounts = purple_accounts_get_all();
    for (; accounts != NULL; accounts = accounts->next) {
        PurpleAccount* account = (PurpleAccount*)accounts->data;
        int protocolID = NULL;

        GList *iter = purple_plugins_get_loaded();
        for (int i = 0; iter; iter = iter->next, i++) {
            if(strcmp(purple_plugin_get_id((PurplePlugin*)iter->data), purple_account_get_protocol_id(account)) == 0)
                protocolID = i;
        }

        AccountsOptionsModel *option = new AccountsOptionsModel(protocolID);
        option->addOptions(account);

        addAccount(option, FALSE);
    }

    void *account_handle = purple_accounts_get_handle();
    static int handle;
    purple_signal_connect(account_handle,
                          "account-error-changed",
                          &handle,
                          PURPLE_CALLBACK(
                              AccountsModel::accountErrorChangedCb),
                          this);
    purple_signal_connect(account_handle,
                          "account-enabled",
                          &handle,
                          PURPLE_CALLBACK(
                              AccountsModel::accountStatusChangedCb),
                          this);
    purple_signal_connect(account_handle,
                          "account-disabled",
                          &handle,
                          PURPLE_CALLBACK(
                              AccountsModel::accountStatusChangedCb),
                          this);
//    purple_signal_connect(account_handle,
//                          "signed-on",
//                          &handle,
//                          PURPLE_CALLBACK(
//                              AccountsModel::accountStatusChangedCb),
//                          this);
//    purple_signal_connect(account_handle,
//                          "signed-off",
//                          &handle,
//                          PURPLE_CALLBACK(
//                              AccountsModel::accountStatusChangedCb),
//                          this)
}

AccountsOptionsModel* AccountsModel::newAccount(int index) {
    PurpleAccount *account = NULL;
    int i = 0;

    GList *iter = purple_plugins_get_loaded();
    for (; iter; iter = iter->next, i++) {
        if(i == index) {
            account = purple_account_new("Name", purple_plugin_get_id((PurplePlugin*)iter->data));
            break;
        }
    }

    if(i == index) {
        AccountsOptionsModel *option = new AccountsOptionsModel(index);
        option->addOptions(account, true);
        return option;
    } else {
        return NULL;
    }
}

void AccountsModel::removeAccount(int index) {
    beginRemoveRows(QModelIndex(), index, index);
    AccountsOptionsModel* option = m_options.at(index);
    m_options.removeAt(index);
    purple_accounts_delete(option->account);
    endRemoveRows();
}

void AccountsModel::addAccount(AccountsOptionsModel *account, int newAccount)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_options << account;
    endInsertRows();

    connect(account, SIGNAL(accountOptionsChanged(PurpleAccount*)),
            this, SLOT(accountOptionsChanged(PurpleAccount*)));
    connect(this, SIGNAL(accountStatusChanged(PurpleAccount*,void*)),
            account, SLOT(accountStatusChanged(PurpleAccount*,void*)));
    connect(this, SIGNAL(accountOptionsAdded()),
            account, SLOT(accountsOptionsAdded()));

    emit this->accountOptionsAdded();

    if(newAccount) {
        purple_accounts_add(account->account);
        purple_account_set_enabled(account->account, UI_ID, true);
    }
}

QStringList AccountsModel::getPluginList() {
    QStringList list;

    GList *iter = purple_plugins_get_loaded();
    for (; iter; iter = iter->next) {
        PurplePlugin *plugin = (PurplePlugin*) iter->data;
        list<<purple_plugin_get_name(plugin);
    }

    return list;
}

int AccountsModel::rowCount(const QModelIndex & parent) const {
    Q_UNUSED(parent);
    return m_options.count();
}

QVariant AccountsModel::data(const QModelIndex & index, int role) const {
    if (index.row() < 0 || index.row() >= m_options.count())
        return QVariant();


    AccountsOptionsModel *account = m_options[index.row()];
    if (role == Enabled)
        return account->getEnabled();
    else if (role == Username)
        return account->getUsername();
    else if (role == Protocol)
        return account->getProtocol();
    else if (role == ProtocolID)
        return account->getProtocolID();
    else if (role == Options)
        return QVariant::fromValue(account);
    else if (role == Account)
        return QVariant::fromValue(account->account);
    return QVariant();
}

QHash<int, QByteArray> AccountsModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[Enabled] = "enabled";
    roles[Username] = "username";
    roles[Protocol] = "protocol";
    roles[Options] = "options";
    roles[ProtocolID] = "protocolID";
    roles[Account] = "account";
    return roles;
}

void AccountsModel::accountErrorChanged(PurpleAccount *account, const PurpleConnectionErrorInfo *infoOld, const PurpleConnectionErrorInfo *infoNew, void *data) {
    gboolean descriptions_differ;
    const char *desc;
    if (infoOld == NULL && infoNew == NULL)
        return;

    if (infoOld != NULL && infoNew == NULL) {
        if(infoOld->type == PURPLE_CONNECTION_ERROR_NAME_IN_USE);
//            remove_from_signed_on_elsewhere(account);
        else
            removeError(account);
        return;
    }

    if (infoOld == NULL && infoNew != NULL) {
        if(infoNew->type == PURPLE_CONNECTION_ERROR_NAME_IN_USE);
//            add_to_signed_on_elsewhere(account);
        else
            addError(account, infoNew);
        return;
    }

    /* else, new and old are both non-NULL */

    descriptions_differ = strcmp(infoOld->description, infoNew->description);
    desc = infoNew->description;

    switch (infoNew->type) {
    case PURPLE_CONNECTION_ERROR_NAME_IN_USE:
        if (infoOld->type == PURPLE_CONNECTION_ERROR_NAME_IN_USE
            && descriptions_differ) {
//            update_signed_on_elsewhere_tooltip(account, desc);
        } else {
//            removeError(account);
//            add_to_signed_on_elsewhere(account);
        }
        break;
    default:
        if (infoOld->type == PURPLE_CONNECTION_ERROR_NAME_IN_USE) {
//            remove_from_signed_on_elsewhere(account);
            addError(account, infoNew);
        } else if (descriptions_differ) {
            updateError(account, infoNew);
        }
        break;
    }
}

void AccountsModel::addError(PurpleAccount *account, const PurpleConnectionErrorInfo *err) {
    const char *username = purple_account_get_username(account);

    this->message = QString::fromUtf8(err->description);
    this->account = account;
    emit messageChanged(message);
    emit accountStatusChanged(account);
    QString accountName = QString::fromUtf8(
                purple_account_get_protocol_name(account)) + ": " +
                QString::fromUtf8(purple_account_get_username(account));
    emit errorAdded(message, accountName);
}

void AccountsModel::updateError(PurpleAccount *account, const PurpleConnectionErrorInfo *err) {
    const char *username = purple_account_get_username(account);

    this->message = QString::fromUtf8(err->description);
    this->account = account;
    emit messageChanged(message);
    emit accountStatusChanged(account);
    QString accountName = QString::fromUtf8(
                purple_account_get_protocol_name(account)) + ": " +
                QString::fromUtf8(purple_account_get_username(account));
    emit errorUpdated(message, accountName);
}

void AccountsModel::removeError(PurpleAccount *account) {
    this->message = "";
    this->account = NULL;
    emit messageChanged(message);
    emit accountStatusChanged(account);
    QString accountName = QString::fromUtf8(
                purple_account_get_protocol_name(account)) + ": " +
                QString::fromUtf8(purple_account_get_username(account));
    emit errorRemoved(message, accountName);
}

void AccountsModel::reconnectAccount() {
    gboolean enabled = purple_account_get_enabled(this->account, purple_core_get_ui());

    if(enabled) {
        purple_account_connect(this->account);
    } else {
        purple_account_clear_current_error(this->account);
        purple_account_set_enabled(this->account, purple_core_get_ui(), TRUE);
    }
}

void AccountsModel::accountOptionsChanged(PurpleAccount *account) {
//    beginResetModel();
//    endResetModel();
}

void AccountsModel::modifyAccount() {
    purple_account_clear_current_error(this->account);
}

QString AccountsModel::getMessage() {
    return this->message;
}

PurpleAccount* AccountsModel::getAccount() {
    return this->account;
}

void AccountsModel::accountErrorChangedCb(PurpleAccount *account,
                                          const PurpleConnectionErrorInfo *infoOld,
                                          const PurpleConnectionErrorInfo *infoNew,
                                          void *data) {
    AccountsModel* accountsModel = (AccountsModel*)data;
    emit accountsModel->accountErrorChanged(account, infoOld, infoNew, data);
}

void AccountsModel::accountStatusChangedCb(PurpleAccount *account,
                                          void *data) {
    AccountsModel* accountsModel = (AccountsModel*)data;
    emit accountsModel->accountStatusChanged(account, data);
}

void* AccountsModel::getAccountHandle(void) {
    static int handle;

    return &handle;
}
