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

static void connection_cb(ConIcConnection *connection,
			  ConIcConnectionEvent *event,
			  gpointer user_data);

static GMainLoop *loop;
static ConIcConnection *connection;
static gchar *iap_id;
static DBusConnection *dbus_connection;

static gboolean request_statistics(gpointer data)
{
	g_debug("request_statistics(%p)", data);

	static int counter = 0;
	
	if (counter >= 10) {
		g_debug("Max counter reached (%i). Quitting.", counter);
		g_main_loop_quit(loop);
		goto exit;
	}
	counter++;

	con_ic_connection_statistics(connection, iap_id);

exit:
	return TRUE;
}


static void statistics_cb(ConIcConnection *connection,
			  ConIcStatisticsEvent *event,
			  gpointer user_data)
{
	g_debug("statistics_cb(%p, %p)", (void *)event, user_data);

	g_assert(GPOINTER_TO_INT(user_data) == USER_DATA_MAGIC);
	
	g_assert(CON_IC_IS_STATISTICS_EVENT(event));

	g_debug("time_active=%i",
		con_ic_statistics_event_get_time_active(event));
	g_debug("signal_strength=%i",
		con_ic_statistics_event_get_signal_strength(event));
	g_debug("rx_packets=%llu",
		con_ic_statistics_event_get_rx_packets(event));
	g_debug("tx_packets=%llu",
		con_ic_statistics_event_get_tx_packets(event));
	g_debug("rx_bytes=%llu",
		con_ic_statistics_event_get_rx_bytes(event));
	g_debug("tx_bytes=%llu",
		con_ic_statistics_event_get_tx_bytes(event));
}


static gboolean start(gpointer data)
{
	g_debug("start(%p)", data);

	connection = con_ic_connection_new();
	g_assert(connection != NULL);

	g_signal_connect(G_OBJECT(connection), "connection-event",
			 G_CALLBACK(connection_cb),
			 GINT_TO_POINTER(USER_DATA_MAGIC));

	g_signal_connect(G_OBJECT(connection), "statistics",
			 G_CALLBACK(statistics_cb),
			 GINT_TO_POINTER(USER_DATA_MAGIC));

	con_ic_connection_connect(connection, CON_IC_CONNECT_FLAG_NONE);
	
	return FALSE;
}

static void connection_cb(ConIcConnection *connection,
			  ConIcConnectionEvent *event,
			  gpointer user_data)
{
	g_debug("connection_cb(%p, %p)", (void *)event, user_data);

	ConIcConnectionStatus status;

	g_assert(GPOINTER_TO_INT(user_data) == USER_DATA_MAGIC);
	
	g_assert(CON_IC_IS_CONNECTION_EVENT(event));
	status = con_ic_connection_event_get_status(event);

	switch (status) {
	case CON_IC_STATUS_CONNECTED:
		g_debug("CONNECTED");
		iap_id =
			g_strdup(con_ic_event_get_iap_id(CON_IC_EVENT(event)));
		g_timeout_add(10000, request_statistics, NULL);
		break;
	case CON_IC_STATUS_DISCONNECTED:
		g_debug("DISCONNECTED");
		break;
	case CON_IC_STATUS_DISCONNECTING:
		g_debug("DISCONNECTING");
		break;
	default:
		break;
	}
}


static GMainLoop *init_glib(void)
{
	g_type_init();
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

int main(int argc, char *argv[]) 
{
	loop = init_glib();
	init_dbus();

	g_idle_add(start, NULL);
	
	g_main_loop_run(loop);

	g_main_loop_unref(loop);
	finalize_dbus();
	
	return 0;
}
