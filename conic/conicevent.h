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

#ifndef CONICEVENT_H
#define CONICEVENT_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

/** 
 * @file conicevent.h
 *
 * ConIcEvent class.
 *
 * ConIcEvent is an abstract class. ConIcConnectionEvent and
 * ConIcStatistcsEvent derive from this class.
 */

#define CON_IC_TYPE_EVENT (con_ic_event_get_type ())
#define CON_IC_EVENT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj),\
				CON_IC_TYPE_EVENT, ConIcEvent))
#define CON_IC_EVENT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), CON_IC_TYPE_EVENT, ConIcEventClass))
#define CON_IC_IS_EVENT(obj)	 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CON_IC_TYPE_EVENT))
#define CON_IC_IS_EVENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CON_IC_TYPE_EVENT))
#define CON_IC_EVENT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), CON_IC_TYPE_EVENT, ConIcEventClass))

/** 
 * ConIcEvent object. The contents of the object is private, to access the
 * object use the functions provided by conicevent.h.
 */
typedef struct _ConIcEvent ConIcEvent;
typedef struct _ConIcEventClass  ConIcEventClass;

GType con_ic_event_get_type(void);

/**
 * Get the id of the IAP this event is associated with.
 *
 * @param event OssIcEvent object
 * @returns Id of the IAP.
 */
const gchar *con_ic_event_get_iap_id(ConIcEvent *event);

/**
 * Get bearer type of the IAP this event is associated with. Different
 * bearers are listed in coniciap.h.
 *
 * @param event OssIcEvent object
 * @returns Bearer of the IAP.
 */
const gchar *con_ic_event_get_bearer_type(ConIcEvent *event);

G_END_DECLS

#endif /* CONICEVENT_H */
