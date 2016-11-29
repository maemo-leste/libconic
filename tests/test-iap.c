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

static GMainLoop *loop;
static ConIcConnection *connection;
static DBusConnection *dbus_connection;

static void test_all_iaps()
{
	GSList *list, *entry;
	ConIcIap *iap;
	
	g_debug("con_ic_connection_get_all_iaps:");
	list = con_ic_connection_get_all_iaps(connection);
	for (entry = list; entry; entry = entry->next) {
		iap = CON_IC_IAP(entry->data);
		g_assert(CON_IC_IS_IAP(iap));
		g_debug("%s: (%s) %s",
			con_ic_iap_get_id(iap),
			con_ic_iap_get_name(iap),
			con_ic_iap_get_bearer_type(iap));
		g_object_unref(iap);
	}
	g_slist_free(list);

	g_main_loop_quit(loop);
}

static void test_get_iap()
{
	ConIcIap *iap = con_ic_connection_get_iap(connection, "osso.net");
	if (iap) {
	  g_assert(CON_IC_IS_IAP(iap));
	  g_debug("get IAP osso.net: %s (%s) %s",
		  con_ic_iap_get_id(iap),
		  con_ic_iap_get_name(iap),
		  con_ic_iap_get_bearer_type(iap));
	} else
	  g_debug ("get IAP osso.net: osso.net does not exist");
}

static gboolean start(gpointer user_data)
{
	g_debug("start(%p)", user_data);

	connection = con_ic_connection_new();
	g_assert(connection != NULL);

	test_all_iaps();
	test_get_iap();
	
	return FALSE;
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
