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

#ifndef CONICEVENT_PRIVATE_H
#define CONICEVENT_PRIVATE_H

#include "conicevent.h"

G_BEGIN_DECLS

struct _ConIcEventClass
{
	GObjectClass parent_class;
};

struct _ConIcEvent 
{
	GObject parent_instance;
	char *id;
	char *bearer_type;
	gboolean dispose_has_run;
};

void con_ic_event_set_iap_id(ConIcEvent *event, const gchar *id);

void con_ic_event_set_bearer_type(ConIcEvent *event, const gchar *type);

G_END_DECLS

#endif /* CONICEVENT_PRIVATE_H */
