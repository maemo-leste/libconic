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

#include "conicconnection.h"

#include <string.h>
#include <dbus/dbus.h>
#include <gconf/gconf-client.h>
#include <osso-ic-dbus.h>
#include <osso-ic-gconf.h>

#ifndef OSSO_IAP_ANY
#define OSSO_IAP_ANY	"[ANY]"
#endif

#include "conicevent.h"
#include "conicevent-private.h"
#include "conicconnectionevent.h"
#include "conicconnectionevent-private.h"
#include "conicstatisticsevent.h"
#include "conicstatisticsevent-private.h"
#include "coniciap-private.h"

#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN "ConIc"

#define ICD_DBUS_MATCH		"type='signal'," \
				"interface='" ICD_DBUS_INTERFACE "'," \
				"path='" ICD_DBUS_PATH "'"

typedef enum {
	CONNECTING,
	CONNECTED,
	DISCONNECTING,
	DISCONNECTED,
} ConIcConnectionInternalStatus;

static GSList *objects = NULL;
static DBusConnection *dbus_connection = NULL;

struct _ConIcConnectionClass
{
	GObjectClass parent_class;
	gint connection_signal_id, statistics_signal_id;
};

struct _ConIcConnection 
{
	GObject parent_instance;

	gboolean dispose_has_run;
	DBusConnection *dbus_connection;
	gchar *iap_id, *bearer;
	gboolean automatic_events;
	ConIcConnectionInternalStatus internal_status;
	ConIcProxyMode proxy_mode;
	gchar *http_host, *ftp_host, *https_host, *socks_host, *rtsp_host,
		*autoconfig_url;
	gint http_port, ftp_port, https_port, socks_port, rtsp_port;
	GSList *ignore_hosts;
	GSList *pending_calls;
    /* keep a copy of connetion status in _ConIcConnection */
    ConIcConnectionStatus   conn_status;
};

static GObjectClass *parent_class = NULL;

enum {
	CON_IC_CONNECTION_AUTOMATIC_EVENTS = 1,
} ConIcConnectionProperties;

static void con_ic_connection_class_init(gpointer g_class,
					   gpointer class_data);
static void con_ic_connection_init(GTypeInstance *instance, gpointer g_class);
static void con_ic_connection_dispose(GObject *obj);
static void con_ic_connection_finalize(GObject *obj);
static void con_ic_connection_set_property(GObject *object,
					     guint property_id,
					     const GValue *value,
					     GParamSpec *pspec);
static void con_ic_connection_get_property(GObject *object,
					     guint property_id,
					     GValue *value,
					     GParamSpec *pspec);
static DBusConnection *con_ic_connection_get_dbus_connection(ConIcConnection *connection);
static void con_ic_connection_send_event(ConIcConnection *connection,
					 const char *id,
					 const char *bearer,
					 ConIcConnectionStatus status);


GType con_ic_connection_get_type(void)
{
	static GType type = 0;
	if (type == 0){
		static const GTypeInfo info =  {
			.class_size = sizeof (ConIcConnectionClass),
			.base_init = NULL,
			.base_finalize = NULL,
			.class_init = con_ic_connection_class_init,
			.class_finalize = NULL,
			.class_data = NULL,
			.instance_size = sizeof (ConIcConnection),
			.n_preallocs = 0,
			.instance_init = con_ic_connection_init
		};
		type = g_type_register_static(G_TYPE_OBJECT,
					      "ConIcConnection",
					      &info, 0);
	}
	return type;
}

static void con_ic_connection_class_init(gpointer g_class,
					   gpointer class_data)
{

	GObjectClass *gobject_klass = G_OBJECT_CLASS(g_class);
	ConIcConnectionClass *klass = CON_IC_CONNECTION_CLASS(g_class);
	GType connection_param_types[] = {CON_IC_TYPE_CONNECTION_EVENT};
	GType statistics_param_types[] = {CON_IC_TYPE_STATISTICS_EVENT};
	GParamSpec *pspec;
	
	parent_class = g_type_class_peek_parent(gobject_klass);

	klass->connection_signal_id = 
		g_signal_newv("connection-event",
			      G_TYPE_FROM_CLASS(klass),
			      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
			      NULL /* class closure */,
			      NULL /* accumulator */,
			      NULL /* accu_data */,
			      g_cclosure_marshal_VOID__OBJECT,
			      G_TYPE_NONE /* return_type */,
			      1     /* n_params */,
			      connection_param_types  /* param_types */);

	klass->statistics_signal_id = 
		g_signal_newv("statistics",
			      G_TYPE_FROM_CLASS(klass),
			      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
			      NULL /* class closure */,
			      NULL /* accumulator */,
			      NULL /* accu_data */,
			      g_cclosure_marshal_VOID__OBJECT,
			      G_TYPE_NONE /* return_type */,
			      1     /* n_params */,
			      statistics_param_types  /* param_types */);

	gobject_klass->dispose  = con_ic_connection_dispose;
	gobject_klass->finalize = con_ic_connection_finalize;

	gobject_klass->set_property = con_ic_connection_set_property;
	gobject_klass->get_property = con_ic_connection_get_property;

	pspec = g_param_spec_boolean("automatic-connection-events",
				     "Automatic connection events",
				     "Set if all connection events need to be sent.",
				     FALSE,
				     G_PARAM_CONSTRUCT | G_PARAM_READWRITE);
	g_object_class_install_property (gobject_klass,
					 CON_IC_CONNECTION_AUTOMATIC_EVENTS,
					 pspec);
}

