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

#include "buddymodel.h"
#include <QTimer>

BuddyModel::BuddyModel(ChatModel* chatModel)
{
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(redoBuddyList()));
    timer->start(5000);

    this->chatModel = chatModel;

    connect(chatModel, SIGNAL(conversationCreated(PurpleConversation*,void*)), this, SLOT(conversationWasCreated(PurpleConversation*,void*)));
}

void BuddyModel::conversationWasCreated(PurpleConversation *conv, void *data) {
    beginInsertRows(QModelIndex(),rowCount(), rowCount());
    chatModel->conversationWasCreated(conv,data);
    endInsertRows();
    updateBlistNode();
//    emit dataChanged(createIndex(0,0),createIndex(rowCount()+1,0));
}

void BuddyModel::redoBuddyList()
{
    PurpleBlistNode *node;
    PurpleBuddyList* list = purple_get_blist();
    node = list->root;

    // Add new nodes dont add if account is disabled
    while (node)
    {
        /* This is only needed when we're reverting to a non-GTK+ sorted
         * status.  We shouldn't need to remove otherwise.
         */
//        if (remove && !PURPLE_BLIST_NODE_IS_GROUP(node))
//            pidgin_blist_hide_node(list, node, FALSE);

        addNode(node);

        node = purple_blist_node_next(node, FALSE);

    }

    // Here remove all buddies that dont exist anymore

    updateBlistNode();
}

void BuddyModel::updateBlistNode() {
    emit dataChanged(createIndex(0,0),createIndex(rowCount(),0));
}

void BuddyModel::addNode(PurpleBlistNode* node) {
    bool found = false;
    PurpleAccount* account = NULL;
    PurpleConnection* gc = NULL;

    for(PurpleBlistNode* bnode : m_nodes) {
        if(getNodeName(bnode) == getNodeName(node)) {
            found = true;
            break;
        }
    }

    if(PURPLE_BLIST_NODE_IS_BUDDY(node)) {
        account = purple_buddy_get_account((PurpleBuddy*)node);
    } else if(PURPLE_BLIST_NODE_IS_CHAT(node)) {
        account = purple_chat_get_account((PurpleChat*)node);
    }

    if(account != NULL)
        gc = purple_account_get_connection(account);

    if(found || PURPLE_BLIST_NODE_IS_CONTACT(node))
        return;

    if(account == NULL || gc == NULL || !purple_connection_get_state(gc))
            return;

    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_nodes << node;
    endInsertRows();

}

int BuddyModel::rowCount(const QModelIndex & parent) const {
    Q_UNUSED(parent);
    return m_nodes.count() + chatModel->chatCount();
}

