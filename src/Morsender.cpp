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

#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

#include <sailfishapp.h>
#include <QtQml>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <qqml.h>
#include <QtQuick/qquickitem.h>
#include <QtQuick/qquickview.h>
#include "buddymodel.h"
#include "accountsmodel.h"
#include "accountsoptionsmodel.h"
#include "chatmodel.h"
#include "defines.h"
#include "connection.h"

#include "purple.h"

#include <glib.h>

#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>

/**
 * The following eventloop functions are used in both pidgin and purple-text. If your
 * application uses glib mainloop, you can safely use this verbatim.
 */
#define PURPLE_GLIB_READ_COND  (G_IO_IN | G_IO_HUP | G_IO_ERR)
#define PURPLE_GLIB_WRITE_COND (G_IO_OUT | G_IO_HUP | G_IO_ERR | G_IO_NVAL)

typedef struct _PurpleGLibIOClosure {
    PurpleInputFunction function;
    guint result;
    gpointer data;
} PurpleGLibIOClosure;

typedef struct
{
    PurpleAccountRequestType type;
    PurpleAccount *account;
    void *ui_handle;
    char *user;
    gpointer userdata;
    PurpleAccountRequestAuthorizationCb auth_cb;
    PurpleAccountRequestAuthorizationCb deny_cb;
    guint ref;
} PurpleAccountRequestInfo;

static void purple_glib_io_destroy(gpointer data)
{
    g_free(data);
}

static gboolean purple_glib_io_invoke(GIOChannel *source, GIOCondition condition, gpointer data)
{
    PurpleGLibIOClosure *closure = (PurpleGLibIOClosure*)data;
    PurpleInputCondition purple_cond = (PurpleInputCondition)0;

    if (condition & PURPLE_GLIB_READ_COND)
        purple_cond = (PurpleInputCondition)(purple_cond | PURPLE_INPUT_READ);
    if (condition & PURPLE_GLIB_WRITE_COND)
        purple_cond = (PurpleInputCondition)(purple_cond | PURPLE_INPUT_WRITE);

    closure->function(closure->data, g_io_channel_unix_get_fd(source),
              purple_cond);

    return TRUE;
}

static guint glib_input_add(gint fd, PurpleInputCondition condition, PurpleInputFunction function,
                               gpointer data)
{
    PurpleGLibIOClosure *closure = g_new0(PurpleGLibIOClosure, 1);
    GIOChannel *channel;
    GIOCondition cond = (GIOCondition)0;

    closure->function = function;
    closure->data = data;

    if (condition & PURPLE_INPUT_READ)
        cond = (GIOCondition)(cond | PURPLE_GLIB_READ_COND);
    if (condition & PURPLE_INPUT_WRITE)
        cond = (GIOCondition)(cond | PURPLE_GLIB_WRITE_COND);

    channel = g_io_channel_unix_new(fd);
    closure->result = g_io_add_watch_full(channel, G_PRIORITY_DEFAULT, cond,
                          purple_glib_io_invoke, closure, purple_glib_io_destroy);

    g_io_channel_unref(channel);
    return closure->result;
}

static PurpleEventLoopUiOps glib_eventloops =
{
    g_timeout_add,
    g_source_remove,
    glib_input_add,
    g_source_remove,
    NULL,
#if GLIB_CHECK_VERSION(2,14,0)
    g_timeout_add_seconds,
#else
    NULL,
#endif

    /* padding */
    NULL,
    NULL,
    NULL
};
/*** End of the eventloop functions. ***/

static void ui_init(void)
{
    /**
     * This should initialize the UI components for all the modules.
     */

    purple_connections_set_ui_ops(connections_get_ui_ops());
    connection_init();

}

static PurpleCoreUiOps core_uiops =
{
    NULL,
    NULL,
    ui_init,
    NULL,

    /* padding */
    NULL,
    NULL,
    NULL,
    NULL
};

