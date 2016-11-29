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

#ifndef CONICCONNECTIONEVENT_H
#define CONICCONNECTIONEVENT_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

/** 
 * @file conicconnectionevent.h
 *
 * ConIcConnectionEvent class.
 *
 * ConIcConnectionEvent is sent whenever status of a connection changes.
 * It's derived from ConIcEvent class.
 */

#define CON_IC_TYPE_CONNECTION_EVENT (con_ic_connection_event_get_type ())
#define CON_IC_CONNECTION_EVENT(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), CON_IC_TYPE_CONNECTION_EVENT, \
		ConIcConnectionEvent))
#define CON_IC_CONNECTION_EVENT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST ((klass), \
		CON_IC_TYPE_CONNECTION_EVENT, ConIcConnectionEventClass))
#define CON_IC_IS_CONNECTION_EVENT(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), CON_IC_TYPE_CONNECTION_EVENT))
#define CON_IC_IS_CONNECTION_EVENT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), CON_IC_TYPE_CONNECTION_EVENT))
#define CON_IC_CONNECTION_EVENT_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), CON_IC_TYPE_CONNECTION_EVENT, \
		ConIcConnectionEventClass))

/** 
 * ConIcConnectionEvent object. The contents of the object is private, to
 * access the object use the functions provided by conicconnectionevent.h.
 */
typedef struct _ConIcConnectionEvent ConIcConnectionEvent;
typedef struct _ConIcConnectionEventClass  ConIcConnectionEventClass;

GType con_ic_connection_event_get_type(void);


/**
 * IAP connection statuses.
 *
 * @note More status types might be added in the future.
 */
typedef enum {
	/** The IAP was connected */
	CON_IC_STATUS_CONNECTED,
	/** The IAP was disconnected */
	CON_IC_STATUS_DISCONNECTED,
	/** The IAP is disconnecting */
	CON_IC_STATUS_DISCONNECTING,
	/** The IAP has a network address, but is not yet fully connected */
	CON_IC_STATUS_NETWORK_UP
} ConIcConnectionStatus;

/**
 * Error codes for connection events. Only set in DISCONNECTED events,
 * otherwise set to CON_IC_CONNECTION_ERROR_NONE.
 *
 * @note More error types might be added in the future.
 */
typedef enum {
	/** No errors. */
	CON_IC_CONNECTION_ERROR_NONE,
	/** Requested IAP was invalid (for example not found or incomplete
	    settings). */
	CON_IC_CONNECTION_ERROR_INVALID_IAP,
	/** Connections establishment failed for unknown reason. */
	CON_IC_CONNECTION_ERROR_CONNECTION_FAILED,
	/** Connections establishment failed because of user
	 * cancellation. */
	CON_IC_CONNECTION_ERROR_USER_CANCELED
} ConIcConnectionError;

/**
 * Get status from the event.
 *
 * @param event OssIcConnectionEvent object
 * @returns status
 */
ConIcConnectionStatus con_ic_connection_event_get_status(ConIcConnectionEvent *event);

/**
 * Get error from the event.
 *
 * @param event OssIcConnectionEvent object
 * @returns error
 */
ConIcConnectionError con_ic_connection_event_get_error(ConIcConnectionEvent *event);

G_END_DECLS

#endif /* CONICCONNECTIONEVENT_H */
