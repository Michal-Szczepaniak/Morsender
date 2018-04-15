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

#ifndef CHATMODEL_H
#define CHATMODEL_H

#include <QAbstractListModel>
#include <purple.h>
#include <QImage>

class ChatModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool status READ status NOTIFY statusChanged)
    Q_PROPERTY(bool fileSendingEnabled READ canSendFile NOTIFY canSendFileChanged)
    Q_PROPERTY(QString topic READ topic NOTIFY topicChanged)
    Q_PROPERTY(QString chatName READ getChatName WRITE setChatName NOTIFY chatNameChanged)
    Q_PROPERTY(QVariantList chatUsers READ getCurrentChatUsers NOTIFY chatUsersChanged)
public:
    /**
     * @brief ChatModel Setup libpurple callbacks
     */
    ChatModel();

    enum ChatRoles {
        Msg = Qt::UserRole + 1,
        From,
        Timestamp,
        Avatar,
        Recived,
        Image,
        Video,
        Status
    };

    struct Message {
       QString  text;
       QString  from;
       QString  timestamp;
       QString  avatar;
       QString  image;
       QString  video;
       bool     recived;
    };

    struct Chat {
        QList<Message> messages;
        PurpleConversation* conv;
    };

    // Getters

    /**
     * @brief status Get status of active convesation. Used by QML
     * @return bool Buddy status (online/offine), Chats always online
     */
    bool status();

    /**
     * @brief status Get status of active convesation. Used by QML
     * @param index Chat index
     * @return bool Buddy status (online/offine), Chats always online
     */
    bool status(int index);

    /**
     * @brief topic Get topic of active conversation. Used by QML
     * @return QString Topic name
     */
    QString topic();

    /**
     * @brief topic Get topic of active conversation. Used by QML
     * @param index Chat index
     * @return QString Topic name
     */
    QString topic(int index);

    /**
     * @brief getChatName Get active chat name. Used in QML
     * @return QString chat name
     */
    QString getChatName();

    /**
     * @brief getStatus
     * @param conv PurpleConversation* Conversation with buddy or chat
     * @return bool Buddy status (online/offine), Chats always online
     */
    bool getStatus(PurpleConversation* conv);

    /**
     * @brief getActiveConv Get active convesation
     * @return PurpleConversation* Active conversation
     */
    PurpleConversation* getActiveConv();

    /**
     * @brief getConv Get convesation with id
     * @return PurpleConversation* conversation
     */
    PurpleConversation* getConv(int index);

    /**
     * @brief removeConv Remove convesation with id
     */
    void removeConv(int index);

    /**
     * @brief getActiveConvAccount Get active convesation account
     * @return PurpleAccount* Active conversation account
     */
    Q_INVOKABLE PurpleAccount* getActiveConvAccount();

    /**
     * @brief addMessage Add message to current chat
     * @param message Message content
     */
    void addMessage(const Message &message);

    /**
     * @brief addMessageSilent Add message to not active chat (do not wake up qml)
     * @param message Message content
     * @param index Index of chat
     */
    void addMessageSilent(const Message &message, int index);

    /**
     * @brief sendMessage Sends message to chat. Called from GUI
     * @param message Message content
     */
    Q_INVOKABLE void sendMessage(QString message);

    /**
     * @brief canSendFile Check if can send file over this protocol. Called from gui
     */
    Q_INVOKABLE bool canSendFile();

    // Data manipulation methods

    /**
     * @brief rowCount Get number of messages in opened chat
     * @param parent
     * @return Number of messages in opened chat
     */
    int chatCount(const QModelIndex & parent = QModelIndex()) const;

    /**
     * @brief rowCount Get number of messages in opened chat
     * @param parent
     * @return Number of messages in opened chat
     */
    int rowCount(const QModelIndex & parent = QModelIndex()) const;

    /**
     * @brief data Get data at specific index
     * @param index Index
     * @param role Role id
     * @return Value at specific index and role
     */
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

    /**
     * @brief getCurrentChatUsers Get current chat users list. Called from gui
     */
    QVariantList getCurrentChatUsers();

    /**
     * @brief chatUsersChangedCb Users in chat changed Purple Callback
     * @param conv Coversation
     */
    static void chatUsersChangedCb(PurpleConversation *conv, GList *cbuddies, gboolean new_arrivals);

    /**
     * @brief sendFile Send file over this protocol. Called from gui
     * @param file Path to the file to send
     */
    Q_INVOKABLE void sendFile(QString file);

    /**
     * @brief chatTopicChanged Purple callback called when conversation topic was chaned
     * @param conv PurpleConversation* Purple convesation with changed topic
     * @param who Old topic
     * @param topic New topic
     * @param data ChatModel object
     */
    static void chatTopicChanged(PurpleConversation *conv, const char *who,
                                 const char *topic, void *data);

    /**
     * @brief convWriteIm Static method for handling Purple callback after writting IM
     * @param conv PurpleCoversation* Conversation that message was written to
     * @param who Sender name
     * @param message Message content
     * @param flags Purple message flags (recived, send etc…)
     * @param mtime Timestamp of the message
     */
    static void convWriteIm(PurpleConversation *conv, const char *who,
                          const char *message, PurpleMessageFlags flags,
                          time_t mtime);

    /**
     * @brief convWriteConv Static method for handling Purple callback after writting to conversation (called by purple_conversation_write in convWriteIm)
     * @param conv PurpleCoversation* Conversation that message was written to
     * @param name Coversation name
     * @param alias Conversation alias
     * @param message Message content
     * @param flags Purple message flags (recived, send etc…)
     * @param mtime Timestamp of the message
     */
    static void convWriteConv(PurpleConversation *conv, const char *name, const char *alias,
                                const char *message, PurpleMessageFlags flags,
                                time_t mtime);

    /**
     * @brief writeConvMsg Handle message written to conversation in GUI
     * @param conv PurpleCoversation* Conversation that message was written to
     * @param name Coversation name
     * @param alias Conversation alias
     * @param message Message content
     * @param flags Purple message flags (recived, send etc…)
     * @param mtime Timestamp of the message
     *
     * TODO: Add images handling
     */
    void writeConvMsg(PurpleConversation *conv, const char *name, const char *alias,
                      const char *message, PurpleMessageFlags flags,
                      time_t mtime);


    /**
     * @brief conversationCreated Calls conversationCreatedCallback
     * @param conv PurpleConversation* Conversation that was created
     * @param data ChatModel Object
     */
    static void conversationCreatedCb(PurpleConversation *conv, void *data);

    /**
     * @brief buddyStatusChanged Calls buddyStatusChangedCb
     * @param buddy PurpleBuddy* Buddy that changed stasus
     * @param old Old status
     * @param newstatus New status
     * @param data ChatModel Object
     */
    static void buddyStatusChanged(PurpleBuddy *buddy, PurpleStatus *old, PurpleStatus *newstatus, void *data);

    /**
     * @brief buddyStatusChangedCb Handle buddy status changing in GUI
     * @param buddy PurpleBuddy* Buddy that changed stasus
     * @param old Old status
     * @param newstatus New status
     * @param data ChatModel object
     */
    void buddyStatusChangedCb(PurpleBuddy *buddy, PurpleStatus *old, PurpleStatus *newstatus, void *data);

