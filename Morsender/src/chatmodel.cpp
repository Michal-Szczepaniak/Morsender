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

#include "chatmodel.h"
#include "imgstore.h"
#include <QTimer>
#include <QTime>
#include <QImage>
#include <purple.h>
#include <glib.h>
#include <glib/gi18n.h>

QList<ChatModel::Chat> ChatModel::m_chats;
bool ChatModel::unreadMessages = false;

struct _PurpleStoredImage
{
    int id;
    guint8 refcount;
    size_t size;     /**< The image data's size. */
    char *filename;  /**< The filename (for the UI) */
    gpointer data;   /**< The image data. */
};

ChatModel::ChatModel()
{
    void *conversations_handle = purple_conversations_get_handle();
    void *blist_handle = purple_blist_get_handle();
    static int handle;

    purple_signal_connect(conversations_handle,
                          "chat-topic-changed",
                          &handle,
                          PURPLE_CALLBACK(
                              ChatModel::chatTopicChanged),
                          this);

    purple_signal_connect(conversations_handle,
                          "conversation-created",
                          &handle,
                          PURPLE_CALLBACK(
                              ChatModel::conversationCreatedCb),
                          this);

    purple_signal_connect(blist_handle,
                          "buddy-status-changed",
                          &handle,
                          PURPLE_CALLBACK(
                              ChatModel::buddyStatusChanged),
                          this);

//    connect(this, SIGNAL(conversationCreated(PurpleConversation*,void*)), this, SLOT(conversationWasCreated(PurpleConversation*,void*)));
}

int ChatModel::chatCount(const QModelIndex & parent) const {
    Q_UNUSED(parent);
    return m_chats.size();
}

int ChatModel::rowCount(const QModelIndex & parent) const {
    Q_UNUSED(parent);
    if(m_chats.count() == 0 || activeChat > m_chats.count() || activeChat == -1)
        return 0;

    const Chat &chat = m_chats[activeChat];
    return chat.messages.count();
}

QVariant ChatModel::data(const QModelIndex & index, int role) const {
    if(m_chats.count() == 0 || activeChat > m_chats.count())
        return QVariant();

    const Chat &chat = m_chats[activeChat];
    if (index.row() < 0 || index.row() >= chat.messages.count())
        return QVariant();

    const Message &message = chat.messages[index.row()];
    if (role == Msg)
        return message.text;
    else if (role == From)
        return message.from;
    else if (role == Timestamp)
        return message.timestamp;
    else if (role == Avatar)
        return message.avatar;
    else if (role == Image)
        return message.image;
    else if (role == Video)
        return message.video;
    else if (role == Recived)
        return message.recived;
    return QVariant();
}

QHash<int, QByteArray> ChatModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[Msg] = "message";
    roles[From] = "name";
    roles[Timestamp] = "timestamp";
    roles[Avatar] = "avatar";
    roles[Recived] = "recived";
    roles[Image] = "image";
    roles[Video] = "video";
    return roles;
}
void ChatModel::addRemoveConversation(PurpleConversation *conv)

{
    PurpleAccount *account;
    const char *name;

    account = purple_conversation_get_account(conv);
    name    = purple_conversation_get_name(conv);

    if (purple_conversation_get_type(conv) == PURPLE_CONV_TYPE_IM) {
        PurpleBuddy *b;

        b = purple_find_buddy(account, name);
        if (b != NULL) {
//            pidgin_dialogs_remove_buddy(b);
        } else if (account != NULL && purple_account_is_connected(account)) {
            purple_blist_request_add_buddy(account,purple_buddy_get_name(b),NULL,purple_buddy_get_alias(b));
//            purple_blist_request_add_buddy(account, (char *)name, NULL, NULL);
        }
    } else if (purple_conversation_get_type(conv) == PURPLE_CONV_TYPE_CHAT) {
        PurpleChat *c;

        c = purple_blist_find_chat(account, name);
        if (c != NULL) {
//            pidgin_dialogs_remove_chat(c);
        } else if (account != NULL && purple_account_is_connected(account)) {
//            purple_blist_add_chat(c,purple_chat_get_group(c),NULL);
            purple_blist_request_add_chat(account, NULL, NULL, name);
        }
    }
}

// Purple callbacks called from static methods

