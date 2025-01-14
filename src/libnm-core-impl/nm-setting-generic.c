/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2013 Red Hat, Inc.
 */

#include "libnm-core-impl/nm-default-libnm-core.h"

#include "nm-setting-generic.h"

#include "nm-setting-private.h"

/**
 * SECTION:nm-setting-generic
 * @short_description: Describes connection properties for generic devices
 *
 * The #NMSettingGeneric object is a #NMSetting subclass that describes
 * optional properties that apply to "generic" devices (ie, devices that
 * NetworkManager does not specifically recognize).
 *
 * There are currently no properties on this object; it exists only to be
 * the "connection type" setting on #NMConnections for generic devices.
 **/

/*****************************************************************************/

/**
 * NMSettingGeneric:
 *
 * Generic Link Settings
 */
struct _NMSettingGeneric {
    NMSetting parent;
};

struct _NMSettingGenericClass {
    NMSettingClass parent;
};

G_DEFINE_TYPE(NMSettingGeneric, nm_setting_generic, NM_TYPE_SETTING)

/*****************************************************************************/

static void
nm_setting_generic_init(NMSettingGeneric *setting)
{}

/**
 * nm_setting_generic_new:
 *
 * Creates a new #NMSettingGeneric object with default values.
 *
 * Returns: (transfer full): the new empty #NMSettingGeneric object
 **/
NMSetting *
nm_setting_generic_new(void)
{
    return g_object_new(NM_TYPE_SETTING_GENERIC, NULL);
}

static void
nm_setting_generic_class_init(NMSettingGenericClass *klass)
{
    NMSettingClass *setting_class = NM_SETTING_CLASS(klass);

    _nm_setting_class_commit(setting_class, NM_META_SETTING_TYPE_GENERIC, NULL, NULL, 0);
}
