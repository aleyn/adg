/* ADG - Automatic Drawing Generation
 * Copyright (C) 2007,2008,2009 Nicola Fontana <ntd at entidi.it>
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


/**
 * SECTION:adg-marker
 * @short_description: Base entity for markers
 *
 * A marker is an entity to be applied at the start or end of a segment.
 * Typical examples include arrows, ticks, dots and so on.
 **/

/**
 * AdgMarker:
 *
 * All fields are privates and should not be used directly.
 * Use its public methods instead.
 **/


#include "adg-marker.h"
#include "adg-marker-private.h"
#include "adg-intl.h"

#define PARENT_OBJECT_CLASS  ((GObjectClass *) adg_marker_parent_class)


enum {
    PROP_0,
    PROP_PATH,
    PROP_N_SEGMENT,
    PROP_POS,
    PROP_SIZE,
    PROP_MODEL
};


static void             dispose                 (GObject        *object);
static void             get_property            (GObject        *object,
                                                 guint           prop_id,
                                                 GValue         *value,
                                                 GParamSpec     *pspec);
static void             set_property            (GObject        *object,
                                                 guint           prop_id,
                                                 const GValue   *value,
                                                 GParamSpec     *pspec);
static gboolean         invalidate              (AdgEntity      *entity);
static gboolean         set_path                (AdgMarker      *marker,
                                                 AdgPath        *path);
static void             unset_path              (AdgMarker      *marker);
static gboolean         set_n_segment           (AdgMarker      *marker,
                                                 gint            n_segment);
static gboolean         set_pos                 (AdgMarker      *marker,
                                                 gdouble         pos);
static gboolean         set_size                (AdgMarker      *marker,
                                                 gdouble         size);
static gboolean         set_model               (AdgMarker      *marker,
                                                 AdgModel       *model);
static AdgModel *       create_model            (AdgMarker      *marker);


G_DEFINE_ABSTRACT_TYPE(AdgMarker, adg_marker, ADG_TYPE_ENTITY);


static void
adg_marker_class_init(AdgMarkerClass *klass)
{
    GObjectClass *gobject_class;
    AdgEntityClass *entity_class;
    GParamSpec *param;

    gobject_class = (GObjectClass *) klass;
    entity_class = (AdgEntityClass *) klass;

    g_type_class_add_private(klass, sizeof(AdgMarkerPrivate));

    gobject_class->dispose = dispose;
    gobject_class->set_property = set_property;
    gobject_class->get_property = get_property;

    entity_class->invalidate = invalidate;

    klass->create_model = create_model;

    param = g_param_spec_object("path",
                                P_("Path"),
                                P_("The subject path for this marker"),
                                ADG_TYPE_PATH,
                                G_PARAM_CONSTRUCT|G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_PATH, param);

    param = g_param_spec_uint("n-segment",
                              P_("Segment Index"),
                              P_("The segment of path where this marker should be applied (where 0 means undefined segment, 1 the first segment and so on)"),
                              0, G_MAXUINT, 0,
                              G_PARAM_CONSTRUCT|G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_N_SEGMENT, param);

    param = g_param_spec_double("pos",
                                P_("Position"),
                                P_("The position ratio inside the segment where to put the marker (0 means the start point while 1 means the end point)"),
                                0, 1, 0,
                                G_PARAM_CONSTRUCT|G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_POS, param);

    param = g_param_spec_double("size",
                                P_("Marker Size"),
                                P_("The size (in global space) of the marker"),
                                0, G_MAXDOUBLE, 12,
                                G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_SIZE, param);

    param = g_param_spec_object("model",
                                P_("Model"),
                                P_("A general purpose model usable by the marker implementations"),
                                ADG_TYPE_MODEL,
                                G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_MODEL, param);
}

static void
adg_marker_init(AdgMarker *marker)
{
    AdgMarkerPrivate *data = G_TYPE_INSTANCE_GET_PRIVATE(marker,
                                                         ADG_TYPE_MARKER,
                                                         AdgMarkerPrivate);
    data->path = NULL;
    data->n_segment = 0;
    data->backup_segment = NULL;
    memset(&data->segment, 0, sizeof(data->segment));
    data->pos = 0;
    data->size = 10;
    data->model = NULL;

    marker->data = data;
}