void ChatModel::conversationWasCreated(PurpleConversation *conv, void *data)
{
    printf("conversation-created (%s)\n",
                    purple_conversation_get_name(conv));
    ChatModel::Chat chat;
    chat.conv = conv;
    m_chats << chat;

    if(strcmp(purple_conversation_get_name(conv), activeChatName) == 0) {
        activeChat = m_chats.size()-1;
    }

    purple_conversation_set_data(conv, "chat", &chat);
    purple_conversation_set_data(conv, "chatmodel", this);
    purple_conversation_set_ui_ops(conv, conversationsGetConvUiOps());
    purple_conversation_present(conv);
    addRemoveConversation(conv);
    emit canSendFileChanged();
}

void ChatModel::buddyStatusChangedCb(PurpleBuddy *buddy, PurpleStatus *old, PurpleStatus *newstatus, void *data) {
    PurpleAccount *account = NULL;
    const char *name = NULL;

    if(m_chats.count() == 0 || activeChat < 0)
        return;

    PurpleConversation* conv = getActiveConv();

    if(conv == NULL)
        return;

    account = purple_conversation_get_account(conv);
    name = purple_conversation_get_name(conv);

    if (purple_conversation_get_type(conv) == PURPLE_CONV_TYPE_IM) {
        PurpleBuddy *b = purple_find_buddy(account, name);
        if (b != NULL && b == buddy) {
            PurplePresence *p;
            p = purple_buddy_get_presence(b);
            if (purple_presence_is_status_primitive_active(p, PURPLE_STATUS_AWAY))
                m_status = false;
            if (purple_presence_is_status_primitive_active(p, PURPLE_STATUS_UNAVAILABLE))
                m_status = false;
            if (purple_presence_is_status_primitive_active(p, PURPLE_STATUS_EXTENDED_AWAY))
                m_status = false;
            if (purple_presence_is_status_primitive_active(p, PURPLE_STATUS_OFFLINE))
                m_status = false;
            else
                m_status = true;

            emit statusChanged(m_status);
        }
    }
}

// Switch chat methods

void ChatModel::switchChat(PurpleBlistNode* node, PurpleAccount* account) {
    if(PURPLE_BLIST_NODE_IS_BUDDY(node)) {
        const char* name = purple_buddy_get_name((PurpleBuddy*)node);
        PurpleConversation* conv = purple_find_conversation_with_account(PURPLE_CONV_TYPE_IM, name, account);
        int i = 0;
        int oldChat = activeChat;
        beginResetModel();
        if(conv != NULL) {
            for(auto &chat : m_chats) {
                if(chat.conv == conv) {
                    activeChat = i;
                    activeChatName = "";
                    activeConvName = purple_buddy_get_name((PurpleBuddy*)node);
                    emit chatNameChanged(QString::fromUtf8(activeConvName));
                    emit canSendFileChanged();
                    break;
                }
                i++;
            }
        } else {
            PurpleConversation* conversation;
            conversation = purple_conversation_new(PURPLE_CONV_TYPE_IM, account, name);
            activeChat = m_chats.size()-1;
            activeConvName = purple_buddy_get_name((PurpleBuddy*)node);
            emit chatNameChanged(QString::fromUtf8(activeConvName));
            emit canSendFileChanged();
        }
    } else if(PURPLE_BLIST_NODE_IS_CHAT(node)) {
        PurpleChat* chat = (PurpleChat*)node;
        PurpleAccount *account;
        PurplePluginProtocolInfo *prpl_info;
        GHashTable *components;
        const char *name;
        char *chat_name;

        account = purple_chat_get_account((PurpleChat*)node);
        prpl_info = PURPLE_PLUGIN_PROTOCOL_INFO(purple_find_prpl(purple_account_get_protocol_id(account)));

        components = purple_chat_get_components((PurpleChat*)node);

        if (prpl_info && prpl_info->get_chat_name)
            chat_name = prpl_info->get_chat_name(components);
        else
            chat_name = NULL;

        if (chat_name)
            name = chat_name;
        else
            name = purple_chat_get_name(chat);
        PurpleConversation* conv = purple_find_conversation_with_account(PURPLE_CONV_TYPE_CHAT, name, account);
        int i = 0;
        int oldChat = activeChat;
        beginResetModel();
        if(conv != NULL) {
            for(auto &chat : m_chats) {
                if(chat.conv == conv) {
                    activeChat = i;
                    activeChatName = "";
                    activeConvName = name;
                    emit chatNameChanged(QString::fromUtf8(activeConvName));
                    emit canSendFileChanged();
                    break;
                }
                i++;
            }
        } else {
            activeChat = -1;
            activeChatName = name;
            activeConvName = name;
            emit chatNameChanged(QString::fromUtf8(activeConvName));
            joinChat((PurpleChat*)node);
        }
    }
    endResetModel();
    beginInsertRows(QModelIndex(), 0, rowCount());
    endInsertRows();
}

