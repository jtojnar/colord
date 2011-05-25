/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2011 Richard Hughes <richard@hughsie.com>
 *
 * Licensed under the GNU Lesser General Public License Version 2.1
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#if !defined (__COLORD_H_INSIDE__) && !defined (CD_COMPILATION)
#error "Only <colord.h> can be included directly."
#endif

#ifndef __CD_DEVICE_SYNC_H
#define __CD_DEVICE_SYNC_H

#include <glib-object.h>

#include "cd-device.h"

G_BEGIN_DECLS

gboolean	 cd_device_set_object_path_sync		(CdDevice	*device,
							 const gchar	*object_path,
							 GCancellable	*cancellable,
							 GError		**error);
gboolean	 cd_device_set_model_sync		(CdDevice	*device,
							 const gchar	*value,
							 GCancellable	*cancellable,
							 GError		**error);
gboolean	 cd_device_set_serial_sync		(CdDevice	*device,
							 const gchar	*value,
							 GCancellable	*cancellable,
							 GError		**error);
gboolean	 cd_device_set_vendor_sync		(CdDevice	*device,
							 const gchar	*value,
							 GCancellable	*cancellable,
							 GError		**error);
gboolean	 cd_device_set_kind_sync		(CdDevice	*device,
							 CdDeviceKind	 kind,
							 GCancellable	*cancellable,
							 GError		**error);
gboolean	 cd_device_set_colorspace_sync		(CdDevice	*device,
							 CdColorspace	 colorspace,
							 GCancellable	*cancellable,
							 GError		**error);
gboolean	 cd_device_set_mode_sync		(CdDevice	*device,
							 CdDeviceMode	 mode,
							 GCancellable	*cancellable,
							 GError		**error);

G_END_DECLS

#endif /* __CD_DEVICE_SYNC_H */

