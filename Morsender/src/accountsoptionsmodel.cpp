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

#include "accountsoptionsmodel.h"
#include "defines.h"
#include <iostream>

AccountsOptionsModel::AccountsOptionsModel()
{

}

void AccountsOptionsModel::addOption(const Option &option)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_options << option;
    endInsertRows();
}

void AccountsOptionsModel::addOptions(PurpleAccount* account, bool newAccount) {
        this->account = account;
        username = QString::fromStdString(std::string(purple_account_get_username(account)));
        protocol = QString::fromStdString(std::string(purple_account_get_protocol_id(account)));
        enabled = purple_account_get_enabled(account, UI_ID);
//        if(enabled)
//            purple_account_set_enabled(account, UI_ID, true);

        GList *iter;
        iter = purple_plugins_get_loaded();
        for (; iter; iter = iter->next) {
            PurplePlugin *plugin = (PurplePlugin*) iter->data;
            const char* pluginid = purple_plugin_get_id(plugin);
            const char* accountid = purple_account_get_protocol_id(account);
            if(strcmp(pluginid, accountid) != 0) continue;
            PurplePluginInfo *info = plugin->info;
            PurplePluginProtocolInfo *prpl_info = PURPLE_PLUGIN_PROTOCOL_INFO(plugin);

            // Basic options
            Option accountOption;
            accountOption.name = "Enabled";
            accountOption.defaultValue = "0";
            accountOption.value = enabled ? "1" : "0";
            accountOption.type = "bool";
            accountOption.setting = "enable";
            if(newAccount)
                accountOption.disabled = true;
            Option usernameOption;
            usernameOption.name = "Username";
            usernameOption.defaultValue = "sailor";
            usernameOption.value = QString::fromUtf8(purple_account_get_username(account));
            usernameOption.type = "string";
            usernameOption.setting = "username";
            usernameOption.disabled = enabled;
            Option passwordOption;
            passwordOption.name = "Password";
            passwordOption.defaultValue = "";
            passwordOption.value = purple_account_get_password(account);
            passwordOption.type = "password";
            passwordOption.setting = "password";
            Option rememberOption;
            rememberOption.name = "Remember Password";
            rememberOption.defaultValue = "1";
            rememberOption.value = purple_account_get_remember_password(account) ? "1" : "0";
            rememberOption.type = "bool";
            rememberOption.setting = "rememberPassword";

            GList *l;
            char* username = g_strdup(purple_account_get_username(account));
            std::vector<Option> splits;
            for (l = g_list_last(prpl_info->user_splits); l != NULL; l = l->prev) {
                    PurpleAccountUserSplit* split = (PurpleAccountUserSplit*)l->data;

                    const char *value = NULL;
                    char *c;

                    if (account != NULL) {
                        if(purple_account_user_split_get_reverse(split))
                            c = strrchr(username,
                                    purple_account_user_split_get_separator(split));
                        else
                            c = strchr(username,
                                    purple_account_user_split_get_separator(split));

                        if (c != NULL) {
                            *c = '\0';
                            c++;

                            value = c;
                        }
                    }
                    if (value == NULL)
                        value = purple_account_user_split_get_default_value(split);


                    Option splitOption;
                    splitOption.name = purple_account_user_split_get_text(split);
                    splitOption.defaultValue = purple_account_user_split_get_default_value(split);
                    splitOption.value = value;
                    splitOption.type = "string";
                    splitOption.setting = "username";
                    splits.push_back(splitOption);

            }
            usernameOption.value = username;


            addOption(accountOption);
            addOption(usernameOption);

            for (auto split = splits.rbegin(); split != splits.rend(); ++split)
            {
                addOption(*split);
            }
            if (prpl_info != NULL && !(prpl_info->options & OPT_PROTO_NO_PASSWORD)) {
                addOption(passwordOption);
                addOption(rememberOption);
            }

            for (l = prpl_info->protocol_options; l != NULL; l = l->next)
            {
                if(!l) continue;
                PurpleAccountOption *purpleOption = (PurpleAccountOption *)l->data;
                PurplePrefType type = purple_account_option_get_type(purpleOption);
                const char* setting = purple_account_option_get_setting(purpleOption);
                setting = purple_account_option_get_text(purpleOption);
                if(PURPLE_PREF_STRING == type) {
                    Option option;
                    option.name = purple_account_option_get_text(purpleOption);
                    option.type = "string";
                    option.defaultValue = QString::fromUtf8(purple_account_option_get_default_string(purpleOption));
                    option.value = QString::fromUtf8(purple_account_get_string(account, purple_account_option_get_setting(purpleOption),purple_account_option_get_default_string(purpleOption)));
                    option.setting = purple_account_option_get_setting(purpleOption);
                    addOption(option);
                } else if(PURPLE_PREF_INT == type) {
                    Option option;
                    option.name = purple_account_option_get_text(purpleOption);
                    option.type = "int";
                    option.defaultValue = QString::number(purple_account_option_get_default_int(purpleOption));
                    option.value = QString::number(purple_account_get_int(account, purple_account_option_get_setting(purpleOption),purple_account_option_get_default_int(purpleOption)));
                    option.setting = purple_account_option_get_setting(purpleOption);
                    addOption(option);
                } else if(PURPLE_PREF_BOOLEAN == type) {
                    Option option;
                    option.name = purple_account_option_get_text(purpleOption);
                    option.type = "bool";
                    option.defaultValue = QString::number(purple_account_option_get_default_bool(purpleOption));
                    option.value = QString::number(purple_account_get_bool(account, purple_account_option_get_setting(purpleOption),purple_account_option_get_default_bool(purpleOption)));
                    option.setting = purple_account_option_get_setting(purpleOption);
                    addOption(option);
                } else if(PURPLE_PREF_STRING_LIST == type) {
                    Option option;
                    option.name = purple_account_option_get_text(purpleOption);
                    option.type = "stringList";
                    option.defaultValue = purple_account_option_get_default_list_value(purpleOption);
                    option.setting = purple_account_option_get_setting(purpleOption);
                    GList *perfList;
                    perfList = purple_account_option_get_list(purpleOption);
                    int index = 0;
                    for (perfList; perfList != NULL; perfList = perfList->next) {
                        if (perfList->data != NULL) {
                            PurpleKeyValuePair* kvp = (PurpleKeyValuePair *) perfList->data;
                            option.listName<<kvp->key;
                            option.listValue<<(char*)kvp->value;
                            if(strcmp((char*) kvp->value, purple_account_get_string(this->account,option.setting,purple_account_option_get_default_list_value(purpleOption))) == 0)
                                option.listIndex = index;
                            index++;
                        }
                    }
                    addOption(option);
                }
            }
        }
}