void ChatModel::switchChat(PurpleConversation* node, PurpleAccount* account) {
    if(purple_conversation_get_type(node) == PURPLE_CONV_TYPE_IM) {
        PurpleConversation* conv = node;
        const char* name = purple_conversation_get_name(conv);
        int i = 0;
        int oldChat = activeChat;
        beginResetModel();
        if(conv != NULL) {
            for(auto &chat : m_chats) {
                if(chat.conv == conv) {
                    activeChat = i;
                    activeChatName = "";
                    activeConvName = purple_buddy_get_name((PurpleBuddy*)node);
                    emit chatNameChanged(QString::fromUtf8(activeConvName));
                    emit canSendFileChanged();
                    break;
                }
                i++;
            }
        }
    } else if(purple_conversation_get_type(node) == PURPLE_CONV_TYPE_CHAT) {
        PurpleConversation* conv = node;
        const char* name =purple_conversation_get_name(conv);
        int i = 0;
        int oldChat = activeChat;
        beginResetModel();
        if(conv != NULL) {
            for(auto &chat : m_chats) {
                if(chat.conv == conv) {
                    activeChat = i;
                    activeChatName = "";
                    activeConvName = name;
                    emit chatNameChanged(QString::fromUtf8(activeConvName));
                    emit canSendFileChanged();
                    break;
                }
                i++;
            }
        }
    }
    endResetModel();
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    endInsertRows();
}

void ChatModel::switchChat(QString username, bool chat, PurpleAccount* account) {
    if(!chat) {
        PurpleConversation* conv = purple_find_conversation_with_account(PURPLE_CONV_TYPE_IM, username.toStdString().c_str(), account);
        int i = 0;
        int oldChat = activeChat;
        beginResetModel();
        if(conv != NULL) {
            for(auto &chat : m_chats) {
                if(chat.conv == conv) {
                    activeChat = i;
                    activeChatName = "";
                    activeConvName = username.toStdString().c_str();
                    emit chatNameChanged(QString::fromUtf8(activeConvName));
                    emit canSendFileChanged();
                    break;
                }
                i++;
            }
        } else {
            PurpleConversation* conversation;
            conversation = purple_conversation_new(PURPLE_CONV_TYPE_IM, account, username.toStdString().c_str());
            activeChat = m_chats.size()-1;
            activeConvName = username.toStdString().c_str();
            emit chatNameChanged(QString::fromUtf8(activeConvName));
            emit canSendFileChanged();
        }
    } else {
        PurpleAccount *account;
        const char *name;
        PurpleConversation* conv = purple_find_conversation_with_account(PURPLE_CONV_TYPE_CHAT, username.toStdString().c_str(), account);
        int i = 0;
        int oldChat = activeChat;
        beginResetModel();
        if(conv != NULL) {
            for(auto &chat : m_chats) {
                if(chat.conv == conv) {
                    activeChat = i;
                    activeChatName = "";
                    activeConvName = username.toStdString().c_str();
                    emit chatNameChanged(QString::fromUtf8(activeConvName));
                    emit canSendFileChanged();
                    break;
                }
                i++;
            }
        } else {
//            activeChat = -1;
//            activeChatName = username;
//            activeConvName = username;
//            emit chatNameChanged(QString::fromUtf8(activeConvName));
//            joinChat((PurpleChat*)node);
        }
    }
    endResetModel();
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    endInsertRows();
}

void ChatModel::joinChat(PurpleChat *chat)
{
    PurpleAccount *account;
    PurpleConversation *conv;
    PurplePluginProtocolInfo *prpl_info;
    GHashTable *components;
    const char *name;
    char *chat_name;

    account = purple_chat_get_account(chat);
    prpl_info = PURPLE_PLUGIN_PROTOCOL_INFO(purple_find_prpl(purple_account_get_protocol_id(account)));

    components = purple_chat_get_components(chat);

    if (prpl_info && prpl_info->get_chat_name)
        chat_name = prpl_info->get_chat_name(components);
    else
        chat_name = NULL;

    if (chat_name)
        name = chat_name;
    else
        name = purple_chat_get_name(chat);

    conv = purple_find_conversation_with_account(PURPLE_CONV_TYPE_CHAT, name,
                                                 account);

    if (conv != NULL) {
//        pidgin_conv_attach_to_conversation(conv);
        purple_conversation_present(conv);
    }

    serv_join_chat(purple_account_get_connection(account), components);
    g_free(chat_name);
}

