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

#include "conicstatisticsevent.h"
#include "conicstatisticsevent-private.h"
#include "conicevent-private.h"

struct _ConIcStatisticsEventClass
{
	ConIcEventClass parent_class;
};

struct _ConIcStatisticsEvent 
{
	ConIcEvent parent_instance;

	gboolean dispose_has_run;
	guint time_active, signal_strength;
	guint64 rx_packets, tx_packets, rx_bytes, tx_bytes;
};

static GObjectClass *parent_class = NULL;

static void con_ic_statistics_event_class_init(gpointer g_class,
						 gpointer class_data);
static void con_ic_statistics_event_init(GTypeInstance *instance, gpointer g_class);
static void con_ic_statistics_event_dispose(GObject *obj);
static void con_ic_statistics_event_finalize(GObject *obj);


GType con_ic_statistics_event_get_type(void)
{
	static GType type = 0;
	if (type == 0){
		static const GTypeInfo info =  {
			.class_size = sizeof (ConIcStatisticsEventClass),
			.base_init = NULL,
			.base_finalize = NULL,
			.class_init = con_ic_statistics_event_class_init,
			.class_finalize = NULL,
			.class_data = NULL,
			.instance_size = sizeof (ConIcStatisticsEvent),
			.n_preallocs = 0,
			.instance_init = con_ic_statistics_event_init
		};
		type = g_type_register_static (CON_IC_TYPE_EVENT,
					       "ConIcStatisticsEvent",
					       &info, 0);
	}
	return type;
}

static void con_ic_statistics_event_class_init(gpointer g_class,
						 gpointer class_data)
{
	GObjectClass *gobject_klass = G_OBJECT_CLASS(g_class);

	parent_class = g_type_class_peek_parent(gobject_klass);
	gobject_klass->dispose  = con_ic_statistics_event_dispose;
	gobject_klass->finalize = con_ic_statistics_event_finalize;


}

static void con_ic_statistics_event_init(GTypeInstance *instance,
					   gpointer g_class)
{
	ConIcStatisticsEvent *statistics = CON_IC_STATISTICS_EVENT(instance);
	statistics->time_active = 0;
	statistics->signal_strength = 0;
	statistics->rx_packets = 0;
	statistics->tx_packets = 0;
	statistics->rx_bytes = 0;
	statistics->tx_bytes = 0;
}

static void con_ic_statistics_event_dispose(GObject *obj)
{
	ConIcStatisticsEvent *event = CON_IC_STATISTICS_EVENT(obj);

	if (event->dispose_has_run) {
		return;
	}

	event->dispose_has_run = TRUE;

	G_OBJECT_CLASS(parent_class)->dispose(obj);
}

static void con_ic_statistics_event_finalize(GObject *obj)
{
/* 	ConIcStatisticsEvent *event = CON_IC_STATISTICS_EVENT(obj); */
}

ConIcStatisticsEvent *con_ic_statistics_event_new()
{
	return g_object_new(con_ic_statistics_event_get_type(), NULL);
}

guint con_ic_statistics_event_get_time_active(ConIcStatisticsEvent *statistics)
{
	return statistics->time_active;
}

guint con_ic_statistics_event_get_signal_strength(ConIcStatisticsEvent *statistics)
{
	return statistics->signal_strength;
}

guint64 con_ic_statistics_event_get_rx_packets(ConIcStatisticsEvent *statistics)
{
	return statistics->rx_packets;
}

guint64 con_ic_statistics_event_get_tx_packets(ConIcStatisticsEvent *statistics)
{
	return statistics->tx_packets;
}

guint64 con_ic_statistics_event_get_rx_bytes(ConIcStatisticsEvent *statistics)
{
	return statistics->rx_bytes;
}

guint64 con_ic_statistics_event_get_tx_bytes(ConIcStatisticsEvent *statistics)
{
	return statistics->tx_bytes;
}

void con_ic_statistics_event_set_time_active(ConIcStatisticsEvent *statistics,
					     guint time)
{
	statistics->time_active = time;
}

void con_ic_statistics_event_set_signal_strength(ConIcStatisticsEvent *statistics,
						 guint strength)
{
	statistics->signal_strength = strength;
}

void con_ic_statistics_event_set_rx_packets(ConIcStatisticsEvent *statistics,
					    guint64 packets)
{
	statistics->rx_packets = packets;
}

void con_ic_statistics_event_set_tx_packets(ConIcStatisticsEvent *statistics,
					    guint64 packets)
{
	statistics->tx_packets = packets;
}

void con_ic_statistics_event_set_rx_bytes(ConIcStatisticsEvent *statistics,
					  guint64 bytes)
{
	statistics->rx_bytes = bytes;
}

void con_ic_statistics_event_set_tx_bytes(ConIcStatisticsEvent *statistics,
					  guint64 bytes)
{
	statistics->tx_bytes = bytes;
}