QVariant BuddyModel::data(const QModelIndex & index, int role) const {
    if (index.row() < 0 || index.row() >= rowCount())
        return QVariant();

    if (index.row() < m_nodes.count()) {
        PurpleBlistNode* node = m_nodes[index.row()];
        if (role == Id) {
            return index.row();
        } else if (role == Name) {
            return getNodeName(node);
        } else if (role == Status) {
            if(PURPLE_BLIST_NODE_IS_BUDDY(node)) {
                return PURPLE_BUDDY_IS_ONLINE((PurpleBuddy*)node) ? "1": "2";
            } else {
                return "1";
            }
        } else if (role == Avatar) {
            if(PURPLE_BLIST_NODE_IS_BUDDY(node)) {
                PurpleBuddy* buddy = (PurpleBuddy*)node;
                PurpleBuddyIcon* icon = NULL;

                if(!buddy || !buddy->icon) return "";

                icon = purple_buddy_get_icon(buddy);

                if(!icon) return "";

                return QString::fromUtf8(purple_buddy_icon_get_full_path(icon));
            } else {
                PurpleStoredImage* icon = NULL;
                const char* iconName = NULL;

                if(purple_buddy_icons_node_has_custom_icon(node))
                    icon = purple_buddy_icons_node_find_custom_icon(node);

                if(!icon) return "";

                iconName = purple_imgstore_get_filename(icon);
                if(iconName)
                    return "/home/nemo/.purple/icons/" + QString::fromUtf8(iconName);
            }
        } else if (role == Node) {
            return QVariant::fromValue(node);
        } else if (role == Type) {
            return QString::number(purple_blist_node_get_type(node));
        } else if (role == Group) {
            return QString::fromUtf8(
                PURPLE_BLIST_NODE_IS_GROUP(node) ?
                    purple_group_get_name((PurpleGroup*)node) :
                    PURPLE_BLIST_NODE_IS_BUDDY(node) ?
                        purple_group_get_name(
                                    purple_buddy_get_group((PurpleBuddy*)node)) :
                        purple_group_get_name(
                            purple_chat_get_group((PurpleChat*)node))
                );
        } else if (role == PluginIcon) {
            PurpleGroup* group =  PURPLE_BLIST_NODE_IS_GROUP(node) ?
                        (PurpleGroup*)node :
                                PURPLE_BLIST_NODE_IS_BUDDY(node) ?
                                    purple_buddy_get_group((PurpleBuddy*)node) :
                                        purple_chat_get_group((PurpleChat*)node);
            if(!group) return "";
            PurpleAccount* account = (PurpleAccount*)purple_group_get_accounts(group)->data;
            return QString::fromStdString(getPluginIconPath(account));
        } else if (role == PluginName) {
            PurpleGroup* group =  PURPLE_BLIST_NODE_IS_GROUP(node) ?
                        (PurpleGroup*)node :
                                PURPLE_BLIST_NODE_IS_BUDDY(node) ?
                                    purple_buddy_get_group((PurpleBuddy*)node) :
                                        purple_chat_get_group((PurpleChat*)node);
            if(!group) return "";
            PurpleAccount* account = (PurpleAccount*)purple_group_get_accounts(group)->data;
            return getPluginName(account);
        } else if (role == Account) {
            if(PURPLE_BLIST_NODE_IS_BUDDY(node)) {
                return QVariant::fromValue(purple_buddy_get_account((PurpleBuddy*)node));
            } else if(PURPLE_BLIST_NODE_IS_CHAT(node)) {
                return QVariant::fromValue(purple_chat_get_account((PurpleChat*)node));
            }
        } else if (role == ConvName) {
            if(PURPLE_BLIST_NODE_IS_BUDDY(node)) {
                return QString::fromUtf8(purple_buddy_get_name((PurpleBuddy*)node));
            } else if(PURPLE_BLIST_NODE_IS_CHAT(node)) {
                return QString::fromUtf8(purple_chat_get_name((PurpleChat*)node));
            }
        } else if (role == Priority) {
            return 0;
        }
    } else if (index.row() >= m_nodes.count() && index.row() < rowCount()) {
        PurpleConversation* conv = chatModel->getConv(index.row() - m_nodes.count());
        if(!conv)
            return QVariant();

        if (role == Id) {
            return index.row();
        } else if (role == Name) {
            const char* name = purple_conversation_get_title(conv);
            if(!name)
                name = purple_conversation_get_name(conv);
            return QString::fromUtf8(name);
        } else if (role == Status) {
            return chatModel->getStatus(conv);
        } else if (role == Avatar) {
            PurpleBuddyIcon* icon;
            PurpleBuddy* buddy = purple_find_buddy(purple_conversation_get_account(conv),purple_conversation_get_name(conv));

            if(buddy) {
                icon = purple_buddy_get_icon(buddy);

                if(!icon) return "";

                return QString::fromUtf8(purple_buddy_icon_get_full_path(icon));
            } else {
                for(PurpleBlistNode* node: m_nodes)
                    if(strcmp(getNodeName(node), purple_conversation_get_title(conv)) == 0) {
                        PurpleStoredImage* icon = NULL;
                        const char* iconName = NULL;

                        if(purple_buddy_icons_node_has_custom_icon(node))
                            icon = purple_buddy_icons_node_find_custom_icon(node);

                        if(!icon) return "";

                        iconName = purple_imgstore_get_filename(icon);
                        if(iconName)
                            return "/home/nemo/.purple/icons/" + QString::fromUtf8(iconName);
                        else
                            return "";
                    }
                return "";
            }
        } else if (role == Node) {
            return QVariant::fromValue(conv);
        } else if (role == Type) {
            return QString::number(((int)purple_conversation_get_type(conv))+1);
        } else if (role == Group) {
            return QString::fromUtf8("Opened chats");
        } else if (role == PluginIcon) {
            PurpleAccount* account = purple_conversation_get_account(conv);
            return QString::fromStdString(getPluginIconPath(account));
        } else if (role == PluginName) {
            PurpleAccount* account = purple_conversation_get_account(conv);
            return getPluginName(account);
        } else if (role == Account) {
            return QVariant::fromValue(purple_conversation_get_account(conv));
        } else if (role == ConvName) {
            return QString::fromUtf8(purple_conversation_get_title(conv));
        } else if (role == Priority) {
            return 1;
        }
    }

    return QVariant();
}