bool AccountsOptionsModel::getEnabled() {
    return purple_account_get_enabled(account, UI_ID);
}

int AccountsOptionsModel::rowCount(const QModelIndex & parent) const {
    Q_UNUSED(parent);
    return m_options.count();
}

QVariant AccountsOptionsModel::data(const QModelIndex & index, int role) const {
    if (index.row() < 0 || index.row() >= m_options.count())
        return QVariant();

    const Option &option = m_options[index.row()];
    if (role == Name)
        return option.name;
    else if (role == Value)
        return option.value;
    else if (role == DefaultValue)
        return option.defaultValue;
    else if (role == Type)
        return option.type;
    else if (role == ListValue)
        return option.listName;
    else if (role == Index)
        return option.listIndex;
    else if (role == Disabled)
        return option.disabled;
    return QVariant();
}

bool AccountsOptionsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid()) {
        Option &option = m_options[index.row()];
        if (role == Value || role == Index) {
            if (option.setting == "enable") {
                purple_account_set_enabled(this->account, UI_ID, value.toBool());
                enabled = value.toBool();
            } else if (option.setting == "username") {
                username.replace(option.value, value.toString());
                option.value = value.toString();
                purple_account_set_username(account, username.toStdString().c_str());
            } else if (option.setting == "password")
                purple_account_set_password(this->account, value.toString().toStdString().c_str());
            else if (option.setting == "rememberPassword")
                purple_account_set_remember_password(this->account, value.toBool());
            else if (option.type == "bool")
                purple_account_set_bool(this->account, option.setting, value.toBool());
            else if (option.type == "string")
                purple_account_set_string(this->account, option.setting, value.toString().toStdString().c_str());
            else if (option.type == "int")
                purple_account_set_int(this->account, option.setting, value.toInt());
            else if (option.type == "stringList") {
                purple_account_set_string(this->account, option.setting, option.listValue.at(value.toInt()).toStdString().c_str());
            }
            option.value = value.toString();
            emit dataChanged(index, index);
            sendModifiedSignal();
        }
        return true;
    }
    return false;

}

Qt::ItemFlags AccountsOptionsModel::flags(const QModelIndex &index) const {
    return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
}

QHash<int, QByteArray> AccountsOptionsModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[Name] = "optionName";
    roles[Value] = "optionValue";
    roles[DefaultValue] = "optionDefaultValue";
    roles[ListValue] = "optionListValue";
    roles[Type] = "optionType";
    roles[Index] = "optionIndex";
    roles[Disabled] = "optionDisabled";
    return roles;
}

void AccountsOptionsModel::sendModifiedSignal() {
    purple_signal_emit(getAccountHandle(), "account-modified", this->account);
}

void* AccountsOptionsModel::getAccountHandle(void) {
    static int handle;

    return &handle;
}
