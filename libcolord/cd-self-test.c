/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2010 Richard Hughes <richard@hughsie.com>
 *
 * Licensed under the GNU General Public License Version 2
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "config.h"

#include <limits.h>
#include <stdlib.h>

#include <glib.h>
#include <glib-object.h>

#include "cd-client.h"

/** ver:1.0 ***********************************************************/
static GMainLoop *_test_loop = NULL;
static guint _test_loop_timeout_id = 0;

static gboolean
_g_test_hang_check_cb (gpointer user_data)
{
	g_main_loop_quit (_test_loop);
	_test_loop_timeout_id = 0;
	return FALSE;
}

/**
 * _g_test_loop_run_with_timeout:
 **/
static void
_g_test_loop_run_with_timeout (guint timeout_ms)
{
	g_assert (_test_loop_timeout_id == 0);
	_test_loop = g_main_loop_new (NULL, FALSE);
	_test_loop_timeout_id = g_timeout_add (timeout_ms, _g_test_hang_check_cb, NULL);
	g_main_loop_run (_test_loop);
}

/**
 * _g_test_realpath:
 **/
static gchar *
_g_test_realpath (const gchar *relpath)
{
	gchar *full;
	char full_tmp[PATH_MAX];
	realpath (relpath, full_tmp);
	full = g_strdup (full_tmp);
	return full;
}

/**********************************************************************/

