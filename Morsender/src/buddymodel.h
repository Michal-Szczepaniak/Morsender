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

#ifndef BUDDYMODEL_H
#define BUDDYMODEL_H

#include <QAbstractListModel>
#include <libpurple/purple.h>
#include "chatmodel.h"

class BuddyModel : public QAbstractListModel
{
    Q_OBJECT
public:
    BuddyModel(ChatModel* chatModel);

    enum BuddyRoles {
        Id = Qt::UserRole + 1,
        Name,
        ConvName,
        Status,
        Avatar,
        Type,
        Group,
        Account,
        Node,
        Priority,
        PluginIcon,
        PluginName
    };

    void addNode(PurpleBlistNode* node);
    Q_INVOKABLE void hide(QString section);
    Q_INVOKABLE bool isSectionHidden(QString section);
    Q_INVOKABLE QString getPluginName(PurpleAccount* account) const;
    Q_INVOKABLE void removeNode(int index);
    Q_INVOKABLE void removeConv(int index);
    Q_INVOKABLE void removeBuddy(int index);
    Q_INVOKABLE void removeChat(int index);

    int rowCount(const QModelIndex & parent = QModelIndex()) const;

    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

protected:
    QHash<int, QByteArray> roleNames() const;
    void updateBlistNode();

private:
    const char* getNodeName(PurpleBlistNode* node) const;
    std::string getPluginIconPath(PurpleAccount* account) const;
    PurpleConvChat* getConvChat(PurpleBlistNode* node) const;
    QList<PurpleBlistNode*> m_nodes;
    QStringList hiddenSections;
    ChatModel* chatModel;

signals:
    Q_INVOKABLE void chatSwitched(PurpleBlistNode* node, PurpleAccount* account);
    Q_INVOKABLE void chatSwitched(PurpleConversation* node, PurpleAccount* account);
    Q_INVOKABLE void chatSwitched(QString username, bool type, PurpleAccount* account);

public slots:
    void redoBuddyList();

    /**
     * @brief conversationWasCreated slot on conversation creation
     * @param conv PurpleCoversation that was created
     * @param data ChatModel object
     */
    void conversationWasCreated(PurpleConversation *conv, void *data);

};

Q_DECLARE_METATYPE(BuddyModel*)
Q_DECLARE_METATYPE(PurpleBlistNode*)

#endif // BUDDYMODEL_H