protected:
    /**
     * @brief roleNames Get role names
     * @return Role names
     */
    QHash<int, QByteArray> roleNames() const;

private:
    static QList<Chat> m_chats;

    int activeChat = 0;
    const char* activeChatName = ""; // Used to connect to chat async
    const char* activeConvName = ""; // Used for chat name
    static bool unreadMessages;
    bool m_status = false;
    QString m_topic = "";

    /**
     * @brief joinChat Join the chat helper function
     * @param chat PurpleChat chat
     */
    void joinChat(PurpleChat *chat);

    /**
     * @brief addRemoveConversation Add/Remove purple conversation
     * @param conv PurpleConversation
     *
     * TODO: Add removing chats
     */
    void addRemoveConversation(PurpleConversation *conv);

    /**
     * @brief conversationsGetConvUiOps Get Purple UI Ops
     * @return PurpleConversationUiOps* UI Ops
     */
    PurpleConversationUiOps* conversationsGetConvUiOps();


signals:
    void statusChanged(bool status);
    void topicChanged(QString topic);
    void chatNameChanged(QString name);
    void canSendFileChanged();
    void messageRecived(QString name, QString message, QString username, bool type);

    /**
     * @brief conversationCreated conversation creation signal
     * @param conv PurpleCoversation that was created
     * @param data ChatModel object
     */
    void conversationCreated(PurpleConversation *conv, void *data);

    /**
     * @brief chatUsersChanged Users in current chat changed
     */
    void chatUsersChanged();

public slots:
    /**
     * @brief setChatName Set active chat name
     * @param name Active chat name
     */
    void setChatName(QString name);

    /**
     * @brief switchChat Called when switching chats in gui. Resets model and reads messages from conversation
     * @param node Node from the buddy list that we are switching to
     * @param account PurpleAccount with that node
     */
    void switchChat(PurpleBlistNode* node, PurpleAccount* account);
    void switchChat(PurpleConversation* node, PurpleAccount* account);
    void switchChat(QString username, bool chat, PurpleAccount* account);

    /**
     * @brief conversationWasCreated slot on conversation creation
     * @param conv PurpleCoversation that was created
     * @param data ChatModel object
     */
    void conversationWasCreated(PurpleConversation *conv, void *data);
};

Q_DECLARE_METATYPE(PurpleAccount*)
Q_DECLARE_METATYPE(PurpleConversation*)

#endif // CHATMODEL_H
