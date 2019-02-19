/*
 * Copyright (c) 2014 Tom Wambold <tom5760@gmail.com>
 * Copyright (c) 2014 Marius L. Jøhndal <mariuslj@ifi.uio.no>
 * Copyright (c) 2019 Moritz Bruder <muesli4 at gmail dot com>
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <iostream>
#include <sstream>
#include <optional>

#include <cstdio>
#include <cstring>

#define UDISKS_API_IS_SUBJECT_TO_CHANGE
#include <udisks/udisks.h>

int generic_link
    ( char const * const cmd
    , char const * const label
    , char const * const uuid
    , char const * const mount_path
    )
{
    std::stringstream ss;
    ss << cmd << " '" << label << "' '" << uuid << "'";

    if (mount_path != nullptr)
    {
        ss << " '" << mount_path << '\'' << std::endl;
    }

    return std::system(ss.str().c_str());
}

int add_link
    ( char const * const label
    , char const * const uuid
    , char const * const mount_path
    )
{
    char const * const ADD_CMD = "mpd-automount-link.sh add";
    return generic_link(ADD_CMD, label, uuid, mount_path);
}

int remove_link
    ( char const * const label
    , char const * const uuid
    )
{
    char const * const REMOVE_CMD = "mpd-automount-link.sh remove";
    return generic_link(REMOVE_CMD, label, uuid, nullptr);
}

static char const BLOCK_DEVICES_PATH [] = "/org/freedesktop/UDisks2/block_devices/";
static std::size_t const BLOCK_DEVICES_LENGTH = sizeof(BLOCK_DEVICES_PATH) / sizeof(char) - 1;

struct filesystem_result
{
    UDisksFilesystem * filesystem;
    char const * const label;
    char const * const uuid;
};

std::optional<filesystem_result> get_filesystem_from_dbus_object
    ( GDBusObject * dbus_object
    , char const * const path
    )
{
    if (strncmp(path, BLOCK_DEVICES_PATH, BLOCK_DEVICES_LENGTH) != 0)
    {
        return std::nullopt;
    }

    UDisksObject * object = UDISKS_OBJECT(dbus_object);

    UDisksBlock * block = udisks_object_peek_block(object);

    if (block == nullptr)
    {
        return std::nullopt;
    }

    UDisksFilesystem * filesystem = udisks_object_peek_filesystem(object);
    if (filesystem == nullptr)
        return std::nullopt;

    auto const label = udisks_block_get_id_label(block);
    auto const uuid = udisks_block_get_id_uuid(block);

    return filesystem_result{filesystem, label, uuid};

}

void log_filesystem_action
    ( char const * const action
    , char const * const path
    , filesystem_result const & result
    )
{
    std::cout << "udisks2 block device " << action << ": "
              // cut off prefix
              << (path + BLOCK_DEVICES_LENGTH)
              << " uuid='" << result.uuid << '\''
              << " label='" << result.label << '\''
              << std::endl;
}

static void on_object_removed
    ( GDBusObjectManager * manager
    , GDBusObject * dbus_object
    , gpointer user_data
    )
{
    const char * path = g_dbus_object_get_object_path(dbus_object);
    auto opt_result = get_filesystem_from_dbus_object(dbus_object, path);
    if (!opt_result.has_value())
    {
        return;
    }
    auto & result = opt_result.value();
    log_filesystem_action("removed", path, result);

    if (remove_link(result.label, result.uuid) != 0)
    {
        std::cerr << "removing link failed" << std::endl;
    }
}

static void on_object_added
    ( GDBusObjectManager * manager
    , GDBusObject * dbus_object
    , gpointer user_data
    )
{

    const char * path = g_dbus_object_get_object_path(dbus_object);
    auto opt_result = get_filesystem_from_dbus_object(dbus_object, path);
    if (!opt_result.has_value())
    {
        return;
    }
    auto & result = opt_result.value();
    log_filesystem_action("added", path, result);

    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);
    g_variant_builder_add(&builder, "{sv}", "auth.no_user_interaction", g_variant_new_boolean(TRUE));
    // TODO mount read-only?

    GVariant * options = g_variant_builder_end (&builder);
    g_variant_ref_sink (options);

    gchar * mount_path = nullptr;
    GError * error = nullptr;
    if (!udisks_filesystem_call_mount_sync
            ( result.filesystem
            , options
            , &mount_path
            , nullptr, &error
            ))
    {
        std::cerr << "failed to mount: " << error->message << std::endl;
        g_error_free(error);
    }
    else
    {
        std::cout << "mounted at " << mount_path << std::endl;

        if (add_link(result.label, result.uuid, mount_path) != 0)
        {
            std::cerr << "adding link failed" << std::endl;
        }

        g_free(mount_path);
    }
    g_variant_unref(options);
}

static void on_interface_added( GDBusObjectManager * manager
                              , GDBusObject * dbus_object
                              , GDBusInterface * interface
                              , gpointer user_data
                              )
{
    on_object_added(manager, dbus_object, user_data);
}

int main()
{
    GMainLoop * loop = g_main_loop_new(nullptr, FALSE);

    GError * error = nullptr;
    UDisksClient * client = udisks_client_new_sync(nullptr, &error);
    if (client == nullptr)
    {
        std::cerr << "Error connecting to the udisks daemon: "
                  << error->message << std::endl;
        g_error_free(error);
        g_main_loop_unref(loop);
        return 1;
    }
    // TODO link already connected block devices if they aren't yet

    GDBusObjectManager * manager = udisks_client_get_object_manager(client);
    g_signal_connect(manager, "object-added", G_CALLBACK(on_object_added), NULL);
    g_signal_connect(manager, "interface-added", G_CALLBACK(on_interface_added), NULL);
    g_signal_connect(manager, "object-removed", G_CALLBACK(on_object_removed), NULL);

    g_main_loop_run(loop);

    // TODO safe devices on add and then remove link here ?

    g_object_unref(client);
    g_main_loop_unref(loop);

    return 0;
}
