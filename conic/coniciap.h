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

#ifndef CONICIAP_H
#define CONICIAP_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

/** 
 * @file coniciap.h
 *
 * ConIcIap class.
 *
 * ConIcIap class is for access IAPs.
 */

#define CON_IC_TYPE_IAP (con_ic_iap_get_type ())
#define CON_IC_IAP(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj),\
				CON_IC_TYPE_IAP, ConIcIap))
#define CON_IC_IAP_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), CON_IC_TYPE_IAP, ConIcIapClass))
#define CON_IC_IS_IAP(obj)	 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CON_IC_TYPE_IAP))
#define CON_IC_IS_IAP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CON_IC_TYPE_IAP))
#define CON_IC_IAP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), CON_IC_TYPE_IAP, ConIcIapClass))

/** 
 * ConIcConnectionEvent object. The contents of the object is private, to
 * access the object use the functions provided by conicconnectionevent.h.
 */
typedef struct _ConIcIap ConIcIap;
typedef struct _ConIcIapClass  ConIcIapClass;

GType con_ic_iap_get_type(void);

/**
 * Returns the name of the IAP as string.
 *
 * @param iap ConIcIap object
 * @return Name of the IAP.
 */
const gchar *con_ic_iap_get_name(ConIcIap *iap);

/**
 * Returns the ID of the IAP as string.
 *
 * @param iap ConIcIap object
 * @return ID of the IAP.
 */
const gchar *con_ic_iap_get_id(ConIcIap *iap);

/**
 * Returns the bearer type of the IAP as string.
 *
 * @param iap ConIcIap object
 * @return Bearer type of the IAP.
 */
const gchar *con_ic_iap_get_bearer_type(ConIcIap *iap);

/**
 * @name Bearers
 *
 * The bearer types returned by con_ic_iap_get_bearer_type() and
 * con_ic_event_get_bearer_type().
 *
 * @note More bearer types might be added in the future.
 */
/*@{*/
/** WLAN infrastructure mode. */
#define CON_IC_BEARER_WLAN_INFRA "WLAN_INFRA"

/** WLAN Ad-Hoc mode. */
#define CON_IC_BEARER_WLAN_ADHOC "WLAN_ADHOC"

/** Bluetooth Dial-up Networking profile using GSM circuit-switched. */
#define CON_IC_BEARER_DUN_GSM_CS "DUN_GSM_CS"

/** Bluetooth Dial-up Networking profile using GSM packet-switched. */
#define CON_IC_BEARER_DUN_GSM_PS "DUN_GSM_PS"
/*@}*/

G_END_DECLS

#endif /* CONICIAP_H */