static void con_ic_connection_init(GTypeInstance *instance, gpointer g_class)
{
	ConIcConnection *connection = CON_IC_CONNECTION(instance);

	connection->iap_id = NULL;
	connection->bearer = NULL;
	connection->automatic_events = FALSE;
	connection->internal_status = DISCONNECTED;

	connection->proxy_mode = CON_IC_PROXY_MODE_NONE;
	connection->http_host = NULL;
	connection->ftp_host = NULL;
	connection->https_host = NULL;
	connection->socks_host = NULL;
	connection->rtsp_host = NULL;
	connection->autoconfig_url = NULL;
	connection->ignore_hosts = NULL;

	connection->http_port = 0;
	connection->ftp_port = 0;
	connection->https_port = 0;
	connection->socks_port = 0;
	connection->rtsp_port = 0;

	objects = g_slist_prepend(objects, connection);
}

static void con_ic_connection_dispose(GObject *obj)
{
	ConIcConnection *connection = CON_IC_CONNECTION(obj);
	
	if (connection->dispose_has_run) {
		return;
	}

	connection->dispose_has_run = TRUE;

	G_OBJECT_CLASS(parent_class)->dispose(obj);
}

static void con_ic_connection_clear_proxy_settings(ConIcConnection *connection)
{
	connection->proxy_mode = CON_IC_PROXY_MODE_NONE;
	g_free(connection->http_host); connection->http_host = NULL;
	g_free(connection->ftp_host); connection->ftp_host = NULL;
	g_free(connection->https_host); connection->https_host = NULL;
	g_free(connection->socks_host); connection->socks_host = NULL;
	g_free(connection->rtsp_host); connection->rtsp_host = NULL;
	g_free(connection->autoconfig_url); connection->autoconfig_url = NULL;

	connection->http_port = 0;
	connection->ftp_port = 0;
	connection->https_port = 0;
	connection->socks_port = 0;
	connection->rtsp_port = 0;
}

static void con_ic_connection_finalize(GObject *obj)
{
	ConIcConnection *connection = CON_IC_CONNECTION(obj);

	g_slist_foreach(connection->pending_calls, 
		        (GFunc)dbus_pending_call_cancel, NULL);
	g_slist_foreach(connection->pending_calls, 
		        (GFunc)dbus_pending_call_unref, NULL);
	g_slist_free(connection->pending_calls);
	
	con_ic_connection_clear_proxy_settings(connection);

	objects = g_slist_remove(objects, connection);
	if (objects == NULL && dbus_connection != NULL) {
		/* If all objects were deleted, unref DBUS stuff */
		dbus_connection_unregister_object_path(dbus_connection,
						       ICD_DBUS_PATH);
		dbus_bus_remove_match(dbus_connection, ICD_DBUS_MATCH, NULL);
		
		dbus_connection_unref(dbus_connection);
		dbus_connection = NULL;
	}
}

static void con_ic_connection_state_cb(DBusPendingCall *pending,
				       void *user_data)
{

	DBusMessage *reply = NULL;
	DBusError error;
	dbus_int32_t num_signals;
	ConIcConnection *connection = CON_IC_CONNECTION(user_data);

	dbus_error_init(&error);

	if (g_slist_find(connection->pending_calls, pending)) {
		connection->pending_calls = g_slist_remove(
			connection->pending_calls, pending);
		dbus_pending_call_unref(pending);
	}

	reply = dbus_pending_call_steal_reply(pending);

	if (reply == NULL) {
		goto exit;
	}
	
	if (dbus_set_error_from_message(&error, reply)) {
		goto exit;
	}

	if (!dbus_message_get_args(reply, &error,
				   DBUS_TYPE_UINT32, &num_signals,
				   DBUS_TYPE_INVALID)) {
		goto exit;
	}

	if (num_signals == 0) {
		con_ic_connection_send_event(connection, NULL, NULL,
					     CON_IC_STATUS_DISCONNECTED);
	}
	
exit:
	if (reply)
		dbus_message_unref(reply);
	if (dbus_error_is_set(&error))
		dbus_error_free(&error);
}

static gboolean con_ic_connection_enable_automatic_events(ConIcConnection *connection) 
{
	gboolean ret;
	DBusMessage *msg = NULL;
	DBusConnection *dbus_conn;
	DBusPendingCall *pending;
	
	/* start listening for signals emitted by icd */
	dbus_conn = con_ic_connection_get_dbus_connection(connection);
	if (dbus_conn == NULL) {
		ret = FALSE;
		goto exit;
	}

	msg = dbus_message_new_method_call(ICD_DBUS_SERVICE,
					   ICD_DBUS_PATH,
					   ICD_DBUS_INTERFACE,
					   ICD_GET_STATE_REQ);
	if (msg == NULL) {
		ret = FALSE;
		goto exit;
	}

	if (!dbus_connection_send_with_reply(dbus_conn, msg,
					     &pending, 1000) ||
	    pending == NULL) {
		ret = FALSE;
		goto exit;
	}

	if (!dbus_pending_call_set_notify(pending,
					  con_ic_connection_state_cb,
					  connection, NULL)) {
		dbus_pending_call_unref(pending);
		ret = FALSE;
		goto exit;
	}
	
	connection->pending_calls = g_slist_insert(
		connection->pending_calls, pending, 0);
	ret = TRUE;
	
exit:
	if (msg)
		dbus_message_unref(msg);

	return ret;

}

