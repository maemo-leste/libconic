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

#include <gconf/gconf-client.h>
#include <osso-ic-gconf.h>

#include "coniciap.h"
#include "coniciap-private.h"

struct _ConIcIapClass
{
	GObjectClass parent_class;
};

struct _ConIcIap 
{
	GObject parent_instance;
	gboolean dispose_has_run;
	gchar *id;
	gchar *name;
	gchar *bearer;
};
	
static GObjectClass *parent_class = NULL;

static void con_ic_iap_class_init(gpointer g_class,
					   gpointer class_data);
static void con_ic_iap_init(GTypeInstance *instance, gpointer g_class);
static void con_ic_iap_dispose(GObject *obj);
static void con_ic_iap_finalize(GObject *obj);


GType con_ic_iap_get_type(void)
{
	static GType type = 0;
	if (type == 0){
		static const GTypeInfo info =  {
			.class_size = sizeof (ConIcIapClass),
			.base_init = NULL,
			.base_finalize = NULL,
			.class_init = con_ic_iap_class_init,
			.class_finalize = NULL,
			.class_data = NULL,
			.instance_size = sizeof (ConIcIap),
			.n_preallocs = 0,
			.instance_init = con_ic_iap_init
		};
		type = g_type_register_static (G_TYPE_OBJECT,
					       "ConIcIap",
					       &info,
					       0);
	}
	return type;
}

static void con_ic_iap_class_init(gpointer g_class,
				      gpointer class_data)
{
	GObjectClass *gobject_klass = G_OBJECT_CLASS(g_class);
/* 	ConIcIapClass *klass = CON_IC_IAP_CLASS(g_class); */

	parent_class = g_type_class_peek_parent(gobject_klass);
	
	gobject_klass->dispose  = con_ic_iap_dispose;
	gobject_klass->finalize = con_ic_iap_finalize;
}

static void con_ic_iap_init(GTypeInstance *instance, gpointer g_class)
{
	ConIcIap *self = CON_IC_IAP(instance);

	self->id = NULL;
	self->name = NULL;
	self->bearer = NULL;
	
}

static void con_ic_iap_dispose(GObject *obj)
{
	ConIcIap *self = CON_IC_IAP(obj);
	
	if (self->dispose_has_run) {
		return;
	}

	self->dispose_has_run = TRUE;

	G_OBJECT_CLASS(parent_class)->dispose(obj);
}

static void con_ic_iap_finalize(GObject *obj)
{
	ConIcIap *self = CON_IC_IAP(obj);

	g_free(self->id);
	g_free(self->name);
	g_free(self->bearer);
}

ConIcIap *con_ic_iap_new(const char *id)
{
  GConfClient *gconf;
  char *escaped;
  char *path;
  GError *gconf_error = NULL;
  ConIcIap *iap = NULL;
  
  gconf = gconf_client_get_default();
  escaped = gconf_escape_key(id, -1);
  path = g_strconcat (ICD_GCONF_PATH, "/", escaped, NULL);
  g_free (escaped);

  if (gconf_client_dir_exists (gconf, path, &gconf_error)) {
    iap = g_object_new(con_ic_iap_get_type(), NULL);
    iap->id = g_strdup(id);
  }
  
  if (gconf_error)
    g_error_free (gconf_error);

  g_free (path);
  g_object_unref (gconf);

  return iap;
}


const gchar *con_ic_iap_get_name(ConIcIap *iap)
{
  GConfClient *gconf_client;
  char *escaped, *key;
  
  if (iap == NULL || iap->id == NULL)
    return NULL;

  gconf_client = gconf_client_get_default();
  escaped = gconf_escape_key(iap->id, -1);
  key = g_strconcat (ICD_GCONF_PATH, "/", escaped, "/name", NULL);
  g_free (escaped);

  g_free (iap->name);

  /* check wheter the IAP has a name attribute */
  iap->name = gconf_client_get_string (gconf_client, key, NULL);
  g_free (key);

  /* default is to use the id as the IAP name */
  if (iap->name == NULL)
    iap->name = g_strdup (iap->id);

  g_object_unref (gconf_client);

  return iap->name;
}


const gchar *con_ic_iap_get_id(ConIcIap *iap)
{
	if (iap == NULL)
	  return NULL;

	return iap->id;
}


const gchar *con_ic_iap_get_bearer_type(ConIcIap *iap)
{
	GConfClient *gconf; 
	char *escaped;
	char *path;

	if (iap == NULL)
	  return NULL;

	gconf = gconf_client_get_default();
	escaped = gconf_escape_key (iap->id, -1);
	path = g_strconcat(ICD_GCONF_PATH, "/", escaped,
			   "/type", NULL);
	g_free (escaped);
	g_free(iap->bearer);
	iap->bearer = gconf_client_get_string(gconf, path, NULL);

	g_free(path);
	g_object_unref (gconf);
	return iap->bearer;
}
