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

#ifndef CONICCONNECTION_H
#define CONICCONNECTION_H

#include <glib.h>
#include <glib-object.h>

#include <coniciap.h>

G_BEGIN_DECLS

/** 
 * @file conicconnection.h
 *
 * ConIcConnection class.
 *
 * ConIcConnection class can be used to request for Internet connections.
 * Also it can be used retrieve current statistics, proxies and settings
 * for Internet Access Points (IAPs).
 *
 * @section signals Gobject signals
 *
 * ConIcConnection has these gobject signals:
 * - connection-event
 * - statistics
 *
 * The different signals are listed below as function prototypes for
 * informational purposes. They are implemented as gobject signals.
 *
 * "connection-event" signal
 *
 * @code
 * void user_function(ConIcConnection *connection, 
 *                     ConIcConnectionEvent *event,
 *                     gpointer user_data);
 * @endcode
 *
 * When there's a new connection event for the application (eg. a
 * connection is opened as requested), it sent as connection-event signal.
 * ConIcConnectionEvent contains the status, for example
 * CON_IC_STATUS_CONNECTED or CON_IC_STATUS_DISCONNECTED.
 *
 * "statistics" signal
 *
 * @code
 * void user_function(ConIcConnection *connection, 
 *                    ConIcStatisticsEvent *event,
 *                    gpointer user_data);
 * @endcode
 *
 * All statistics are sent using this signal as ConIcStatistics event. 
 *
 * @section properties Gobject properties
 *
 * ConIcConnection has one gobject property:
 *
 * @code
 * "automatic-connection-events"        gboolean              : Read / Write 
 * @endcode
 *
 * If set to true, application will receive connection-events automatically
 * as connections are established and tore down. Normally events are only
 * sent when applications request for a connection, with this all events
 * are received constantly. This makes it possible, for example, to create
 * an application which executes something from the network every time a
 * connection is established.
 *
 * @note Automatic events are not stopped by Internet Connectivity system
 * when con_ic_connection_connect() is called. The documentation was wrong
 * and has now been updated.
 */

#define CON_IC_TYPE_CONNECTION (con_ic_connection_get_type ())
#define CON_IC_CONNECTION(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
					CON_IC_TYPE_CONNECTION, \
					ConIcConnection))
#define CON_IC_CONNECTION_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), CON_IC_TYPE_CONNECTION, ConIcConnectionClass))
#define CON_IC_IS_CONNECTION(obj)	 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CON_IC_TYPE_CONNECTION))
#define CON_IC_IS_CONNECTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CON_IC_TYPE_CONNECTION))
#define CON_IC_CONNECTION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), CON_IC_TYPE_CONNECTION, ConIcConnectionClass))


/** 
 * ConIcConnection object. The contents of the object is private, use only
 * the functions provided by conicconnection.h.
 */
typedef struct _ConIcConnection ConIcConnection;

typedef struct _ConIcConnectionClass  ConIcConnectionClass;

GType con_ic_connection_get_type(void);

/**
 * Creates new ConIcConnection. When not needed anymore, release with
 * g_object_unref().
 *
 * Creating the object does not yet open the connection. Use, for example,
 * con_ic_connection_connect() to do that.
 *
 * @return New ConIcConnection object. NULL, if the object creation
 * failed.
 */
ConIcConnection *con_ic_connection_new(void);

/**
 * Connection request flags. With these flags special connection requests
 * can be made. The flags can be ORred.
 *
 * Normally just use CON_IC_CONNECT_FLAG_NONE.
 */
typedef enum {
	/** No flags set. */
	CON_IC_CONNECT_FLAG_NONE = 0,
	/** Connection establishment wasn't started by a user action. 
	 * Instead it was triggered by a timer, or something similar, event
	 * from the application. Using this flags means that if a
	 * connection isn't already established, the connection request
	 * will fail. */
	CON_IC_CONNECT_FLAG_AUTOMATICALLY_TRIGGERED = 1 << 0,
	/** Process requesting the connection won't be monitored for
	 * killing. Normally if the process died, it will be automatically
	 * detached from the connection. If this flag is set and process
	 * dies, the connection won't closed automatically.*/
	CON_IC_CONNECT_FLAG_UNMANAGED = 1 << 1
} ConIcConnectFlags;

/** 
 * Request for a connection. The Internet Connectivity subsystem will
 * choose the best connection based on user settings and input provided by
 * the user.
 *
 * Answer is sent using the connection-event signal from a ConIcConnection
 * object. If the connection establishment succeeded, or there was already
 * a connection established, a CON_IC_STATUS_CONNECTED inside
 * ConIcConnectionEvent is sent. If connection establishment failed, a
 * CON_IC_STATUS_DISCONNECTED inside ConIcConnectionEvent is sent and error
 * is set accordingly.
 *
 * Normally this one should be used.
 *
 * @param connection ConIcConnection object.
 * @param flags Flags for the request.
 * @retval TRUE If no DBUS errors.
 * @retval FALSE If a DBUS message couldn't be sent.
 */
gboolean con_ic_connection_connect(ConIcConnection *connection,
				    ConIcConnectFlags flags);

/**
 * Request for a connection using the IAP id. The Internet Connectivity
 * subsystem will choose the best connection based on user settings and
 * input provided by the user.
 *
 * Answer is sent using the connection-event signal from a ConIcConnection
 * object. If the connection establishment succeeded, or there was already
 * a connection established, a CON_IC_STATUS_CONNECTED inside
 * ConIcConnectionEvent is sent. If connection establishment failed, a
 * CON_IC_STATUS_DISCONNECTED inside ConIcConnectionEvent is sent and error
 * is set accordingly.
 *
 * Normally con_ic_connection_connect() should be used. Use this one if you
 * want to use a specific connection.
 *
 * @param connection ConIcConnection object.
 * @param flags Flags for the request.
 * @param id Id of the requested IAP.
 * @retval TRUE If no DBUS errors.
 * @retval FALSE If a DBUS message couldn't be sent.
 */
