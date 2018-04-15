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

#include "connection.h"
#include "defines.h"

#define INITIAL_RECON_DELAY_MIN  8000
#define INITIAL_RECON_DELAY_MAX 60000

#define MAX_RECON_DELAY 600000
#define MAX_RACCOON_DELAY "shorter in urban areas"

typedef struct {
    int delay;
    guint timeout;
} AutoRecon;

/**
 * Contains accounts that are auto-reconnecting.
 * The key is a pointer to the PurpleAccount and the
 * value is a pointer to a PidginAutoRecon.
 */
static GHashTable *auto_reconns = NULL;

static void
connection_connect_progress(PurpleConnection *gc,
        const char *text, size_t step, size_t step_count)
{
//  TODO: Set connecting icon
//
//    PidginBuddyList *gtkblist = blist_get_default_gtk_blist();
//    if (!gtkblist)
//        return;
//    status_box_set_connecting(STATUS_BOX(gtkblist->statusbox),
//                       (purple_connections_get_connecting() != NULL));
//    status_box_pulse_connecting(STATUS_BOX(gtkblist->statusbox));
}

static void
connection_connected(PurpleConnection *gc)
{
    PurpleAccount *account;
//    PidginBuddyList *gtkblist;

    account  = purple_connection_get_account(gc);

//  TODO: Remove connecting icon
//
//    gtkblist = blist_get_default_gtk_blist();

//    if (gtkblist != NULL)
//        status_box_set_connecting(STATUS_BOX(gtkblist->statusbox),
//                       (purple_connections_get_connecting() != NULL));

    g_hash_table_remove(auto_reconns, account);
}

static void
connection_disconnected(PurpleConnection *gc)
{
//  TODO: Add connecting icon
//
//    PidginBuddyList *gtkblist = blist_get_default_gtk_blist();
//    if (!gtkblist)
//        return;
//    status_box_set_connecting(STATUS_BOX(gtkblist->statusbox),
//                       (purple_connections_get_connecting() != NULL));

    if (purple_connections_get_all() != NULL)
        return;

//    dialogs_destroy_all();
}

static void
free_auto_recon(gpointer data)
{
    AutoRecon *info = (AutoRecon*)data;

    if (info->timeout != 0)
        g_source_remove(info->timeout);

    g_free(info);
}

static gboolean
do_signon(gpointer data)
{
    PurpleAccount *account = (PurpleAccount*)data;
    AutoRecon *info;
    PurpleStatus *status;

    purple_debug_info("autorecon", "do_signon called\n");
    g_return_val_if_fail(account != NULL, FALSE);
    info = (AutoRecon*)g_hash_table_lookup(auto_reconns, account);

    if (info)
        info->timeout = 0;

    status = purple_account_get_active_status(account);
    if (purple_status_is_online(status))
    {
        purple_debug_info("autorecon", "calling purple_account_connect\n");
        purple_account_connect(account);
        purple_debug_info("autorecon", "done calling purple_account_connect\n");
    }

    return FALSE;
}

static void
connection_report_disconnect_reason (PurpleConnection *gc,
                                            PurpleConnectionError reason,
                                            const char *text)
{
    PurpleAccount *account = NULL;
    AutoRecon *info;

    account = purple_connection_get_account(gc);
    info = (AutoRecon*)g_hash_table_lookup(auto_reconns, account);

    if (!purple_connection_error_is_fatal (reason)) {
        if (info == NULL) {
            info = g_new0(AutoRecon, 1);
            g_hash_table_insert(auto_reconns, account, info);
            info->delay = g_random_int_range(INITIAL_RECON_DELAY_MIN, INITIAL_RECON_DELAY_MAX);
        } else {
            info->delay = MIN(2 * info->delay, MAX_RECON_DELAY);
            if (info->timeout != 0)
                g_source_remove(info->timeout);
        }
        info->timeout = g_timeout_add(info->delay, do_signon, account);
    } else {
        if (info != NULL)
            g_hash_table_remove(auto_reconns, account);

        purple_account_set_enabled(account, UI_ID, FALSE);
    }
}

static void connection_network_connected (void)
{
    GList *list, *l;
//  TODO: Add network available?
//    PidginBuddyList *gtkblist = blist_get_default_gtk_blist();

//    if(gtkblist)
//        status_box_set_network_available(STATUS_BOX(gtkblist->statusbox), TRUE);

    l = list = purple_accounts_get_all_active();
    while (l) {
        PurpleAccount *account = (PurpleAccount*)l->data;
        g_hash_table_remove(auto_reconns, account);
        if (purple_account_is_disconnected(account))
            do_signon(account);
        l = l->next;
    }
    g_list_free(list);
}

static void connection_network_disconnected (void)
{
    GList *list, *l;
//  TODO: Add network unavilable
//    PidginBuddyList *gtkblist = blist_get_default_gtk_blist();

//    if(gtkblist)
//        status_box_set_network_available(STATUS_BOX(gtkblist->statusbox), FALSE);

    l = list = purple_accounts_get_all_active();
    while (l) {
        PurpleAccount *a = (PurpleAccount*)l->data;
        if (!purple_account_is_disconnected(a)) {
            char *password = g_strdup(purple_account_get_password(a));
            purple_account_disconnect(a);
            purple_account_set_password(a, password);
            g_free(password);
        }
        l = l->next;
    }
    g_list_free(list);
}

static void connection_notice(PurpleConnection *gc, const char *text)
{ }

static PurpleConnectionUiOps conn_ui_ops =
{
    connection_connect_progress,
    connection_connected,
    connection_disconnected,
    connection_notice,
    NULL, /* report_disconnect */
    connection_network_connected,
    connection_network_disconnected,
    connection_report_disconnect_reason,
    NULL,
    NULL,
    NULL
};

PurpleConnectionUiOps *
connections_get_ui_ops(void)
{
    return &conn_ui_ops;
}

static void
account_removed_cb(PurpleAccount *account, gpointer user_data)
{
    g_hash_table_remove(auto_reconns, account);
}

void *
connection_get_handle(void)
{
    static int handle;

    return &handle;
}

void
connection_init(void)
{
    auto_reconns = g_hash_table_new_full(
                            g_direct_hash, g_direct_equal,
                            NULL, free_auto_recon);

    purple_signal_connect(purple_accounts_get_handle(), "account-removed",
                        connection_get_handle(),
                        PURPLE_CALLBACK(account_removed_cb), NULL);
}

void
connection_uninit(void)
{
    purple_signals_disconnect_by_handle(connection_get_handle());

    g_hash_table_destroy(auto_reconns);
}