static void init_libpurple(void)
{
    /* Set a custom user directory (optional) */
//    purple_util_set_user_dir(CUSTOM_USER_DIRECTORY);

    /* We do not want any debugging for now to keep the noise to a minimum. */
//    purple_debug_set_enabled(FALSE);
    purple_debug_set_enabled(TRUE);

    /* Set the core-uiops, which is used to
     * 	- initialize the ui specific preferences.
     * 	- initialize the debug ui.
     * 	- initialize the ui components for all the modules.
     * 	- uninitialize the ui components for all the modules when the core terminates.
     */
    purple_core_set_ui_ops(&core_uiops);

    /* Set the uiops for the eventloop. If your client is glib-based, you can safely
     * copy this verbatim. */
    purple_eventloop_set_ui_ops(&glib_eventloops);

    /* Set path to search for plugins. The core (libpurple) takes care of loading the
     * core-plugins, which includes the protocol-plugins. So it is not essential to add
     * any path here, but it might be desired, especially for ui-specific plugins. */
    purple_plugins_add_search_path(CUSTOM_PLUGIN_PATH);

    /* Now that all the essential stuff has been set, let's try to init the core. It's
     * necessary to provide a non-NULL name for the current ui to the core. This name
     * is used by stuff that depends on this ui, for example the ui-specific plugins. */
    if (!purple_core_init(UI_ID)) {
        /* Initializing the core failed. Terminate. */
        fprintf(stderr,
                "libpurple initialization failed. Dumping core.\n"
                "Please report this!\n");
        abort();
    }

    purple_imgstore_init();

    /* Create and load the buddylist. */
    purple_set_blist(purple_blist_new());
    purple_blist_load();

    /* Load the preferences. */
    purple_prefs_load();

    /* Load the desired plugins. The client should save the list of loaded plugins in
     * the preferences using purple_plugins_save_loaded(PLUGIN_SAVE_PREF) */
    purple_plugins_load_saved(PLUGIN_SAVE_PREF);

    /* Load the pounces. */
    purple_pounces_load();

    purple_blist_show();

    /* Everything is good to go--sign on already */
    if (!purple_prefs_get_bool("/purple/savedstatus/startup_current_status"))
        purple_savedstatus_activate(purple_savedstatus_get_startup());
    purple_accounts_restore_current_statuses();
}

int main(int argc, char *argv[])
{
    signal(SIGCHLD, SIG_IGN);

    init_libpurple();

//    QGuiApplication::setQuitOnLastWindowClosed(false);

    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));
    QSharedPointer<QQuickView> view(SailfishApp::createView());

    ChatModel chatModel;
    BuddyModel buddyModel(&chatModel);
    AccountsModel accountsModel;

    QObject::connect(&buddyModel,
                     SIGNAL(chatSwitched(PurpleBlistNode*, PurpleAccount*)),
                     &chatModel,
                     SLOT(switchChat(PurpleBlistNode*, PurpleAccount*)));
    QObject::connect(&buddyModel,
                     SIGNAL(chatSwitched(PurpleConversation*, PurpleAccount*)),
                     &chatModel,
                     SLOT(switchChat(PurpleConversation*, PurpleAccount*)));
    QObject::connect(&buddyModel,
                     SIGNAL(chatSwitched(QString, bool, PurpleAccount*)),
                     &chatModel,
                     SLOT(switchChat(QString, bool, PurpleAccount*)));


    qmlRegisterType<AccountsOptionsModel>("com.mistermagister.options", 1, 0, "Options");
    qmlRegisterType<AccountsModel>("com.mistermagister.accounts", 1, 0, "Accounts");

    view->rootContext()->setContextProperty("buddyModel",&buddyModel);
    view->rootContext()->setContextProperty("chatModel",&chatModel);
    view->rootContext()->setContextProperty("accountsModel",&accountsModel);
    view->setSource(SailfishApp::pathTo("qml/Morsender.qml"));
    view->show();

    return app->exec();
}
