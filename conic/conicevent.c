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

#include "conicevent.h"
#include "conicevent-private.h"

	
static GObjectClass *parent_class = NULL;

static void con_ic_event_class_init(gpointer g_class,
					   gpointer class_data);
static void con_ic_event_init(GTypeInstance *instance, gpointer g_class);
static void con_ic_event_dispose(GObject *obj);
static void con_ic_event_finalize(GObject *obj);


GType con_ic_event_get_type(void)
{
	static GType type = 0;
	if (type == 0){
		static const GTypeInfo info =  {
			.class_size = sizeof (ConIcEventClass),
			.base_init = NULL,
			.base_finalize = NULL,
			.class_init = con_ic_event_class_init,
			.class_finalize = NULL,
			.class_data = NULL,
			.instance_size = sizeof (ConIcEvent),
			.n_preallocs = 0,
			.instance_init = con_ic_event_init
		};
		type = g_type_register_static (G_TYPE_OBJECT,
					       "ConIcEvent",
					       &info,
					       G_TYPE_FLAG_ABSTRACT);
	}
	return type;
}

static void con_ic_event_class_init(gpointer g_class,
				      gpointer class_data)
{
	GObjectClass *gobject_klass = G_OBJECT_CLASS(g_class);
/* 	ConIcEventClass *klass = CON_IC_EVENT_CLASS(g_class); */

	parent_class = g_type_class_peek_parent(gobject_klass);
	
	gobject_klass->dispose  = con_ic_event_dispose;
	gobject_klass->finalize = con_ic_event_finalize;
}

static void con_ic_event_init(GTypeInstance *instance, gpointer g_class)
{
	ConIcEvent *self = CON_IC_EVENT(instance);
	
	self->id = NULL;
	self->bearer_type = NULL;
}

static void con_ic_event_dispose(GObject *obj)
{
	ConIcEvent *self = CON_IC_EVENT(obj);
	
	if (self->dispose_has_run) {
		return;
	}

	self->dispose_has_run = TRUE;

	G_OBJECT_CLASS(parent_class)->dispose(obj);
}

static void con_ic_event_finalize(GObject *obj)
{
	ConIcEvent *self = CON_IC_EVENT(obj);

	g_free(self->id);
	g_free(self->bearer_type);
}

const gchar *con_ic_event_get_iap_id(ConIcEvent *event)
{
	return event->id;
}


const gchar *con_ic_event_get_bearer_type(ConIcEvent *event)
{
	return event->bearer_type;
}

void con_ic_event_set_iap_id(ConIcEvent *event, const gchar *id)
{
	g_free(event->id);
	event->id = g_strdup(id);
}


void con_ic_event_set_bearer_type(ConIcEvent *event, const gchar *type)
{
	g_free(event->bearer_type);
	event->bearer_type = g_strdup(type);
}

