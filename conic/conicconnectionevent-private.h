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

#ifndef CONICCONNECTIONEVENT_PRIVATE_H
#define CONICCONNECTIONEVENT_PRIVATE_H


G_BEGIN_DECLS

ConIcConnectionEvent *con_ic_connection_event_new(void);
void con_ic_connection_event_set_status(ConIcConnectionEvent *event,
					  ConIcConnectionStatus status);
void con_ic_connection_event_set_error(ConIcConnectionEvent *event,
					 ConIcConnectionError error);



G_END_DECLS

#endif /* CONICCONNECTIONEVENT_PRIVATE_H */