// Recive/send messages

void ChatModel::addMessage(const Message &message) {
    if(activeChat > m_chats.count() || activeChat < 0)
        return;

    Chat &chat = m_chats[activeChat];
    beginInsertRows(QModelIndex(), 0, 0);
    chat.messages.push_front(message);
    endInsertRows();
}

void ChatModel::addMessageSilent(const Message &message, int index) {
    if(index > m_chats.count() || activeChat < 0)
        return;

    Chat &chat = m_chats[index];
    chat.messages.push_front(message);
}

void ChatModel::sendMessage(QString message) {
    if(activeChat > m_chats.count() || activeChat < 0)
        return;

    PurpleConversation* conv = getActiveConv();
    const char *text = message.toStdString().c_str();
    if (*text == '/' && *(text + 1) != '/')
    {
        PurpleCmdStatus status;
        const char *cmdline = text + 1;
        char *error = NULL, *escape;

        escape = g_markup_escape_text(cmdline, -1);
        status = purple_cmd_do_command(conv, cmdline, escape, &error);
        g_free(escape);

        switch (status)
        {
            case PURPLE_CMD_STATUS_OK:
                break;
            case PURPLE_CMD_STATUS_NOT_FOUND:
                purple_conversation_write(conv, "", _("No such command."),
                        PURPLE_MESSAGE_NO_LOG, time(NULL));
                break;
            case PURPLE_CMD_STATUS_WRONG_ARGS:
                purple_conversation_write(conv, "", _("Syntax Error:  You typed the wrong number of arguments "
                            "to that command."),
                        PURPLE_MESSAGE_NO_LOG, time(NULL));
                break;
            case PURPLE_CMD_STATUS_FAILED:
                purple_conversation_write(conv, "", error ? error : _("Your command failed for an unknown reason."),
                        PURPLE_MESSAGE_NO_LOG, time(NULL));
                break;
            case PURPLE_CMD_STATUS_WRONG_TYPE:
                if(purple_conversation_get_type(conv) == PURPLE_CONV_TYPE_IM)
                    purple_conversation_write(conv, "", _("That command only works in chats, not IMs."),
                            PURPLE_MESSAGE_NO_LOG, time(NULL));
                else
                    purple_conversation_write(conv, "", _("That command only works in IMs, not chats."),
                            PURPLE_MESSAGE_NO_LOG, time(NULL));
                break;
            case PURPLE_CMD_STATUS_WRONG_PRPL:
                purple_conversation_write(conv, "", _("That command doesn't work on this protocol."),
                        PURPLE_MESSAGE_NO_LOG, time(NULL));
                break;
        }
        g_free(error);
    }
    else if (!purple_account_is_connected(purple_conversation_get_account(conv)))
    {
        purple_conversation_write(conv, "", _("Message was not sent, because you are not signed on."),
                (PurpleMessageFlags)(PURPLE_MESSAGE_ERROR | PURPLE_MESSAGE_NO_LOG), time(NULL));
    }
    else
    {
        char *escape = purple_markup_escape_text((*text == '/' ? text + 1 : text), -1);
        switch (purple_conversation_get_type(conv))
        {
            case PURPLE_CONV_TYPE_IM:
                purple_conv_im_send_with_flags(PURPLE_CONV_IM(conv), escape, PURPLE_MESSAGE_SEND);
                break;
            case PURPLE_CONV_TYPE_CHAT:
                purple_conv_chat_send(PURPLE_CONV_CHAT(conv), escape);
                break;
            default:
                g_free(escape);
                g_return_if_reached();
        }
        g_free(escape);
        purple_idle_touch();
    }
}

bool ChatModel::canSendFile() {
    PurpleConversation* conv = getActiveConv();

    if(activeChat == -1 || conv == NULL) return false;

    PurplePluginProtocolInfo *prpl_info = PURPLE_PLUGIN_PROTOCOL_INFO(purple_conversation_get_gc(conv)->prpl);
    return prpl_info && prpl_info->send_file;
}

