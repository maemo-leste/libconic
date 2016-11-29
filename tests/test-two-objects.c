/*
  libconic - Internet Connectivity library
  
  Copyright (C) 2006 Nokia Corporation. All rights reserved.

  Contact: Patrik Flykt <patrik.flykt@nokia.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License
  version 2.1 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
  02110-1301 USA
*/

#include <string.h>

#include <glib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>

#include <conic.h>

#define USER_DATA_MAGIC 0xacdcacdc
#define USER_DATA_MAGIC2 0xabababab

static GMainLoop *loop;
static DBusConnection *dbus_connection;

static gboolean connect2(gpointer data);


static void connection_cb(ConIcConnection *connection,
			  ConIcConnectionEvent *event,
			  gpointer user_data)
{
	g_debug("connection_cb(%p, %p, %p)", 
		(void *)connection, (void *)event, user_data);

	const gchar *iap_id, *bearer;
	ConIcConnectionStatus status;
	ConIcConnectionError error;

	g_assert(GPOINTER_TO_INT(user_data) == USER_DATA_MAGIC);
	g_assert(CON_IC_IS_CONNECTION_EVENT(event));

	status = con_ic_connection_event_get_status(event);
	error = con_ic_connection_event_get_error(event);
	iap_id = con_ic_event_get_iap_id(CON_IC_EVENT(event));
	bearer = con_ic_event_get_bearer_type(CON_IC_EVENT(event));
	
	switch (status) {
	case CON_IC_STATUS_CONNECTED:
		g_debug("1: CONNECTED (%s, %s, %i, %i)",
			iap_id, bearer, status, error);
		break;
	case CON_IC_STATUS_DISCONNECTED:
		g_debug("1: DISCONNECTED (%s, %s, %i, %i)",
			iap_id, bearer, status, error);
		break;
	case CON_IC_STATUS_DISCONNECTING:
		g_debug("1: DISCONNECTING (%s, %s, %i, %i)",
			iap_id, bearer, status, error);
		break;
	default:
		break;
	}
}

static void connection_cb2(ConIcConnection *connection,
			   ConIcConnectionEvent *event,
			   gpointer user_data)
{
	g_debug("connection_cb2(%p, %p, %p)", 
		(void *)connection, (void *)event, user_data);

	const gchar *iap_id, *bearer;
	ConIcConnectionStatus status;
	ConIcConnectionError error;

	g_assert(GPOINTER_TO_INT(user_data) == USER_DATA_MAGIC2);
	g_assert(CON_IC_IS_CONNECTION_EVENT(event));

	status = con_ic_connection_event_get_status(event);
	error = con_ic_connection_event_get_error(event);
	iap_id = con_ic_event_get_iap_id(CON_IC_EVENT(event));
	bearer = con_ic_event_get_bearer_type(CON_IC_EVENT(event));
	
	switch (status) {
	case CON_IC_STATUS_CONNECTED:
		g_debug("2: CONNECTED (%s, %s, %i, %i)",
			iap_id, bearer, status, error);
		break;
	case CON_IC_STATUS_DISCONNECTED:
		g_debug("2: DISCONNECTED (%s, %s, %i, %i)",
			iap_id, bearer, status, error);
		break;
	case CON_IC_STATUS_DISCONNECTING:
		g_debug("2: DISCONNECTING (%s, %s, %i, %i)",
			iap_id, bearer, status, error);
		break;
	default:
		break;
	}
}


static GMainLoop *init_glib(void)
{
#if !GLIB_CHECK_VERSION(2,35,0)
	g_type_init();
#endif
	return g_main_loop_new(NULL, FALSE);

}

static void init_dbus(void)
{
	DBusError error;
	
	dbus_error_init(&error);
	dbus_connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
	if (dbus_connection == NULL)
		g_error("Error when connecting to the session bus: %s",
			error.message);

	dbus_connection_setup_with_g_main(dbus_connection, NULL);
}

static void finalize_dbus(void)
{
	dbus_connection_unref(dbus_connection);
}

static gboolean connect1(gpointer data)
{
	g_debug("connect1(%p)", data);

	ConIcConnection *connection;

	connection = con_ic_connection_new();
	g_assert(connection != NULL);

	g_signal_connect(G_OBJECT(connection), "connection-event",
			 G_CALLBACK(connection_cb),
			 GINT_TO_POINTER(USER_DATA_MAGIC));

#if 0
	gboolean b;
	b = con_ic_connection_connect(connection,
					CON_IC_CONNECT_FLAG_NONE);
	g_assert(b);
#else
	GValue value;
	memset(&value, 0, sizeof(value));
	g_value_init(&value, G_TYPE_BOOLEAN);
	g_value_set_boolean(&value, TRUE);
	g_object_set_property(G_OBJECT(connection),
			      "automatic-connection-events",
			      &value);
#endif

	return FALSE;
}

static gboolean connect2(gpointer data)
{
	g_debug("connect2(%p)", data);

	ConIcConnection *connection;
	
	connection = con_ic_connection_new();
	g_assert(connection != NULL);

	g_signal_connect(G_OBJECT(connection), "connection-event",
			 G_CALLBACK(connection_cb2),
			 GINT_TO_POINTER(USER_DATA_MAGIC2));

#if 1
	gboolean b;
	b = con_ic_connection_connect(connection,
					CON_IC_CONNECT_FLAG_NONE);
	g_assert(b);
#else
	GValue value;
	memset(&value, 0, sizeof(value));
	g_value_init(&value, G_TYPE_BOOLEAN);
	g_value_set_boolean(&value, TRUE);
	g_object_set_property(G_OBJECT(connection),
			      "automatic-connection-events",
			      &value);
#endif

	return FALSE;
}

int main(int argc, char *argv[]) 
{
	loop = init_glib();
	init_dbus();

	g_idle_add(connect1, NULL);
	g_idle_add(connect2, NULL);
	
	g_main_loop_run(loop);

	g_main_loop_unref(loop);
	finalize_dbus();
	
	return 0;
}