QString BuddyModel::getPluginName(PurpleAccount* account) const {
    if(!account) return "";

    PurpleConnection* gc = purple_account_get_connection(account);

    if(!gc) return "";

    PurplePlugin* prpl = purple_connection_get_prpl(gc);

    if(!prpl) return "";

    const char *protoname = NULL;

    protoname = purple_plugin_get_name(prpl);

    if(!protoname) return "";

    return QString::fromUtf8(protoname);
}

void BuddyModel::removeNode(int index) {
    if(index < 0 || index > rowCount())
        return;

    if(index >= m_nodes.count()) {
        removeConv(index);
    } else {
        PurpleBlistNode* node = m_nodes[index];
        if(PURPLE_BLIST_NODE_IS_BUDDY(node))
            removeBuddy(index);
        else if (PURPLE_BLIST_NODE_IS_CHAT(node))
            removeChat(index);
    }
}

void BuddyModel::removeConv(int index) {
    beginRemoveRows(QModelIndex(), index, index);
    PurpleConversation* conv = chatModel->getConv(index - m_nodes.count());
    chatModel->removeConv(index - m_nodes.count());
    purple_conversation_destroy(conv);
    endRemoveRows();
}

void BuddyModel::removeBuddy(int index) {
    PurpleBlistNode* node = m_nodes[index];
    PurpleBuddy* buddy = (PurpleBuddy*)node;
    PurpleGroup *group;
    gchar *name;
    PurpleAccount *account;

    group = purple_buddy_get_group(buddy);
    name = g_strdup(buddy->name); /* b->name is a crasher after remove_buddy */
    account = buddy->account;

    beginRemoveRows(QModelIndex(), index, index);

    m_nodes.removeOne(node);

    purple_account_remove_buddy(account, buddy, group);
    purple_blist_remove_buddy(buddy);

    endRemoveRows();

    g_free(name);
}

void BuddyModel::removeChat(int index) {
    PurpleBlistNode* node = m_nodes[index];

    beginRemoveRows(QModelIndex(), index, index);

    m_nodes.removeOne(node);
    purple_blist_remove_chat((PurpleChat*)node);

    endRemoveRows();
}

std::string BuddyModel::getPluginIconPath(PurpleAccount* account) const {
    if(!account) return "";

    PurpleConnection* gc = purple_account_get_connection(account);

    if(!gc) return "";

    PurplePlugin* prpl = purple_connection_get_prpl(gc);

    if(!prpl) return "";

    PurplePluginProtocolInfo *prpl_info;
    const char *protoname = NULL;
    char *tmp;
    char *filename = NULL;

    prpl_info = PURPLE_PLUGIN_PROTOCOL_INFO(prpl);
    if (prpl_info->list_icon == NULL)
        return "";

    protoname = prpl_info->list_icon(account, NULL);
    if (protoname == NULL)
        return "";

    tmp = g_strconcat(protoname, ".png", NULL);

    filename = g_build_filename("/usr/share/Morsender/qml/resources/images/protocols/48/", tmp, NULL);
    g_free(tmp);
    return filename;
}

void BuddyModel::hide(QString section) {
    if(hiddenSections.indexOf(section) != -1)
        hiddenSections.removeAt(hiddenSections.indexOf(section));
    else
        hiddenSections.append(section);
    updateBlistNode();
}

bool BuddyModel::isSectionHidden(QString section) {
    if(hiddenSections.indexOf(section) != -1)
        return true;
    else
        return false;
}

const char* BuddyModel::getNodeName(PurpleBlistNode* node) const {
    if(PURPLE_BLIST_NODE_IS_BUDDY(node))
        return purple_buddy_get_alias((PurpleBuddy*)node);
    else if(PURPLE_BLIST_NODE_IS_CHAT(node))
        return purple_chat_get_name((PurpleChat*)node);
    else if(PURPLE_BLIST_NODE_IS_GROUP(node))
        return purple_group_get_name((PurpleGroup*)node);
    else if(PURPLE_BLIST_NODE_IS_CONTACT(node))
        return purple_contact_get_alias((PurpleContact*)node);
}

PurpleConvChat* BuddyModel::getConvChat(PurpleBlistNode* node) const {
    return NULL;
}

QHash<int, QByteArray> BuddyModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[Id] = "id";
    roles[Name] = "name";
    roles[Status] = "status";
    roles[Avatar] = "avatar";
    roles[Type] = "type";
    roles[Group] = "group";
    roles[Account] = "account";
    roles[ConvName] = "convName";
    roles[Node] = "node";
    roles[Priority] = "priority";
    roles[PluginIcon] = "pluginIcon";
    roles[PluginName] = "pluginName";
    return roles;
}
