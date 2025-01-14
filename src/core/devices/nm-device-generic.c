/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2013 Red Hat, Inc.
 */

#include "src/core/nm-default-daemon.h"

#include "nm-device-generic.h"

#include "nm-device-private.h"
#include "libnm-platform/nm-platform.h"
#include "libnm-core-intern/nm-core-internal.h"

/*****************************************************************************/

NM_GOBJECT_PROPERTIES_DEFINE_BASE(PROP_TYPE_DESCRIPTION, );

typedef struct {
    const char *type_description;
} NMDeviceGenericPrivate;

struct _NMDeviceGeneric {
    NMDevice               parent;
    NMDeviceGenericPrivate _priv;
};

struct _NMDeviceGenericClass {
    NMDeviceClass parent;
};

G_DEFINE_TYPE(NMDeviceGeneric, nm_device_generic, NM_TYPE_DEVICE)

#define NM_DEVICE_GENERIC_GET_PRIVATE(self) \
    _NM_GET_PRIVATE(self, NMDeviceGeneric, NM_IS_DEVICE_GENERIC, NMDevice)

/*****************************************************************************/

static NMDeviceCapabilities
get_generic_capabilities(NMDevice *device)
{
    int ifindex = nm_device_get_ifindex(device);

    if (ifindex > 0
        && nm_platform_link_supports_carrier_detect(nm_device_get_platform(device), ifindex))
        return NM_DEVICE_CAP_CARRIER_DETECT;
    else
        return NM_DEVICE_CAP_NONE;
}

static const char *
get_type_description(NMDevice *device)
{
    if (NM_DEVICE_GENERIC_GET_PRIVATE(device)->type_description)
        return NM_DEVICE_GENERIC_GET_PRIVATE(device)->type_description;
    return NM_DEVICE_CLASS(nm_device_generic_parent_class)->get_type_description(device);
}

static void
realize_start_notify(NMDevice *device, const NMPlatformLink *plink)
{
    NMDeviceGeneric        *self = NM_DEVICE_GENERIC(device);
    NMDeviceGenericPrivate *priv = NM_DEVICE_GENERIC_GET_PRIVATE(self);
    int                     ifindex;

    NM_DEVICE_CLASS(nm_device_generic_parent_class)->realize_start_notify(device, plink);

    ifindex = nm_device_get_ip_ifindex(NM_DEVICE(self));
    if (ifindex > 0) {
        priv->type_description =
            nm_platform_link_get_type_name(nm_device_get_platform(device), ifindex);
    }
}

static gboolean
check_connection_compatible(NMDevice     *device,
                            NMConnection *connection,
                            gboolean      check_properties,
                            GError      **error)
{
    NMSettingConnection *s_con;

    if (!NM_DEVICE_CLASS(nm_device_generic_parent_class)
             ->check_connection_compatible(device, connection, check_properties, error))
        return FALSE;

    s_con = nm_connection_get_setting_connection(connection);
    if (!nm_setting_connection_get_interface_name(s_con)) {
        nm_utils_error_set_literal(error,
                                   NM_UTILS_ERROR_CONNECTION_AVAILABLE_TEMPORARY,
                                   "generic profiles need an interface name");
        return FALSE;
    }

    return TRUE;
}

static void
update_connection(NMDevice *device, NMConnection *connection)
{
    NMSettingConnection *s_con;

    if (!nm_connection_get_setting_generic(connection))
        nm_connection_add_setting(connection, nm_setting_generic_new());

    s_con = nm_connection_get_setting_connection(connection);
    g_assert(s_con);
    g_object_set(G_OBJECT(s_con),
                 NM_SETTING_CONNECTION_INTERFACE_NAME,
                 nm_device_get_iface(device),
                 NULL);
}

/*****************************************************************************/

static void
get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
    NMDeviceGeneric        *self = NM_DEVICE_GENERIC(object);
    NMDeviceGenericPrivate *priv = NM_DEVICE_GENERIC_GET_PRIVATE(self);

    switch (prop_id) {
    case PROP_TYPE_DESCRIPTION:
        g_value_set_string(value, priv->type_description);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

/*****************************************************************************/

static void
nm_device_generic_init(NMDeviceGeneric *self)
{}

static GObject *
constructor(GType type, guint n_construct_params, GObjectConstructParam *construct_params)
{
    GObject *object;

    object = G_OBJECT_CLASS(nm_device_generic_parent_class)
                 ->constructor(type, n_construct_params, construct_params);

    nm_device_set_unmanaged_flags((NMDevice *) object, NM_UNMANAGED_BY_DEFAULT, TRUE);

    return object;
}

NMDevice *
nm_device_generic_new(const NMPlatformLink *plink, gboolean nm_plugin_missing)
{
    g_return_val_if_fail(plink != NULL, NULL);

    return g_object_new(NM_TYPE_DEVICE_GENERIC,
                        NM_DEVICE_IFACE,
                        plink->name,
                        NM_DEVICE_TYPE_DESC,
                        "Generic",
                        NM_DEVICE_DEVICE_TYPE,
                        NM_DEVICE_TYPE_GENERIC,
                        NM_DEVICE_NM_PLUGIN_MISSING,
                        nm_plugin_missing,
                        NULL);
}

static const NMDBusInterfaceInfoExtended interface_info_device_generic = {
    .parent = NM_DEFINE_GDBUS_INTERFACE_INFO_INIT(
        NM_DBUS_INTERFACE_DEVICE_GENERIC,
        .properties = NM_DEFINE_GDBUS_PROPERTY_INFOS(
            NM_DEFINE_DBUS_PROPERTY_INFO_EXTENDED_READABLE(
                "HwAddress",
                "s",
                NM_DEVICE_HW_ADDRESS,
                .annotations = NM_GDBUS_ANNOTATION_INFO_LIST_DEPRECATED(), ),
            NM_DEFINE_DBUS_PROPERTY_INFO_EXTENDED_READABLE(
                "TypeDescription",
                "s",
                NM_DEVICE_GENERIC_TYPE_DESCRIPTION), ), ),
};

static void
nm_device_generic_class_init(NMDeviceGenericClass *klass)
{
    GObjectClass      *object_class      = G_OBJECT_CLASS(klass);
    NMDBusObjectClass *dbus_object_class = NM_DBUS_OBJECT_CLASS(klass);
    NMDeviceClass     *device_class      = NM_DEVICE_CLASS(klass);

    object_class->constructor  = constructor;
    object_class->get_property = get_property;

    dbus_object_class->interface_infos = NM_DBUS_INTERFACE_INFOS(&interface_info_device_generic);

    device_class->connection_type_supported        = NM_SETTING_GENERIC_SETTING_NAME;
    device_class->connection_type_check_compatible = NM_SETTING_GENERIC_SETTING_NAME;
    device_class->link_types                       = NM_DEVICE_DEFINE_LINK_TYPES(NM_LINK_TYPE_ANY);

    device_class->realize_start_notify        = realize_start_notify;
    device_class->get_generic_capabilities    = get_generic_capabilities;
    device_class->get_type_description        = get_type_description;
    device_class->check_connection_compatible = check_connection_compatible;
    device_class->update_connection           = update_connection;

    obj_properties[PROP_TYPE_DESCRIPTION] =
        g_param_spec_string(NM_DEVICE_GENERIC_TYPE_DESCRIPTION,
                            "",
                            "",
                            NULL,
                            G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties(object_class, _PROPERTY_ENUMS_LAST, obj_properties);
}
