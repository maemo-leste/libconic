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

#include "conicconnectionevent.h"
#include "conicconnectionevent-private.h"

#include "conicevent.h"
#include "conicevent-private.h"

struct _ConIcConnectionEventClass
{
	ConIcEventClass parent_class;
};

struct _ConIcConnectionEvent 
{
	ConIcEvent parent_instance;

	ConIcConnectionStatus status;
	ConIcConnectionError error;
};

static void con_ic_connection_event_class_init(gpointer g_class,
						 gpointer class_data);
static void con_ic_connection_event_init(GTypeInstance *instance, gpointer g_class);


GType con_ic_connection_event_get_type(void)
{
	static GType type = 0;
	if (type == 0){
		static const GTypeInfo info =  {
			.class_size = sizeof (ConIcConnectionEventClass),
			.base_init = NULL,
			.base_finalize = NULL,
			.class_init = con_ic_connection_event_class_init,
			.class_finalize = NULL,
			.class_data = NULL,
			.instance_size = sizeof (ConIcConnectionEvent),
			.n_preallocs = 0,
			.instance_init = con_ic_connection_event_init
		};
		type = g_type_register_static (CON_IC_TYPE_EVENT,
					       "ConIcConnectionEvent",
					       &info, 0);
	}
	return type;
}

static void con_ic_connection_event_class_init(gpointer g_class,
						 gpointer class_data)
{
/* 	ConIcConnectionEventClass *klass = CON_IC_CONNECTION_EVENT_CLASS(g_class); */
}

static void con_ic_connection_event_init(GTypeInstance *instance,
					   gpointer g_class)
{
	ConIcConnectionEvent *self = CON_IC_CONNECTION_EVENT(instance);

	self->status = CON_IC_STATUS_DISCONNECTED;
	self->error = CON_IC_CONNECTION_ERROR_NONE;
}

ConIcConnectionEvent *con_ic_connection_event_new()
{
	return g_object_new(con_ic_connection_event_get_type(), NULL);
}

ConIcConnectionStatus con_ic_connection_event_get_status(ConIcConnectionEvent *event)
{
	return event->status;
}

ConIcConnectionError con_ic_connection_event_get_error(ConIcConnectionEvent *event)
{
	return event->error;
}

void con_ic_connection_event_set_status(ConIcConnectionEvent *event,
					  ConIcConnectionStatus status)
{
	event->status = status;
}

void con_ic_connection_event_set_error(ConIcConnectionEvent *event,
					 ConIcConnectionError error)
{
	event->error = error;
}


