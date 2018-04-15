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

#ifndef CONNECTION_H
#define CONNECTION_H
#include <glib.h>
#include <purple.h>

/**
 * Gets GTK+ Connection UI ops
 *
 * @return UI operations struct
 */
PurpleConnectionUiOps *connections_get_ui_ops(void);

/**
 * Returns the GTK+ connection handle.
 *
 * @return The handle to the GTK+ connection system.
 */
void *connection_get_handle(void);

/**
 * Initializes the GTK+ connection system.
 */
void connection_init(void);

/**
 * Uninitializes the GTK+ connection system.
 */
void connection_uninit(void);

#endif // CONNECTION_H