void ChatModel::sendFile(QString file) {
//    PurplePluginProtocolInfo *prpl_info = PURPLE_PLUGIN_PROTOCOL_INFO(purple_conversation_get_gc(getActiveConv())->prpl);

    PurpleConversation* conv = getActiveConv();

    g_return_if_fail(conv != NULL);

    PurpleConnection *gc  = purple_conversation_get_gc(conv);
    const char *who = purple_conversation_get_name(conv);

    g_return_if_fail(gc != NULL);

    serv_send_file(gc, who, file.toStdString().c_str());
}

// Handling recived/sent messages

void ChatModel::convWriteConv(PurpleConversation *conv, const char *name, const char *alias,
                               const char *message, PurpleMessageFlags flags,
                               time_t mtime) {
    ChatModel::Chat* chat;
    ChatModel* chatModel;

    g_return_if_fail(conv != NULL);
    chat = (ChatModel::Chat*)purple_conversation_get_data(conv, "chat");
    g_return_if_fail(chat != NULL);
    chatModel = (ChatModel*)purple_conversation_get_data(conv, "chatmodel");
    g_return_if_fail(chatModel != NULL);

    if ((flags & PURPLE_MESSAGE_SYSTEM) && !(flags & PURPLE_MESSAGE_NOTIFY)) {
        flags = (PurpleMessageFlags) (flags & ~(PURPLE_MESSAGE_SEND | PURPLE_MESSAGE_RECV));
    }
    chatModel->writeConvMsg(conv,name,alias,message,flags,mtime);
}

void ChatModel::convWriteIm(PurpleConversation *conv, const char *who,
                          const char *message, PurpleMessageFlags flags,
                          time_t mtime) {
    g_return_if_fail(conv != NULL);
    PurpleAccount *account = purple_conversation_get_account(conv);
    if (flags & PURPLE_MESSAGE_SEND)
    {
        who = purple_connection_get_display_name(purple_account_get_connection(account));
        if (!who)
            who = purple_account_get_alias(account);
        if (!who)
            who = purple_account_get_username(account);
    }
    else if (flags & PURPLE_MESSAGE_RECV)
    {
    }

    purple_conversation_write(conv, who, message, flags, mtime);
}