gboolean con_ic_connection_connect_by_id(ConIcConnection *connection,
					 const gchar *id, 
					 ConIcConnectFlags flags);

/** 
 * Disconnects all IAPs associated with the connection.
 *
 * Normally use this one.
 *
 * @param connection ConIcConnection object.
 * @retval TRUE If no DBUS errors.
 * @retval FALSE If a DBUS message couldn't be sent.
 */
gboolean con_ic_connection_disconnect(ConIcConnection *connection);

/**
 * Disconnects specific IAP associated with the application. 
 *
 * Normally use con_ic_connection_disconnect().
 *
 * @param connection ConIcConnection object.
 * @param id Id of the IAP to disconnected.
 * @retval TRUE If no DBUS errors.
 * @retval FALSE If a DBUS message couldn't be sent.
 */
gboolean con_ic_connection_disconnect_by_id(ConIcConnection *connection,
					    const gchar *id);


/**
 * Requests statistics for a IAP. The answer is sent as ConIcStatistics in
 * statistics signal.
 *
 * @param connection ConIcConnection object.
 * @param id Id of the IAP
 * @retval TRUE If no DBUS errors.
 * @retval FALSE If a DBUS message couldn't be sent. No statistics events will
 *         be emitted.
 */
gboolean con_ic_connection_statistics(ConIcConnection *connection,
					const gchar *id);

/**
 * Proxy modes.
 */
typedef enum {
	/** No proxies set. */
	CON_IC_PROXY_MODE_NONE,
	/** Manual proxies set */
	CON_IC_PROXY_MODE_MANUAL,
	/** Automatic proxy URL set. */
	CON_IC_PROXY_MODE_AUTO
} ConIcProxyMode;

/**
 * Get current proxy mode.
 *
 * If proxy mode is CON_IC_PROXY_MODE_NONE, do not use proxies at all.
 *
 * If proxy mode is CON_IC_PROXY_MODE_MANUAL, use proxies and <em>only</em>
 * use these functions to get the proxy settings:
 *
 * - con_ic_connection_get_proxy_host()
 * - con_ic_connection_get_proxy_port()
 * - con_ic_connection_get_proxy_ignore_hosts()
 *
 * If proxy mode is CON_IC_PROXY_MODE_AUTO, then <em>only</em> use Proxy
 * Auto-config file specified in
 * <http://wp.netscape.com/eng/mozilla/2.0/relnotes/demo/proxy-live.html>.
 * To get the URL for the file, use this function: 
 *
 * - con_ic_connection_get_proxy_autoconfig_url()
 *
 */
ConIcProxyMode con_ic_connection_get_proxy_mode(ConIcConnection *connection);

/**
 * Get a list of hosts to be ignored. Connections to these hosts shouldn't
 * use proxies.
 *
 * It is guaranteed that this function always returns the current and
 * up-to-date settings.
 * @param connection ConIcConnection object
 * @returns A GSList of strings, which contain the hosts. Free all strings
 * individually, and the list with g_slist_free(). NULL, if no hosts
 * specified.
 */
GSList *con_ic_connection_get_proxy_ignore_hosts(ConIcConnection *connection);

/** 
 * Get the URL of Auto-Config Proxy.
 *
 * It is guaranteed that this function always returns the current and
 * up-to-date settings.
 *
 * @param connection ConIcConnection object
 * @returns URL as a string. NULL, if no URL specified.
 */
const gchar *con_ic_connection_get_proxy_autoconfig_url(ConIcConnection *connection);

/**
 * Proxy protocol.
 */
typedef enum {
	/** HTTP proxy protocol. */
	CON_IC_PROXY_PROTOCOL_HTTP,
	/** HTTPS proxy protocol. */
	CON_IC_PROXY_PROTOCOL_HTTPS,
	/** FTP proxy protocol. */
	CON_IC_PROXY_PROTOCOL_FTP,
	/** SOCKS proxy protocol. */
	CON_IC_PROXY_PROTOCOL_SOCKS,
	/** RTSP proxy protocol. */
	CON_IC_PROXY_PROTOCOL_RTSP
} ConIcProxyProtocol;

/**
 * Get proxy host.
 *
 * @param connection ConIcConnection object
 * @param protocol Protocol, for which the proxy host should be returned
 * @returns hostname of the proxy for the protocol
 */
const gchar *con_ic_connection_get_proxy_host(ConIcConnection* connection,
					      ConIcProxyProtocol protocol);

/**
 * Get proxy port.
 *
 * @param connection ConIcConnection object
 * @param protocol Protocol, for which the proxy port should be returned
 * @returns port number of the proxy host for the protocol
 */
gint con_ic_connection_get_proxy_port(ConIcConnection* connection,
				      ConIcProxyProtocol protocol);

/**
 * Get a list of all configured IAPs. 
 *
 * @param connection ConIcConnection object
 * @returns A singly linked list of ConIcIaps or NULL for error. Caller
 * should free all ConIcIaps with g_object_unref() and the list itself with
 * g_slist_free().
 */
GSList *con_ic_connection_get_all_iaps(ConIcConnection *connection);

/**
 * Retrieve an IAP by id.
 *
 * @returns ConIcIap gobject which caller must free with
 * g_object_unref().
 */
ConIcIap *con_ic_connection_get_iap(ConIcConnection *connection,
				    const gchar *id);

G_END_DECLS

#endif /* CONICCONNECTION_H */