static void
dispose(GObject *object)
{
    AdgMarker *marker = (AdgMarker *) object;

    adg_marker_set_model(marker, NULL);
    adg_marker_set_path(marker, NULL);

    if (PARENT_OBJECT_CLASS->dispose != NULL)
        PARENT_OBJECT_CLASS->dispose(object);
}


static void
get_property(GObject *object,
             guint prop_id, GValue *value, GParamSpec *pspec)
{
    AdgMarkerPrivate *data = ((AdgMarker *) object)->data;

    switch (prop_id) {
    case PROP_PATH:
        g_value_set_object(value, data->path);
        break;
    case PROP_N_SEGMENT:
        g_value_set_uint(value, data->n_segment);
        break;
    case PROP_POS:
        g_value_set_double(value, data->pos);
        break;
    case PROP_SIZE:
        g_value_set_double(value, data->size);
        break;
    case PROP_MODEL:
        g_value_set_object(value, data->model);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void
set_property(GObject *object,
             guint prop_id, const GValue *value, GParamSpec *pspec)
{
    AdgMarker *marker = (AdgMarker *) object;

    switch (prop_id) {
    case PROP_PATH:
        set_path(marker, g_value_get_object(value));
        break;
    case PROP_N_SEGMENT:
        set_n_segment(marker, g_value_get_uint(value));
        break;
    case PROP_POS:
        set_pos(marker, g_value_get_double(value));
        break;
    case PROP_SIZE:
        set_size(marker, g_value_get_double(value));
        break;
    case PROP_MODEL:
        set_model(marker, g_value_get_object(value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}


/**
 * adg_marker_get_path:
 * @marker: an #AdgMarker
 *
 * Gets the path where this marker should be applied.
 *
 * Returns: the path owned by @marker or %NULL on errors
 **/
AdgPath *
adg_marker_get_path(AdgMarker *marker)
{
    AdgMarkerPrivate *data;

    g_return_val_if_fail(ADG_IS_MARKER(marker), NULL);

    data = marker->data;

    return data->path;
}

/**
 * adg_marker_set_path:
 * @marker: an #AdgMarker
 * @path: a new #AdgPath
 *
 * Sets a new path where the marker should be applied. The weak reference
 * to the old path (if an old path was present) is dropped while a new
 * weak reference is added to @path. If @path is destroyed, the weak
 * reference callback will automatically unset #AdgMarker:path and will
 * set #AdgMarker:n-segment to %0.
 *
 * After setting a new path, the #AdgMarker:n-segment property is
 * reset to %1. This means the first segment of the path is always
 * selected by default.
 **/
void
adg_marker_set_path(AdgMarker *marker, AdgPath *path)
{
    g_return_if_fail(ADG_IS_MARKER(marker));

    if (set_path(marker, path))
        g_object_notify((GObject *) marker, "path");
}

/**
 * adg_marker_get_n_segment:
 * @marker: an #AdgMarker
 *
 * Returns the segment of the associated path where this marker
 * should be applied, where %1 is the first segment.
 *
 * Returns: an index greather than %0 on success or %0 on errors
 **/
gint
adg_marker_get_n_segment(AdgMarker *marker)
{
    AdgMarkerPrivate *data;

    g_return_val_if_fail(ADG_IS_MARKER(marker), 0);

    data = marker->data;

    return data->n_segment;
}

/**
 * adg_marker_set_n_segment:
 * @marker: an #AdgMarker
 * @n_segment: a new segment index
 *
 * Sets a new segment to use. @n_segment is expected to be greather than
 * %0 and to not exceed the number of segments in the underlying path.
 * By convention, %1 is the first segment.
 **/
void
adg_marker_set_n_segment(AdgMarker *marker, gint n_segment)
{
    g_return_if_fail(ADG_IS_MARKER(marker));

    if (set_n_segment(marker, n_segment))
        g_object_notify((GObject *) marker, "n-segment");
}

/**
 * adg_marker_get_backup_segment:
 * @marker: an #AdgMarker
 *
 * <note><para>
 * This function is only useful in marker implementations.
 * </para></note>
 *
 * Gets the original segment where the marker has been applied.
 * Applying a marker could modify the underlying path, usually
 * by trimming the original segment of a #AdgMarker:size dependent
 * length from the end. The marker instance holds a copy of the
 * original segment, got using adg_segment_deep_dup(), to be used
 * in recomputation (when the marker changes size, for instance).
 *
 * When the subject segment is changed (either by changing
 * #AdgMarker:path or #AdgMarker:n-segment) the original segment
 * is restored.
 *
 * Returns: the original segment or %NULL on errors
 **/
const AdgSegment *
adg_marker_get_backup_segment(AdgMarker *marker)
{
    AdgMarkerPrivate *data;

    g_return_val_if_fail(ADG_IS_MARKER(marker), NULL);

    data = marker->data;

    return data->backup_segment;
}

/**
 * adg_marker_get_segment:
 * @marker: an #AdgMarker
 *
 * <note><para>
 * This function is only useful in marker implementations.
 * </para></note>
 *
 * Gets the segment where the marker will be applied. This segment
 * is eventually a modified version of the backup segment, after
 * having applied the marker.
 *
 * Returns: the segment or %NULL on errors
 **/
AdgSegment *
adg_marker_get_segment(AdgMarker *marker)
{
    AdgMarkerPrivate *data;

    g_return_val_if_fail(ADG_IS_MARKER(marker), NULL);

    data = marker->data;

    return &data->segment;
}

/**
 * adg_marker_get_pos:
 * @marker: an #AdgMarker
 *
 * Gets the current position of @marker. The returned value is a ratio
 * position referred to the segment associated to @marker: %0 means the
 * start point and %1 means the end point of the segment.
 *
 * Returns: the marker position
 **/
gdouble
adg_marker_get_pos(AdgMarker *marker)
{
    AdgMarkerPrivate *data;

    g_return_val_if_fail(ADG_IS_MARKER(marker), 0);

    data = marker->data;

    return data->pos;
}

/**
 * adg_marker_set_pos:
 * @marker: an #AdgMarker
 * @pos: the new pos
 *
 * Sets a new position on @marker. Check out adg_marker_get_pos() for
 * details on what @pos represents.
 **/
void
adg_marker_set_pos(AdgMarker *marker, gdouble pos)
{
    g_return_if_fail(ADG_IS_MARKER(marker));

    if (set_pos(marker, pos))
        g_object_notify((GObject *) marker, "pos");
}

/**
 * adg_marker_get_size:
 * @marker: an #AdgMarker
 *
 * Gets the current size of @marker.
 *
 * Returns: the marker size, in global space
 **/
gdouble
adg_marker_get_size(AdgMarker *marker)
{
    AdgMarkerPrivate *data;

    g_return_val_if_fail(ADG_IS_MARKER(marker), 0);

    data = marker->data;

    return data->size;
}

/**
 * adg_marker_set_size:
 * @marker: an #AdgMarker
 * @size: the new size
 *
 * Sets a new size on @marker. The @size is an implementation-dependent
 * property: it has meaning only when used by an #AdgMarker derived type.
 **/
void
adg_marker_set_size(AdgMarker *marker, gdouble size)
{
    g_return_if_fail(ADG_IS_MARKER(marker));

    if (set_size(marker, size))
        g_object_notify((GObject *) marker, "size");
}

/**
 * adg_marker_model:
 * @marker: an #AdgMarker
 *
 * <note><para>
 * This function is only useful in marker implementations.
 * </para></note>
 *
 * Gets the model of @marker. If the model is not found, it is
 * automatically created by calling the create_model() virtual method.
 *
 * Returns: the current model owned by @marker or %NULL on errors
 **/
AdgModel *
adg_marker_model(AdgMarker *marker)
{
    AdgMarkerPrivate *data;

    g_return_val_if_fail(ADG_IS_MARKER(marker), NULL);

    data = marker->data;

    if (data->model == NULL) {
        /* Model not found: regenerate it */
        AdgMarkerClass *marker_class = ADG_MARKER_GET_CLASS(marker);

        adg_marker_set_model(marker, marker_class->create_model(marker));
    }

    return data->model;
}

/**
 * adg_marker_get_model:
 * @marker: an #AdgMarker
 *
 * <note><para>
 * This function is only useful in marker implementations.
 * </para></note>
 *
 * Gets the current model of @marker. This is an accessor method:
 * if you need to get the model for rendering, use adg_marker_model()
 * instead.
 *
 * Returns: the cached model owned by @marker or %NULL on errors
 **/
AdgModel *
adg_marker_get_model(AdgMarker *marker)
{
    AdgMarkerPrivate *data;

    g_return_val_if_fail(ADG_IS_MARKER(marker), NULL);

    data = marker->data;

    return data->model;
}

/**
 * adg_marker_set_model:
 * @marker: an #AdgMarker
 * @model: a new #AdgModel
 *
 * <note><para>
 * This function is only useful in marker implementations.
 * </para></note>
 *
 * Sets a new model for @marker. The reference to the old model (if an
 * old model was present) is dropped while a new reference is added to
 * @model.
 **/
void
adg_marker_set_model(AdgMarker *marker, AdgModel *model)
{
    g_return_if_fail(ADG_IS_MARKER(marker));

    if (set_model(marker, model))
        g_object_notify((GObject *) marker, "model");
}


static gboolean
invalidate(AdgEntity *entity)
{
    AdgMarker *marker = (AdgMarker *) entity;

    adg_marker_set_model(marker, NULL);

    return TRUE;
}


static gboolean
set_path(AdgMarker *marker, AdgPath *path)
{
    AdgMarkerPrivate *data;
    AdgEntity *entity;

    data = marker->data;

    if (path == data->path)
        return FALSE;

    entity = (AdgEntity *) marker;

    if (data->path != NULL) {
        /* Restore the original segment in the old path */
        set_n_segment(marker, 0);

        g_object_weak_unref((GObject *) data->path,
                            (GWeakNotify) unset_path, marker);
        adg_model_remove_dependency((AdgModel *) data->path, entity);
    }

    data->path = path;

    if (data->path != NULL) {
        g_object_weak_ref((GObject *) data->path,
                          (GWeakNotify) unset_path, marker);
        adg_model_add_dependency((AdgModel *) data->path, entity);

        /* Set the first segment by default */
        set_n_segment(marker, 1);
    }

    return TRUE;
}

static void
unset_path(AdgMarker *marker)
{
    AdgMarkerPrivate *data = marker->data;

    if (data->path != NULL) {
        data->path = NULL;
        set_n_segment(marker, 0);
    }
}

static gboolean
set_n_segment(AdgMarker *marker, gint n_segment)
{
    AdgMarkerPrivate *data = marker->data;

    if (n_segment == data->n_segment)
        return FALSE;

    if (data->n_segment > 0) {
        g_assert(data->backup_segment != NULL);

        /* Restore the original segment */
        adg_segment_deep_copy(&data->segment, data->backup_segment);
    }

    if (data->backup_segment != NULL) {
        g_free(data->backup_segment);
        data->backup_segment = NULL;
    }

    if (n_segment > 0) {
        data->n_segment = 0;
        g_return_val_if_fail(data->path != NULL, FALSE);

        if (!adg_path_get_segment(data->path,
                                  &data->segment, data->n_segment))
            return FALSE;

        /* Backup the segment */
        data->backup_segment = adg_segment_deep_dup(&data->segment);
    }

    data->n_segment = n_segment;

    return TRUE;
}

static gboolean
set_pos(AdgMarker *marker, gdouble pos)
{
    AdgMarkerPrivate *data = marker->data;

    if (pos == data->pos)
        return FALSE;

    data->pos = pos;

    return TRUE;
}

static gboolean
set_size(AdgMarker *marker, gdouble size)
{
    AdgMarkerPrivate *data = marker->data;

    if (size == data->size)
        return FALSE;

    data->size = size;

    return TRUE;
}

static gboolean
set_model(AdgMarker *marker, AdgModel *model)
{
    AdgMarkerPrivate *data = marker->data;

    if (model == data->model)
        return FALSE;

    if (data->model != NULL)
        g_object_unref((GObject *) data->model);

    data->model = model;

    if (data->model != NULL)
        g_object_ref((GObject *) data->model);

    return TRUE;
}

static AdgModel *
create_model(AdgMarker *marker)
{
    g_warning("%s: `create_model' method not implemented for type `%s'",
              G_STRLOC, g_type_name(G_OBJECT_TYPE(marker)));
    return NULL;
}