static void
colord_client_func (void)
{
	CdClient *client;
	CdDevice *device;
	CdProfile *profile;
	CdProfile *profile_tmp;
	gboolean ret;
	gchar *filename;
	GError *error = NULL;
	GPtrArray *array;
	GPtrArray *devices;
	GPtrArray *profiles;

	/* create */
	client = cd_client_new ();
	g_assert (client != NULL);

	/* connect */
	ret = cd_client_connect_sync (client, NULL, &error);
	g_assert_no_error (error);
	g_assert (ret);

	/* get number of devices */
	devices = cd_client_get_devices_sync (client, NULL, &error);
	g_assert_no_error (error);
	g_assert (devices != NULL);

	/* get number of profiles */
	profiles = cd_client_get_profiles_sync (client, NULL, &error);
	g_assert_no_error (error);
	g_assert (profiles != NULL);

	/* create device */
	device = cd_client_create_device_sync (client,
					       "device-self-test",
					       CD_DBUS_OPTIONS_MASK_TEMP,
					       NULL,
					       &error);

	g_assert_cmpstr (cd_device_get_object_path (device), ==,
			 "/org/freedesktop/ColorManager/device_self_test");
	g_assert_cmpstr (cd_device_get_id (device), ==, "device-self-test");

	/* get new number of devices */
	array = cd_client_get_devices_sync (client, NULL, &error);
	g_assert_no_error (error);
	g_assert (array != NULL);
	g_assert_cmpint (devices->len + 1, ==, array->len);
	g_ptr_array_unref (array);

	/* set device model */
	ret = cd_device_set_model_sync (device, "CRAY 3000", NULL, &error);
	g_assert_no_error (error);
	g_assert (ret);

	/* set device model */
	ret = cd_device_set_kind_sync (device, CD_DEVICE_KIND_DISPLAY,
				       NULL, &error);
	g_assert_no_error (error);
	g_assert (ret);

	/* wait for daemon */
	_g_test_loop_run_with_timeout (50);

	/* check device model */
	g_assert_cmpstr (cd_device_get_model (device), ==, "CRAY 3000");

	/* check device kind */
	g_assert_cmpint (cd_device_get_kind (device), ==, CD_DEVICE_KIND_DISPLAY);

	/* create profile */
	profile = cd_client_create_profile_sync (client,
						 "profile-self-test",
						 CD_DBUS_OPTIONS_MASK_TEMP,
						 NULL,
						 &error);

	g_assert_cmpstr (cd_profile_get_object_path (profile), ==,
			 "/org/freedesktop/ColorManager/profile_self_test");
	g_assert_cmpstr (cd_profile_get_id (profile), ==, "profile-self-test");

	/* get new number of profiles */
	array = cd_client_get_profiles_sync (client, NULL, &error);
	g_assert_no_error (error);
	g_assert (array != NULL);
	g_assert_cmpint (profiles->len + 1, ==, array->len);
	g_ptr_array_unref (array);

	/* set profile filename */
	filename = _g_test_realpath (TESTDATADIR "/ibm-t61.icc");
	ret = cd_profile_set_filename_sync (profile, filename, NULL, &error);
	g_assert_no_error (error);
	g_assert (ret);

	/* set profile qualifier */
	ret = cd_profile_set_qualifier_sync (profile, "Epson.RGB.300dpi",
					     NULL, &error);
	g_assert_no_error (error);
	g_assert (ret);

	/* wait for daemon */
	_g_test_loop_run_with_timeout (50);

	/* check profile filename */
	g_assert (g_str_has_suffix (cd_profile_get_filename (profile), "data/tests/ibm-t61.icc"));

	/* check profile qualifier */
	g_assert_cmpstr (cd_profile_get_qualifier (profile), ==, "Epson.RGB.300dpi");

	/* check profile title set from ICC profile */
	g_assert_cmpstr (cd_profile_get_title (profile), ==, "Huey, LENOVO - 6464Y1H - 15\" (2009-12-23)");

	/* check none assigned */
	array = cd_device_get_profiles (device);
	g_assert_cmpint (array->len, ==, 0);
	g_ptr_array_unref (array);

	/* check nothing matches qualifier */
	profile_tmp = cd_device_get_profile_for_qualifier_sync (device,
								"Epson.RGB.300dpi",
								NULL,
								&error);
	g_assert_error (error, CD_DEVICE_ERROR, CD_DEVICE_ERROR_FAILED);
	g_assert (profile_tmp == NULL);
	g_clear_error (&error);

	/* assign profile to device */
	ret = cd_device_add_profile_sync (device, profile, NULL, &error);
	g_assert_no_error (error);
	g_assert (ret);

	/* wait for daemon */
	_g_test_loop_run_with_timeout (50);

	/* check profile assigned */
	array = cd_device_get_profiles (device);
	g_assert (array != NULL);
	g_assert_cmpint (array->len, ==, 1);
	profile_tmp = g_ptr_array_index (array, 0);
	g_assert_cmpstr (cd_profile_get_qualifier (profile_tmp), ==, "Epson.RGB.300dpi");
	g_ptr_array_unref (array);

	/* make profile default */
	ret = cd_device_make_profile_default_sync (device, profile, NULL, &error);
	g_assert_no_error (error);
	g_assert (ret);

	/* check matches exact qualifier */
	profile_tmp = cd_device_get_profile_for_qualifier_sync (device,
								"Epson.RGB.300dpi",
								NULL,
								&error);
	g_assert_no_error (error);
	g_assert (profile_tmp != NULL);
	g_assert_cmpstr (cd_profile_get_object_path (profile), ==,
			 "/org/freedesktop/ColorManager/profile_self_test");
	g_object_unref (profile_tmp);

	/* check matches wildcarded qualifier */
	profile_tmp = cd_device_get_profile_for_qualifier_sync (device,
								"Epson.RGB.*",
								NULL,
								&error);
	g_assert_no_error (error);
	g_assert (profile_tmp != NULL);
	g_assert_cmpstr (cd_profile_get_object_path (profile), ==,
			 "/org/freedesktop/ColorManager/profile_self_test");
	g_object_unref (profile_tmp);

	/* delete profile */
	ret = cd_client_delete_profile_sync (client, profile, NULL, &error);
	g_assert_no_error (error);
	g_assert (ret);

	/* get new number of profiles */
	array = cd_client_get_profiles_sync (client, NULL, &error);
	g_assert_no_error (error);
	g_assert (array != NULL);
	g_assert_cmpint (profiles->len, ==, array->len);
	g_ptr_array_unref (array);

	/* wait for daemon */
	_g_test_loop_run_with_timeout (50);

	/* ensure device no longer lists deleted profile */
	array = cd_device_get_profiles (device);
	g_assert (array != NULL);
	g_assert_cmpint (array->len, ==, 0);
	g_ptr_array_unref (array);

	/* delete device */
	ret = cd_client_delete_device_sync (client, device, NULL, &error);
	g_assert_no_error (error);
	g_assert (ret);

	/* get new number of devices */
	array = cd_client_get_devices_sync (client, NULL, &error);
	g_assert_no_error (error);
	g_assert (array != NULL);
	g_assert_cmpint (devices->len, ==, array->len);
	g_ptr_array_unref (array);

	g_free (filename);
	g_ptr_array_unref (devices);
	g_ptr_array_unref (profiles);
	g_object_unref (device);
	g_object_unref (profile);
	g_object_unref (client);
}

int
main (int argc, char **argv)
{
	g_type_init ();
	g_thread_init (NULL);
	g_test_init (&argc, &argv, NULL);

	/* only critical and error are fatal */
	g_log_set_fatal_mask (NULL, G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL);

	/* tests go here */
	g_test_add_func ("/colord/client", colord_client_func);
	return g_test_run ();
}
