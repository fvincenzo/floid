/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *                                        
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
#ifndef __DRV_CONF_H__
#define __DRV_CONF_H__
#include "autoconf.h"

#if defined (PLATFORM_LINUX) && defined (PLATFORM_WINDOWS)

#error "Shall be Linux or Windows, but not both!\n"

#endif

//Rockchips' android kernel doesn't has CONFIG_ANDROID defined,
//add this to force CONFIG_ANDROID defined
#ifdef CONFIG_PLATFORM_ANDROID
//BARBA: this symbol is defined into the linux kernel. 
//#define CONFIG_ANDROID
#endif

#ifdef RESUME_IN_WORKQUEUE
	#if !defined( CONFIG_WAKELOCK) && !defined(CONFIG_ANDROID_POWER)
	#error "enable RESUME_IN_WORKQUEUE without CONFIG_WAKELOCK or CONFIG_ANDROID_POWER will suffer from the danger of wifi's unfunctionality...\n"
	#error "If you still want to enable RESUME_IN_WORKQUEUE in this case, mask this preprossor checking and GOOD LUCK...\n"
	#endif
#endif


//#include <rtl871x_byteorder.h>

#endif // __DRV_CONF_H__

