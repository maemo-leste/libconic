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

static void test_proxies()
{
	GSList *list, *entry;
	
	g_debug("proxy_mode=%i",
		con_ic_connection_get_proxy_mode(connection));
	g_debug("http_host=%s",
		con_ic_connection_get_proxy_host(connection,
						 CON_IC_PROXY_PROTOCOL_HTTP));
	g_debug("http_port=%i",
		con_ic_connection_get_proxy_port(connection,
						 CON_IC_PROXY_PROTOCOL_HTTP));
	g_debug("https_host=%s",
		con_ic_connection_get_proxy_host(connection,
						 CON_IC_PROXY_PROTOCOL_HTTPS));
	g_debug("https_port=%i",
		con_ic_connection_get_proxy_port(connection,
						 CON_IC_PROXY_PROTOCOL_HTTPS));
	g_debug("ftp_host=%s",
		con_ic_connection_get_proxy_host(connection,
						 CON_IC_PROXY_PROTOCOL_FTP));
	g_debug("ftp_port=%i",
		con_ic_connection_get_proxy_port(connection,
						 CON_IC_PROXY_PROTOCOL_FTP));
	g_debug("rtsp_host=%s",
		con_ic_connection_get_proxy_host(connection,
						 CON_IC_PROXY_PROTOCOL_RTSP));
	g_debug("rtsp_port=%i",
		con_ic_connection_get_proxy_port(connection,
						 CON_IC_PROXY_PROTOCOL_RTSP));
	g_debug("socks_host=%s",
		con_ic_connection_get_proxy_host(connection,
						 CON_IC_PROXY_PROTOCOL_SOCKS));
	g_debug("socks_port=%i",
		con_ic_connection_get_proxy_port(connection,
						 CON_IC_PROXY_PROTOCOL_SOCKS));
	g_debug("autoconfig_url=%s",
		con_ic_connection_get_proxy_autoconfig_url(connection));

	g_debug("ignore_hosts:");
	list = con_ic_connection_get_proxy_ignore_hosts(connection);
	for (entry = list; entry; entry = entry->next) {
		g_debug("%s", (gchar*)entry->data);
		g_free(entry->data);
	}
	g_slist_free(list);

	g_main_loop_quit(loop);
}

#if 0
static void test_proxies_old()
{
	GSList *list, *entry;
	
	g_debug("proxy_mode=%i",
		con_ic_connection_get_proxy_mode(connection));
	g_debug("http_host=%s",
		con_ic_connection_get_proxy_http_host(connection));
	g_debug("http_port=%i",
		con_ic_connection_get_proxy_http_port(connection));
	g_debug("https_host=%s",
		con_ic_connection_get_proxy_https_host(connection));
	g_debug("https_port=%i",
		con_ic_connection_get_proxy_https_port(connection));
	g_debug("ftp_host=%s",
		con_ic_connection_get_proxy_ftp_host(connection));
	g_debug("ftp_port=%i",
		con_ic_connection_get_proxy_ftp_port(connection));
	g_debug("rtsp_host=%s",
		con_ic_connection_get_proxy_rtsp_host(connection));
	g_debug("rtsp_port=%i",
		con_ic_connection_get_proxy_rtsp_port(connection));
	g_debug("socks_host=%s",
		con_ic_connection_get_proxy_socks_host(connection));
	g_debug("socks_port=%i",
		con_ic_connection_get_proxy_socks_port(connection));
	g_debug("autoconfig_url=%s",
		con_ic_connection_get_proxy_autoconfig_url(connection));

	g_debug("ignore_hosts:");
	list = con_ic_connection_get_proxy_ignore_hosts(connection);
	for (entry = list; entry; entry = entry->next) {
		g_debug("%s", (gchar*)entry->data);
		g_free(entry->data);
	}
	g_slist_free(list);

	g_main_loop_quit(loop);
}
#endif

static gboolean start(gpointer user_data)
{
	g_debug("start(%p)", user_data);

	connection = con_ic_connection_new();
	g_assert(connection != NULL);

	test_proxies();
	
	return FALSE;
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