void ChatModel::writeConvMsg(PurpleConversation *conv, const char *name, const char *alias,
                  const char *message, PurpleMessageFlags flags,
                  time_t mtime) {
    int i = 0;
    PurpleAccount* account = purple_conversation_get_account(conv);
    PurpleBuddy *buddy;
    const char *who;
    const char *start, *end;
    char *idstr = NULL, *videoPath = NULL;
    int imgId = 0;
    GData *attributes;

    who = purple_conversation_get_name(conv);
    buddy = purple_find_buddy(account, name);

    if(purple_markup_find_tag("img",message,&start,&end,&attributes)) {
        gpointer id = g_datalist_get_data(&attributes, "id");
        idstr = (char*)id;
        if (idstr != NULL)
            imgId = atoi(idstr);
    }

    const char* begin;
    size_t animLen = strlen("[animation");
    if((begin = strstr(message, "[animation")) != NULL) { //strncmp("[animation", message, strlen("[animation")) == 0
        if(begin == message) {
            if(purple_markup_find_tag("a",message+animLen,&start,&end,&attributes)) {
                char* path = (char*)g_datalist_get_data(&attributes, "href");
                const char* suffix = ".mp4";
                size_t lenstr = strlen(path);
                size_t lensuffix = strlen(suffix);
                if (lensuffix >  lenstr)
                   videoPath = NULL;
                if(strncmp(path + lenstr - lensuffix, suffix, lensuffix) == 0)
                    videoPath = (char*)path;
                message = "";
            }
        } else {
            char* msg = strdup(message);
            char* newMsg = (char*) malloc(begin-message+1);
            strncpy(newMsg, msg, begin-message);
            *(newMsg+((size_t)(begin-message))) = '\0';
            message = strdup(newMsg);

            if(purple_markup_find_tag("a",begin+animLen,&start,&end,&attributes)) {
                char* path = (char*)g_datalist_get_data(&attributes, "href");
                const char* suffix = ".mp4";
                size_t lenstr = strlen(path);
                size_t lensuffix = strlen(suffix);
                if (lensuffix >  lenstr)
                   videoPath = NULL;
                if(strncmp(path + lenstr - lensuffix, suffix, lensuffix) == 0)
                    videoPath = (char*)path;
            }
        }
    }

    message = purple_markup_strip_html(message);

    for(auto &chat : m_chats) {
        if(chat.conv == conv) {
            Message newMessage;
            if(purple_conversation_get_type(conv) == PURPLE_CONV_TYPE_IM)
                newMessage.from = flags & PURPLE_MESSAGE_SEND ? "Me" : QString::fromUtf8(purple_conversation_get_title(conv));
            else if(buddy)
                newMessage.from = flags & PURPLE_MESSAGE_SEND ? "Me" : QString::fromUtf8(purple_buddy_get_contact_alias(buddy));
            else
                newMessage.from = flags & PURPLE_MESSAGE_SEND ? "Me" : QString::fromUtf8(name);

            newMessage.text = QString::fromUtf8(message);

            _PurpleStoredImage* image = purple_imgstore_find_by_id(imgId);
            gchar *filename;
            if(image) {
                GError *err = NULL;
                filename = g_build_filename (g_get_tmp_dir(), std::to_string(imgId).c_str(), NULL) ;
                gconstpointer data = purple_imgstore_get_data (image);
                g_file_set_contents (filename, (const char*)data, purple_imgstore_get_size (image), &err);
            }

            newMessage.recived = flags & PURPLE_MESSAGE_RECV ? true : false;
            newMessage.timestamp = QTime::currentTime().toString("hh:mm AP");
            newMessage.image = "";
            newMessage.video = "";

            PurpleBuddyIcon* icon = NULL;
            if(buddy)
                icon = purple_buddy_get_icon(buddy);

            if(image) {
                newMessage.image = QString::fromUtf8(filename);
            }

            if(videoPath) {
                newMessage.video = QString::fromUtf8(videoPath+strlen("file:///"));
            }

            if(buddy && icon)
                newMessage.avatar = QString::fromUtf8(purple_buddy_icon_get_full_path(icon));
            else
                newMessage.avatar = "";

            if(activeChat == i) {
                addMessage(newMessage);
                if(flags & PURPLE_MESSAGE_RECV)
                    emit messageRecived(QString::fromUtf8(purple_conversation_get_title(conv)), QString::fromUtf8(message), QString::fromUtf8(purple_conversation_get_name(conv)), purple_conversation_get_type(conv) == PURPLE_CONV_TYPE_IM);
            } else {
                if(flags & PURPLE_MESSAGE_RECV)
                    emit messageRecived(QString::fromUtf8(purple_conversation_get_title(conv)), QString::fromUtf8(message), QString::fromUtf8(purple_conversation_get_name(conv)), purple_conversation_get_type(conv) == PURPLE_CONV_TYPE_IM);
                addMessageSilent(newMessage, i);
            }
            ChatModel::unreadMessages = true;
            break;
        }
        i++;
    }
}

// Getters and Setters

PurpleConversation* ChatModel::getActiveConv() {
    if(activeChat < 0 || activeChat >= m_chats.count())
        return NULL;
    else
        return m_chats[activeChat].conv;
}

PurpleConversation* ChatModel::getConv(int index) {
    if(index < m_chats.count() && index >= 0)
        return m_chats[index].conv;
    else
        return NULL;
}

void ChatModel::removeConv(int index) {
    return m_chats.removeAt(index);
}

PurpleAccount* ChatModel::getActiveConvAccount() {
    if(activeChat < 0 || activeChat >= m_chats.count())
        return NULL;
    else
        return purple_conversation_get_account(m_chats[activeChat].conv);
}

PurpleConversationUiOps* ChatModel::conversationsGetConvUiOps() {
    static PurpleConversationUiOps conversationUiOps =
    {
        NULL,//pidgin_conv_new,
        NULL,//pidgin_conv_destroy,              /* destroy_conversation */
        NULL,                              /* write_chat           */
        ChatModel::convWriteIm,//purple_conversation_write,             /* write_im             */
        ChatModel::convWriteConv,//pidgin_conv_write_conv,           /* write_conv           */
        ChatModel::chatUsersChangedCb,//pidgin_conv_chat_add_users,       /* chat_add_users       */
        NULL,//pidgin_conv_chat_rename_user,     /* chat_rename_user     */
        NULL,//pidgin_conv_chat_remove_users,    /* chat_remove_users    */
        NULL,//pidgin_conv_chat_update_user,     /* chat_update_user     */
        NULL,//pidgin_conv_present_conversation, /* present              */
        NULL,//pidgin_conv_has_focus,            /* has_focus            */
        NULL,//pidgin_conv_custom_smiley_add,    /* custom_smiley_add    */
        NULL,//pidgin_conv_custom_smiley_write,  /* custom_smiley_write  */
        NULL,//pidgin_conv_custom_smiley_close,  /* custom_smiley_close  */
        NULL,//pidgin_conv_send_confirm,         /* send_confirm         */
        NULL,
        NULL,
        NULL,
        NULL
    };
    return &conversationUiOps;
}

