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

#ifndef CONICSTATISTICSEVENT_H
#define CONICSTATISTICSEVENT_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

/** 
 * @file conicstatisticsevent.h
 *
 * ConIcStatisticsEvent class.
 *
 * ConIcStatisticsEvent is sent when an application has requested
 * statistics. It's derived from ConIcEvent class.
 */

#define CON_IC_TYPE_STATISTICS_EVENT (con_ic_statistics_event_get_type ())
#define CON_IC_STATISTICS_EVENT(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), CON_IC_TYPE_STATISTICS_EVENT, \
		ConIcStatisticsEvent))
#define CON_IC_STATISTICS_EVENT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST ((klass), \
		CON_IC_TYPE_STATISTICS_EVENT, ConIcStatisticsEventClass))
#define CON_IC_IS_STATISTICS_EVENT(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), CON_IC_TYPE_STATISTICS_EVENT))
#define CON_IC_IS_STATISTICS_EVENT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), CON_IC_TYPE_STATISTICS_EVENT))
#define CON_IC_STATISTICS_EVENT_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), CON_IC_TYPE_STATISTICS_EVENT, \
		ConIcStatisticsEventClass))

/** 
 * ConIcStatisticsEvent object. The contents of the object is private, to
 * access the object use the functions provided by conicstatisticsevent.h.
 */
typedef struct _ConIcStatisticsEvent ConIcStatisticsEvent;
typedef struct _ConIcStatisticsEventClass  ConIcStatisticsEventClass;

GType con_ic_statistics_event_get_type(void);

/**
 * Get the active time from the event.
 *
 * @param statistics ConIcStatisticsEvent object.
 * @return Active time.
 */
guint con_ic_statistics_event_get_time_active(ConIcStatisticsEvent *statistics);

/**
 * Get the signal strength from the event.
 *
 * @param statistics ConIcStatisticsEvent object.
 * @return Signal strength.
 */
guint con_ic_statistics_event_get_signal_strength(ConIcStatisticsEvent *statistics);

/**
 * Get the number of received packets from the event.
 *
 * @param statistics ConIcStatisticsEvent object.
 * @return Received packets.
 */
guint64 con_ic_statistics_event_get_rx_packets(ConIcStatisticsEvent *statistics);

/**
 * Get the number of transmitted packets from the event.
 *
 * @param statistics ConIcStatisticsEvent object.
 * @return Transmitted packets.
 */
guint64 con_ic_statistics_event_get_tx_packets(ConIcStatisticsEvent *statistics);

/**
 * Get the received bytes from the event.
 *
 * @param statistics ConIcStatisticsEvent object.
 * @return Received bytes.
 */
guint64 con_ic_statistics_event_get_rx_bytes(ConIcStatisticsEvent *statistics);

/**
 * Get the transfered bytes from the event.
 *
 * @param statistics ConIcStatisticsEvent object.
 * @return Transfered bytes.
 */
guint64 con_ic_statistics_event_get_tx_bytes(ConIcStatisticsEvent *statistics);

G_END_DECLS

#endif /* CONICSTATISTICSEVENT_H */