static void con_ic_connection_set_property(GObject *object,
					     guint property_id,
					     const GValue *value,
					     GParamSpec *pspec)
{
	ConIcConnection *connection = CON_IC_CONNECTION(object);

	switch (property_id) {
	case CON_IC_CONNECTION_AUTOMATIC_EVENTS:
		g_assert(G_VALUE_HOLDS_BOOLEAN(value));
		connection->automatic_events = g_value_get_boolean(value);
		if (connection->automatic_events)
			con_ic_connection_enable_automatic_events(connection);
		break;
	default:
		/* We don't have any other property... */
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void con_ic_connection_get_property(GObject *object,
					     guint property_id,
					     GValue *value,
					     GParamSpec *pspec)
{
	ConIcConnection *connection = CON_IC_CONNECTION(object);

	switch (property_id) {
	case CON_IC_CONNECTION_AUTOMATIC_EVENTS:
		g_value_set_boolean(value, connection->automatic_events);
		break;
	default:
		/* We don't have any other property... */
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void con_ic_connection_send_event(ConIcConnection *connection,
					 const char *id,
					 const char *bearer,
					 ConIcConnectionStatus status)
{
	/*g_debug("con_ic_connection_send_event(%p, %s, %s, %i)",
		(void *)connection, id, bearer, status);*/
	
	ConIcConnectionEvent *event = con_ic_connection_event_new();

	con_ic_event_set_iap_id(CON_IC_EVENT(event), id);
	con_ic_connection_event_set_status(event, status);
        connection->conn_status = status;    
	con_ic_event_set_bearer_type(CON_IC_EVENT(event), bearer);
	
	g_signal_emit(connection,
		      CON_IC_CONNECTION_GET_CLASS(connection)->connection_signal_id,
		      0, event);
	g_object_unref(G_OBJECT(event));
}

static DBusHandlerResult con_ic_connection_handle_icd_message(DBusConnection *c,
							      DBusMessage *msg,
							      gpointer user_data)
{
	char *iap_id, *bearer, *state;
	ConIcConnectionStatus status;
	GSList *e, *copy = NULL;
	
	if (!dbus_message_is_signal(msg, ICD_DBUS_INTERFACE, 
				    ICD_STATUS_CHANGED_SIG)) {
		goto exit;
	}

	if (!dbus_message_get_args(msg, NULL,
				   DBUS_TYPE_STRING, &iap_id,
				   DBUS_TYPE_STRING, &bearer,
				   DBUS_TYPE_STRING, &state,
				   DBUS_TYPE_INVALID)) {
		goto exit;
	}

	if (strcmp(state, "IDLE") == 0) {
		status = CON_IC_STATUS_DISCONNECTED;
	} else if (strcmp(state, "CONNECTED") == 0) {
		status = CON_IC_STATUS_CONNECTED;
	} else if (strcmp(state, "NETWORKUP") == 0) {
		status = CON_IC_STATUS_NETWORK_UP;
	} else if (strcmp(state, "DISCONNECTING") == 0) {
		status = CON_IC_STATUS_DISCONNECTING;
	} else {
		goto exit;
	}
	
	/* ref all objects in the list so they don't get destroyed, and
	 * take a copy so we remember which objects have been reffed;
	 * con_ic_connection_send_event might lead to unref on any of the
	 * objects in the list or addition of objects */
	copy = g_slist_copy(objects);
	g_slist_foreach(copy, (GFunc)g_object_ref, NULL);
	
	/* is this process connected already? */
	for (e = copy; e != NULL; e = g_slist_next(e)) {
		ConIcConnection *connection = e->data;

		/* Clear cached proxy settings, if connections are changed */
		if (status == CON_IC_STATUS_CONNECTED ||
		    status == CON_IC_STATUS_DISCONNECTED)
			con_ic_connection_clear_proxy_settings(connection);

		/* if automatic events are enabled, application has to track
		   the connections itself */
		if (connection->automatic_events) {
			if (status == CON_IC_STATUS_CONNECTED) {
				/* If IAP was connected, start tracking the 
				   new IAP */
				ConIcIap *iap;
				connection->internal_status = CONNECTED;
				g_free(connection->iap_id);
				connection->iap_id = g_strdup(iap_id);

				g_free(connection->bearer);
				iap = con_ic_iap_new(connection->iap_id);
				if (iap) {
				  connection->bearer =
				    g_strdup(con_ic_iap_get_bearer_type(iap));
				  g_object_unref(iap);
				} else
				  connection->bearer = NULL;
			}
			con_ic_connection_send_event(connection,
						     iap_id,
						     bearer,
						     status);
		}

		switch (status) {
		case CON_IC_STATUS_CONNECTED:
		  break;

		case CON_IC_STATUS_DISCONNECTED:		
		case CON_IC_STATUS_DISCONNECTING:
		  /* only the first IAP connection will be tracked */
		  if (connection->internal_status == CONNECTED &&
		      connection->iap_id != NULL &&
		      strcmp(iap_id, connection->iap_id) == 0) {

			if (!connection->automatic_events)
				con_ic_connection_send_event(
							connection,
							connection->iap_id,
							connection->bearer,
							status);

			if (status == CON_IC_STATUS_DISCONNECTING)
				connection->internal_status = DISCONNECTING;
			else {
				connection->internal_status = DISCONNECTED;
				g_free(connection->iap_id);
				connection->iap_id = NULL;
				g_free(connection->bearer);
				connection->bearer = NULL;
			}
		  }
		  break;

		default:
		  break;
		}
	}
	
exit:
	g_slist_foreach(copy, (GFunc)g_object_unref, NULL);
	g_slist_free(copy);

	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static DBusConnection *con_ic_connection_get_dbus_connection(ConIcConnection *connection)
{
	static struct DBusObjectPathVTable icd_vtable = {
		.message_function = &con_ic_connection_handle_icd_message
	};

	if (dbus_connection == NULL) {
		dbus_connection = dbus_bus_get(DBUS_BUS_SYSTEM, NULL);

		if (dbus_connection == NULL) {
			g_critical("Couldn't get DBUS connection.");
			goto exit;
		}

		dbus_bus_add_match(dbus_connection,
				   ICD_DBUS_MATCH,
				   NULL);
		/* FIXME: pass a list as user_data */
		if (!dbus_connection_register_object_path(dbus_connection,
							  ICD_DBUS_PATH,
							  &icd_vtable,
							  NULL)) {
			dbus_connection_unref(dbus_connection);
			dbus_connection = NULL;
		}
	}

exit:
	return connection->dbus_connection = dbus_connection;
}


static void con_ic_connection_connected_cb(DBusPendingCall *pending,
					   void *user_data)
{

	DBusMessage *reply = NULL;
	DBusError error;
	const char *iap_id;
	ConIcConnection *connection = CON_IC_CONNECTION(user_data);
	ConIcIap *iap;
	
	dbus_error_init(&error);

	if (g_slist_find(connection->pending_calls, pending)) {
		connection->pending_calls = g_slist_remove(
			connection->pending_calls, pending);
		dbus_pending_call_unref(pending);
	}

	reply = dbus_pending_call_steal_reply(pending);

	if (reply == NULL) {
		goto error;
	}
	
	if (dbus_set_error_from_message(&error, reply)) {
		goto error;
	}

	if (!dbus_message_get_args(reply, &error,
				   DBUS_TYPE_STRING, &iap_id,
				   DBUS_TYPE_INVALID)) {
		goto error;
	}

	if (connection->internal_status != CONNECTING)
		goto exit;

#if 0
	/* Broken applications are broken by not sending an extra
	   connected signal, revert the fix. Now applications must
	   track all connected IAPs in order to tell whether we are
	   online or offline.
	*/
	/* if automatic events are enabled, we already get the
	   connected signal via that, don't send two connected
	   events */
	if (connection->automatic_events)
		goto exit;
#endif
	
	connection->internal_status = CONNECTED;

	g_free(connection->iap_id);
	connection->iap_id = g_strdup(iap_id);

	g_free(connection->bearer);
	iap = con_ic_iap_new(connection->iap_id);
	if (iap) {
	  connection->bearer = g_strdup(con_ic_iap_get_bearer_type(iap));
	  g_object_unref(iap);
	} else
	  connection->bearer = NULL;
	
	con_ic_connection_send_event(connection, connection->iap_id,
				     connection->bearer,
				     CON_IC_STATUS_CONNECTED);

	goto exit;
	
error:
	connection->internal_status = DISCONNECTED;
	con_ic_connection_send_event(connection, NULL, NULL,
				     CON_IC_STATUS_DISCONNECTED);

exit:
	if (reply)
		dbus_message_unref(reply);
	if (dbus_error_is_set(&error))
		dbus_error_free(&error);
}

static void con_ic_connection_disconnected_cb(DBusPendingCall *pending,
					    void *user_data)
{
	
	DBusMessage *reply = NULL;
	DBusError error;
	const char *iap_id;
	ConIcConnection *connection = user_data;

	dbus_error_init(&error);

	if (g_slist_find(connection->pending_calls, pending)) {
		connection->pending_calls = g_slist_remove(
			connection->pending_calls, pending);
		dbus_pending_call_unref(pending);
	}

	reply = dbus_pending_call_steal_reply(pending);

	if (reply == NULL) {
		goto disconnected;
	}
	
	if (dbus_set_error_from_message(&error, reply)) {
		goto disconnected;
	}

	if (!dbus_message_get_args(reply, &error,
				   DBUS_TYPE_STRING, &iap_id,
				   DBUS_TYPE_INVALID)) {
		goto disconnected;
	}

	if (connection->internal_status != DISCONNECTING
	    || iap_id == NULL || strcmp(connection->iap_id, iap_id) != 0)
		goto exit;

disconnected:
	connection->internal_status = DISCONNECTED;
	con_ic_connection_send_event(connection, connection->iap_id,
				     connection->bearer,
				     CON_IC_STATUS_DISCONNECTED);

	g_free(connection->iap_id);
	connection->iap_id = NULL;
	g_free(connection->bearer);
	connection->bearer = NULL;

exit:

	if (reply)
		dbus_message_unref(reply);
	if (dbus_error_is_set(&error))
		dbus_error_free(&error);
}

static void con_ic_connection_statistics_cb(DBusPendingCall *pending,
					    void *user_data)
{

	const char *iap_id;
	dbus_uint32_t time_active = 0, signal_strength = 0,  rx_packets = 0,
	  tx_packets = 0, rx_bytes = 0, tx_bytes = 0;
	DBusMessage *reply = NULL;
	DBusError error;
	ConIcConnection *connection = CON_IC_CONNECTION(user_data);
	ConIcStatisticsEvent *event;

	dbus_error_init(&error);

	if (g_slist_find(connection->pending_calls, pending)) {
		connection->pending_calls = g_slist_remove(
			connection->pending_calls, pending);
		dbus_pending_call_unref(pending);
	}

	reply = dbus_pending_call_steal_reply(pending);

	if (reply == NULL) {
		goto error;
	}
	
	/* if an error is sent or reading arguments fail we'll fall back to
	   default values */
	if (!dbus_set_error_from_message(&error, reply))
	  dbus_message_get_args(reply, &error,
				DBUS_TYPE_STRING, &iap_id,
				DBUS_TYPE_UINT32, &time_active,
				DBUS_TYPE_UINT32, &signal_strength,
				DBUS_TYPE_UINT32, &rx_packets,
				DBUS_TYPE_UINT32, &tx_packets,
				DBUS_TYPE_UINT32, &rx_bytes,
				DBUS_TYPE_UINT32, &tx_bytes,
				DBUS_TYPE_INVALID);

	event = con_ic_statistics_event_new();

	con_ic_event_set_iap_id(CON_IC_EVENT(event), connection->iap_id);
	con_ic_event_set_bearer_type(CON_IC_EVENT(event), connection->bearer);
	con_ic_statistics_event_set_time_active(event, time_active);
	con_ic_statistics_event_set_signal_strength(event, signal_strength);
	con_ic_statistics_event_set_rx_packets(event, rx_packets);
	con_ic_statistics_event_set_tx_packets(event, tx_packets);
	con_ic_statistics_event_set_rx_bytes(event, rx_bytes);
	con_ic_statistics_event_set_tx_bytes(event, tx_bytes);

	g_signal_emit(connection,
		      CON_IC_CONNECTION_GET_CLASS(connection)->statistics_signal_id,
		      0, event);

error:
	if (reply)
		dbus_message_unref(reply);
	if (dbus_error_is_set(&error))
		dbus_error_free(&error);
}


ConIcConnection *con_ic_connection_new()
{
	return g_object_new(con_ic_connection_get_type(), NULL);
}


gboolean con_ic_connection_connect(ConIcConnection *connection,
				     ConIcConnectFlags flags)
{
	return con_ic_connection_connect_by_id(connection,
						 OSSO_IAP_ANY,
						 flags);
}

/* Request for a specific connection using the IAP id. */
gboolean con_ic_connection_connect_by_id(ConIcConnection *connection,
					    const gchar *id, 
					   ConIcConnectFlags flags)
{

	DBusMessage *msg = NULL;
	DBusConnection *dbus_conn;
	DBusPendingCall *pending = NULL;
	gboolean ret;
	GSList *e;
	
	/* is this process connected already? */
	/* No need to copy objects here as no connection nor objects are
	   accessed after send_event() */
	for (e = objects; e != NULL; e = g_slist_next(e)) {
		ConIcConnection *c = e->data;
		if (c == connection) {
			/* skip this object */
			continue;
		}
		
		if (c->internal_status == CONNECTED &&
		    (strcmp(id, OSSO_IAP_ANY) == 0 || strcmp(id, c->iap_id) == 0)) {
			/* already connected to the correct IAP */
			con_ic_connection_send_event(connection,
						     c->iap_id,
						     c->bearer,
						     CON_IC_STATUS_CONNECTED);
			ret = TRUE;
			goto exit;
		}
		/* FIXME: handle CONNECTING state also, put connection to
		 * CONNECTING state and send the CONNECTED event in
		 * connected_cb */
		/* FIXME: what about DISCONNECTING state? that case might
		 * be racy */
	}
	

	/* start listening for signals emitted by icd */
	dbus_conn = con_ic_connection_get_dbus_connection(connection);
	if (dbus_conn == NULL) {
		ret = FALSE;
		goto exit;
	}

	msg = dbus_message_new_method_call(ICD_DBUS_SERVICE,
					   ICD_DBUS_PATH,
					   ICD_DBUS_INTERFACE,
					   ICD_CONNECT_REQ);
	if (msg == NULL) {
		ret = FALSE;
		goto exit;
	}

	if (!dbus_message_append_args(msg,
				      DBUS_TYPE_STRING, &id,
				      DBUS_TYPE_UINT32, &flags,
				      DBUS_TYPE_INVALID)) {
		ret = FALSE;
		goto exit;
	}

	if (!dbus_connection_send_with_reply(dbus_conn, msg,
					     &pending, 3*60*1000) ||
	    pending == NULL) {
		ret = FALSE;
		goto exit;
	}

	if (!dbus_pending_call_set_notify(pending,
					  con_ic_connection_connected_cb,
					  connection, NULL)) {
		dbus_pending_call_unref(pending);
		ret = FALSE;
		goto exit;
	}

	connection->pending_calls = g_slist_insert(
		connection->pending_calls, pending, 0);
	connection->internal_status = CONNECTING;
	ret = TRUE;
	
exit:
	if (msg)
		dbus_message_unref(msg);

	return ret;
}

gboolean con_ic_connection_disconnect(ConIcConnection *connection)
{
	return con_ic_connection_disconnect_by_id(connection,
						    connection->iap_id);
}


gboolean con_ic_connection_disconnect_by_id(ConIcConnection *connection,
					      const gchar *id)
{
	DBusMessage *msg = NULL;
	DBusConnection *dbus_conn;
	DBusPendingCall *pending = NULL;
	gboolean ret;
	GSList *e;
	
	g_return_val_if_fail(connection != NULL && id != NULL, FALSE);

	/* does this process have other connections active still? */
	for (e = objects; e != NULL; e = g_slist_next(e)) {
		ConIcConnection *c = e->data;

		if (c == connection) {
			/* skip this object */
			continue;
		}

		if (c->internal_status == CONNECTED && strcmp(id, c->iap_id) == 0) {
			/* other object is connected to this IAP,
			   let's not request disconnection from icd yet */
			connection->internal_status = DISCONNECTED;
			con_ic_connection_send_event(connection,
						     connection->iap_id,
						     connection->bearer,
						     CON_IC_STATUS_DISCONNECTED);
			ret = TRUE;
			goto exit;
		}
	}
	
	/* start listening for signals emitted by icd */
	dbus_conn = con_ic_connection_get_dbus_connection(connection);
	if (dbus_conn == NULL) {
		ret = FALSE;
		goto exit;
	}

	msg = dbus_message_new_method_call(ICD_DBUS_SERVICE,
					   ICD_DBUS_PATH,
					   ICD_DBUS_INTERFACE,
					   ICD_DISCONNECT_REQ);
	if (msg == NULL) {
		ret = FALSE;
		goto exit;
	}

	if (!dbus_message_append_args(msg,
				      DBUS_TYPE_STRING, &id,
				      DBUS_TYPE_INVALID)) {
		ret = FALSE;
		goto exit;
	}

	if (!dbus_connection_send_with_reply(dbus_conn, msg,
					     &pending, 30*1000) ||
	    pending == NULL) {
		ret = FALSE;
		goto exit;
	}

	if (!dbus_pending_call_set_notify(pending,
					  con_ic_connection_disconnected_cb,
					  connection, NULL)) {
		dbus_pending_call_unref(pending);
		ret = FALSE;
		goto exit;
	}

	connection->pending_calls = g_slist_insert(
		connection->pending_calls, pending, 0);
	ret = TRUE;
	connection->internal_status = DISCONNECTING;
	
exit:
	if (msg)
		dbus_message_unref(msg);

	return ret;
}


gboolean con_ic_connection_statistics(ConIcConnection *connection,
					const gchar *id)
{

	DBusMessage *msg = NULL;
	DBusConnection *dbus_conn;
	DBusPendingCall *pending = NULL;
	gboolean ret;

	/* start listening for signals emitted by icd */
	dbus_conn = con_ic_connection_get_dbus_connection(connection);
	if (dbus_conn == NULL) {
		ret = FALSE;
		goto exit;
	}

	msg = dbus_message_new_method_call(ICD_DBUS_SERVICE,
					   ICD_DBUS_PATH,
					   ICD_DBUS_INTERFACE,
					   ICD_GET_STATISTICS_REQ);
	if (msg == NULL) {
		ret = FALSE;
		goto exit;
	}

	if (!dbus_connection_send_with_reply(dbus_conn, msg,
					     &pending, 1000) ||
	    pending == NULL) {
		ret = FALSE;
		goto exit;
	}

	if (!dbus_pending_call_set_notify(pending,
					  con_ic_connection_statistics_cb,
					  connection, NULL)) {
		dbus_pending_call_unref(pending);
		ret = FALSE;
		goto exit;
	}

	connection->pending_calls = g_slist_insert(
		connection->pending_calls, pending, 0);
	ret = TRUE;
	
exit:
	if (msg)
		dbus_message_unref(msg);

	return ret;
}


static gchar *con_ic_connection_get_gconf_path(ConIcConnection *connection,
					       const gchar *key)
{
	gchar *escaped = NULL;
	gchar *path = NULL;

	g_return_val_if_fail(connection != NULL, NULL);
	if (connection->iap_id == NULL) return NULL;

	escaped = gconf_escape_key (connection->iap_id, -1);
	path = g_strconcat (ICD_GCONF_PATH, "/", escaped, "/", key, NULL );
	g_free (escaped);
	return path;
}


ConIcProxyMode con_ic_connection_get_proxy_mode(ConIcConnection *connection)
{
	char *s = NULL;
	GConfClient *gconf = NULL;

    if (CON_IC_STATUS_CONNECTED != connection->conn_status) {
        return  CON_IC_PROXY_MODE_NONE;
    }

	char *path = con_ic_connection_get_gconf_path(connection, "proxytype");

	if (path == NULL) return CON_IC_PROXY_MODE_NONE;

	/* FIXME: use dbus proxy signals as soon as icd sends them */

	gconf = gconf_client_get_default();
	s = gconf_client_get_string(gconf, path, NULL);
	g_free (path);

	if (!s)
		goto exit;
	
	if (strcmp(s, "NONE") == 0) {
		connection->proxy_mode = CON_IC_PROXY_MODE_NONE;
	} else if (strcmp(s, "MANUAL") == 0) {
		connection->proxy_mode = CON_IC_PROXY_MODE_MANUAL;
	} else if (strcmp(s, "AUTOCONF") == 0) {
		connection->proxy_mode = CON_IC_PROXY_MODE_AUTO;
	} else {
		connection->proxy_mode = CON_IC_PROXY_MODE_NONE;
	}

exit:
	g_free(s);
	g_object_unref(gconf);
	return connection->proxy_mode;
}


static const gchar *con_ic_connection_get_proxy_http_host(ConIcConnection *connection)
{
	GConfClient *gconf = NULL;
	char *path = con_ic_connection_get_gconf_path(connection, 
						      "proxy_http");

	if (path == NULL) return NULL;

	/* FIXME: use dbus proxy signals as soon as icd sends them */

	gconf = gconf_client_get_default();
	g_free(connection->http_host);
	connection->http_host = gconf_client_get_string(gconf,
							path,
							NULL);
	g_free (path);
	g_object_unref(gconf);
	return connection->http_host;
}

static gint con_ic_connection_get_proxy_http_port(ConIcConnection *connection)
{
	GConfClient *gconf = NULL;
	char *path = con_ic_connection_get_gconf_path(connection, 
						      "proxy_http_port");

	if (path == NULL) return 0;

	/* FIXME: use dbus proxy signals as soon as icd sends them */

	gconf = gconf_client_get_default();
	connection->http_port = gconf_client_get_int(gconf,
						     path,
						     NULL);
	g_free (path);
	g_object_unref(gconf);
	return connection->http_port;
}

static const gchar *con_ic_connection_get_proxy_ftp_host(ConIcConnection *connection)
{
	GConfClient *gconf = NULL;
	char *path = con_ic_connection_get_gconf_path(connection,
						      "proxy_ftp");

	if (path == NULL) return NULL;

	/* FIXME: use dbus proxy signals as soon as icd sends them */

	gconf = gconf_client_get_default();
	g_free(connection->ftp_host);
	connection->ftp_host = gconf_client_get_string(gconf,
						       path,
						       NULL);
	g_free (path);
	g_object_unref(gconf);
	return connection->ftp_host;
}

static gint con_ic_connection_get_proxy_ftp_port(ConIcConnection *connection)
{
	GConfClient *gconf = NULL;
	char *path = con_ic_connection_get_gconf_path(connection,
						      "proxy_ftp_port");

	if (path == NULL) return 0;

	/* FIXME: use dbus proxy signals as soon as icd sends them */

	gconf = gconf_client_get_default();
	connection->ftp_port = gconf_client_get_int(gconf,
						    path,
						    NULL);
	g_free (path);
	g_object_unref(gconf);
	return connection->ftp_port;
}

static const gchar *con_ic_connection_get_proxy_https_host(ConIcConnection *connection)
{
	GConfClient *gconf = NULL;
	char *path = con_ic_connection_get_gconf_path(connection,
						      "proxy_https");

	if (path == NULL) return NULL;

	/* FIXME: use dbus proxy signals as soon as icd sends them */

	gconf = gconf_client_get_default();
	g_free(connection->https_host);
	connection->https_host = gconf_client_get_string(gconf,
							 path,
							NULL);
	g_free (path);
	g_object_unref(gconf);
	return connection->https_host;
}

static gint con_ic_connection_get_proxy_https_port(ConIcConnection *connection)
{
	GConfClient *gconf = NULL;
	char *path = con_ic_connection_get_gconf_path (connection,
						       "proxy_https_port");

	if (path == NULL) return 0;

	/* FIXME: use dbus proxy signals as soon as icd sends them */

	gconf = gconf_client_get_default();
	connection->https_port = gconf_client_get_int(gconf,
						      path,
						      NULL);
	g_free (path);
	g_object_unref(gconf);
	return connection->https_port;
}

static const gchar *con_ic_connection_get_proxy_socks_host(ConIcConnection *connection)
{
	GConfClient *gconf = NULL;
	char *path = con_ic_connection_get_gconf_path(connection,
						      "proxy_socks");

	if (path == NULL) return NULL;

	/* FIXME: use dbus proxy signals as soon as icd sends them */

	gconf = gconf_client_get_default();
	g_free(connection->socks_host);
	connection->socks_host = gconf_client_get_string(gconf,
							 path,
							 NULL);
	g_free (path);
	g_object_unref(gconf);
	return connection->socks_host;
}

static gint con_ic_connection_get_proxy_socks_port(ConIcConnection *connection)
{
	GConfClient *gconf = NULL;
	char *path = con_ic_connection_get_gconf_path (connection,
						       "proxy_socks_port");

	if (path == NULL) return 0;

	/* FIXME: use dbus proxy signals as soon as icd sends them */

	gconf = gconf_client_get_default();
	connection->socks_port = gconf_client_get_int(gconf,
						      path,
						      NULL);

	g_free (path);
	g_object_unref(gconf);
	return connection->socks_port;
}

static const gchar *con_ic_connection_get_proxy_rtsp_host(ConIcConnection *connection)
{
	GConfClient *gconf = NULL;
	char *path = con_ic_connection_get_gconf_path(connection,
						      "proxy_rtsp");

	if (path == NULL) return NULL;

	/* FIXME: use dbus proxy signals as soon as icd sends them */

	gconf = gconf_client_get_default();
	g_free(connection->rtsp_host);
	connection->rtsp_host = gconf_client_get_string(gconf,
							path,
							NULL);
	g_free (path);
	g_object_unref(gconf);
	return connection->rtsp_host;
}

static gint con_ic_connection_get_proxy_rtsp_port(ConIcConnection *connection)
{
	GConfClient *gconf = NULL;
	char *path = con_ic_connection_get_gconf_path (connection,
						       "proxy_rtsp_port");

	if (path == NULL) return 0;

	/* FIXME: use dbus proxy signals as soon as icd sends them */

	gconf = gconf_client_get_default();
	connection->rtsp_port = gconf_client_get_int(gconf,
						     path,
						     NULL);
	g_free (path);
	g_object_unref(gconf);
	return connection->rtsp_port;
}


const gchar *con_ic_connection_get_proxy_autoconfig_url(ConIcConnection *connection)
{
	GConfClient *gconf = NULL;

    if (CON_IC_STATUS_CONNECTED != connection->conn_status) {
        return NULL;
    }

	char *path = con_ic_connection_get_gconf_path (connection,
						       "autoconf_url");

	if (path == NULL) return NULL;

	/* FIXME: use dbus proxy signals as soon as icd sends them */

	gconf = gconf_client_get_default();
	g_free(connection->http_host);
	connection->http_host = gconf_client_get_string(gconf,
							path,
							NULL);
	g_free (path);
	g_object_unref(gconf);
	return connection->http_host;
}

const gchar *con_ic_connection_get_proxy_host(ConIcConnection* connection,
					      ConIcProxyProtocol protocol) 
{
	const gchar *result = NULL;

    if (CON_IC_STATUS_CONNECTED != connection->conn_status) {
        return NULL;
    }

	switch(protocol) {
	case CON_IC_PROXY_PROTOCOL_HTTP:
		result = con_ic_connection_get_proxy_http_host(connection);
		break;
	case CON_IC_PROXY_PROTOCOL_HTTPS:
		result = con_ic_connection_get_proxy_https_host(connection);
		break;
	case CON_IC_PROXY_PROTOCOL_FTP:
		result = con_ic_connection_get_proxy_ftp_host(connection);
		break;
	case CON_IC_PROXY_PROTOCOL_SOCKS:
		result = con_ic_connection_get_proxy_socks_host(connection);
		break;
	case CON_IC_PROXY_PROTOCOL_RTSP:
		result = con_ic_connection_get_proxy_rtsp_host(connection);
		break;
	default:
		break;
	}
	
	return result;
}



gint con_ic_connection_get_proxy_port(ConIcConnection* connection,
				      ConIcProxyProtocol protocol)
{
	gint result = 0;

    if (CON_IC_STATUS_CONNECTED != connection->conn_status) {
        return 0;
    }
	
	switch(protocol) {
	case CON_IC_PROXY_PROTOCOL_HTTP:
		result = con_ic_connection_get_proxy_http_port(connection);
		break;
	case CON_IC_PROXY_PROTOCOL_HTTPS:
		result = con_ic_connection_get_proxy_https_port(connection);
		break;
	case CON_IC_PROXY_PROTOCOL_FTP:
		result = con_ic_connection_get_proxy_ftp_port(connection);
		break;
	case CON_IC_PROXY_PROTOCOL_SOCKS:
		result = con_ic_connection_get_proxy_socks_port(connection);
		break;
	case CON_IC_PROXY_PROTOCOL_RTSP:
		result = con_ic_connection_get_proxy_rtsp_port(connection);
		break;
	default:
		break;
	}
	
	return result;
}


GSList *con_ic_connection_get_proxy_ignore_hosts(ConIcConnection *connection)
{
	GConfClient *gconf = NULL;

    if (CON_IC_STATUS_CONNECTED != connection->conn_status) {
            return (GSList *) NULL;
    }

	char *path = con_ic_connection_get_gconf_path(connection, 
						      "omit_proxy");
	GSList *list;

	if (path == NULL) return NULL;

	gconf = gconf_client_get_default();
	list = gconf_client_get_list(gconf, path,
				     GCONF_VALUE_STRING, NULL);
	g_free (path);
	g_object_unref(gconf);
	return list;
}


/** Check wheter an IAP is temporary
 * @param gconf_client GConfClient
 * @param iap_key the full GConf IAP path name
 * @param id the IAP id
 * @return TRUE if temporary, FALSE otherwise
 */
static gboolean con_ic_connection_is_temporary (GConfClient *gconf_client,
						const gchar *iap_key,
						const gchar *id)
{
  gchar *key;
  gboolean result = FALSE;
  
  if (id == NULL)
    return FALSE;

  key = g_strconcat (iap_key, "/temporary", NULL);
  if (!strncmp (id, "[Easy", 5) ||
      gconf_client_get_bool (gconf_client, key, NULL))
    result = TRUE;

  g_free (key);
  return result;
}

/* Check if an IAP is provisioned properly.
 * This is a kludge for bug NB#102394 where half provisioned GPRS
 * IAP is shown to the user even thou it should not be shown.
 * There should be a separate flag in gconf for these kind of IAPs
 * that are not yet ready to be connected. This function will
 * probably change in harmattan.
 * Return TRUE if IAP is ok, FALSE otherwise (so that it will not be
 * returned by get_all_iaps())
 */
static gboolean con_ic_connection_is_provisioned (GConfClient *gconf_client,
						  const gchar *iap_key)
{
  gchar *key, *iap_type = NULL;
  gboolean result = TRUE;
  GError *error = NULL;

  /* In harmattan we should check a special flag in gconf instead
   * of this kludge how it is now done in fremantle.
   */
  key = g_strconcat (iap_key, "/type", NULL);
  iap_type = gconf_client_get_string(gconf_client, key, &error);
  if (!iap_type)
    result = FALSE;
  else if (!strncmp(iap_type, "GPRS", 4)) {
    gchar *gprs_ap_name;
    g_free (key);
    key = g_strconcat (iap_key, "/gprs_accesspointname", NULL);
    gprs_ap_name = gconf_client_get_string(gconf_client, key, &error);
    if (!gprs_ap_name || !strlen(gprs_ap_name)) {
      result = FALSE;
    }
    g_free(gprs_ap_name);
  }

  if (error) {
    g_debug("GConf error: %s", error->message);
    g_clear_error(&error);
  }
  g_free (iap_type);
  g_free (key);
  return result;
}

GSList *con_ic_connection_get_all_iaps(ConIcConnection *connection)
{
	GConfClient *gconf = gconf_client_get_default();
	GSList *list, *entry;
	gchar *id;
	ConIcIap *iap;
	GSList *newl = NULL;

	list = gconf_client_all_dirs(gconf, ICD_GCONF_PATH, NULL);

	/* GConf returns absolute directory names, so we just
	 * return the relative directory names */
	for (entry = list; entry; entry = entry->next) {
	  if (entry->data != NULL) {
	    id = gconf_unescape_key(strrchr(entry->data, '/')+1, -1);
	    if (!con_ic_connection_is_temporary (gconf, entry->data, id) &&
		con_ic_connection_is_provisioned(gconf, entry->data) &&
		(iap = con_ic_iap_new(id)) != NULL)
	      newl = g_slist_prepend (newl, iap);
	    g_free (id);
	    g_free(entry->data);
	  }
	}
	g_slist_free (list);
	newl = g_slist_reverse (newl);

	g_object_unref(gconf);

	return newl;
}


ConIcIap *con_ic_connection_get_iap(ConIcConnection *connection,
				    const gchar *id)
{
	ConIcIap *iap;

	iap = con_ic_iap_new(id);
	
	return iap;
}