bool ChatModel::status() {
    return getStatus(getActiveConv());
}

bool ChatModel::status(int index) {
    if(index < 0 || index >= m_chats.count())
        return false;
    else
        return getStatus(m_chats[index].conv);
}

QString ChatModel::topic() {
    PurpleConversation* conv = getActiveConv();

    if(m_chats.count() == 0 || activeChat < 0 || conv == NULL)
        return false;

    PurpleConvChat* convChat = purple_conversation_get_chat_data(conv);
    const char* topic = purple_conv_chat_get_topic(convChat);
    if(!topic)
        topic = "";

    return QString::fromUtf8(topic);
}

QString ChatModel::topic(int index) {
    PurpleConversation* conv = getConv(index);

    if(conv == NULL) return QString("");

    PurpleConvChat* convChat = purple_conversation_get_chat_data(conv);
    const char* topic = purple_conv_chat_get_topic(convChat);
    if(!topic)
        topic = "";

    return QString::fromUtf8(topic);
}

QString ChatModel::getChatName() {
    return QString::fromUtf8(activeConvName);
}

void ChatModel::setChatName(QString name) {
    activeConvName = name.toStdString().c_str();
}

bool ChatModel::getStatus(PurpleConversation* conv) {
    PurpleAccount *account = NULL;
    const char *name = NULL;

    if(conv == NULL)
        return false;

    account = purple_conversation_get_account(conv);
    name = purple_conversation_get_name(conv);

    if (purple_conversation_get_type(conv) == PURPLE_CONV_TYPE_IM) {
        PurpleBuddy *b = purple_find_buddy(account, name);
        return PURPLE_BUDDY_IS_ONLINE(b) ? "1": "2";
    }
    return true;
}

QVariantList ChatModel::getCurrentChatUsers() {
    PurpleConversation* conv = getActiveConv();
    QVariantList users_names;

    if(!conv) return users_names;

    GList* users = purple_conv_chat_get_users(purple_conversation_get_chat_data(conv));

    GList *l = users;
    while (l != NULL) {
        PurpleConvChatBuddy* buddy = (PurpleConvChatBuddy*)l->data;
        const char* name = NULL;
        QVariantList user;

        name = buddy->alias;
        if(!name)
            name = buddy->name;

        user << name;
        user << buddy->name;

        users_names << QVariant::fromValue(user);

        l = l->next;
    }

    return users_names;
}

void ChatModel::chatUsersChangedCb(PurpleConversation *conv, GList *cbuddies, gboolean new_arrivals) {
    ChatModel* chatModel;

    chatModel = (ChatModel*)purple_conversation_get_data(conv, "chatmodel");

    g_return_if_fail(chatModel != NULL);

    if(conv == chatModel->getActiveConv())
        emit chatModel->chatUsersChanged();
}

// Static metods that call methods on class object used as C callbacks

void ChatModel::conversationCreatedCb(PurpleConversation *conv, void *data) {
    ChatModel* chatModel = (ChatModel*)data;
    emit chatModel->conversationCreated(conv, data);
}

void ChatModel::buddyStatusChanged(PurpleBuddy *buddy, PurpleStatus *old, PurpleStatus *newstatus, void *data) {
    ChatModel* chatModel;
    chatModel = (ChatModel*)data;
    chatModel->buddyStatusChangedCb(buddy,old,newstatus,data);
}

void ChatModel::chatTopicChanged(PurpleConversation *conv, const char *who,
                                 const char *topic, void *data) {
    ChatModel* chatModel = (ChatModel*)data;
    if(purple_conversation_get_type(conv) == PURPLE_CONV_TYPE_CHAT && conv == chatModel->getActiveConv())
        emit chatModel->topicChanged(QString::fromUtf8(topic));
}
