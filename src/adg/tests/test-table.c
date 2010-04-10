/* ADG - Automatic Drawing Generation
 * Copyright (C) 2010  Nicola Fontana <ntd at entidi.it>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */


#include "test-internal.h"


static void
_adg_test_table_dress(void)
{
    AdgTable *table;
    AdgDress valid_dress, incompatible_dress;
    AdgDress table_dress;

    table = adg_table_new();
    valid_dress = ADG_DRESS_TABLE;
    incompatible_dress = ADG_DRESS_LINE;

    /* Using the public APIs */
    adg_table_set_table_dress(table, valid_dress);
    table_dress = adg_table_get_table_dress(table);
    g_assert_cmpint(table_dress, ==, valid_dress);

    adg_table_set_table_dress(table, incompatible_dress);
    table_dress = adg_table_get_table_dress(table);
    g_assert_cmpint(table_dress, ==, valid_dress);

    /* Using GObject property methods */
    g_object_set(table, "table-dress", valid_dress, NULL);
    g_object_get(table, "table-dress", &table_dress, NULL);
    g_assert_cmpint(table_dress, ==, valid_dress);

    g_object_set(table, "table-dress", incompatible_dress, NULL);
    g_object_get(table, "table-dress", &table_dress, NULL);
    g_assert_cmpint(table_dress, ==, valid_dress);

    g_object_unref(table);
}

static void
_adg_test_has_frame(void)
{
    AdgTable *table;
    gboolean invalid_boolean;
    gboolean has_frame;

    table = adg_table_new();
    invalid_boolean = (gboolean) 1234;

    /* Using the public APIs */
    adg_table_switch_frame(table, FALSE);
    has_frame = adg_table_has_frame(table);
    g_assert(!has_frame);

    adg_table_switch_frame(table, invalid_boolean);
    has_frame = adg_table_has_frame(table);
    g_assert(!has_frame);

    adg_table_switch_frame(table, TRUE);
    has_frame = adg_table_has_frame(table);
    g_assert(has_frame);

    /* Using GObject property methods */
    g_object_set(table, "has-frame", FALSE, NULL);
    g_object_get(table, "has-frame", &has_frame, NULL);
    g_assert(!has_frame);

    g_object_set(table, "has-frame", invalid_boolean, NULL);
    g_object_get(table, "has-frame", &has_frame, NULL);
    g_assert(!has_frame);

    g_object_set(table, "has-frame", TRUE, NULL);
    g_object_get(table, "has-frame", &has_frame, NULL);
    g_assert(has_frame);

    g_object_unref(table);
}


int
main(int argc, char *argv[])
{
    adg_test_init(&argc, &argv);

    adg_test_add_func("/adg/table/table-dress", _adg_test_table_dress);
    adg_test_add_func("/adg/table/has-frame", _adg_test_has_frame);

    return g_test_run();
}