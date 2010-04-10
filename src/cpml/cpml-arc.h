/* CPML - Cairo Path Manipulation Library
 * Copyright (C) 2008,2009,2010  Nicola Fontana <ntd at entidi.it>
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


#if !defined(__CPML_H__)
#error "Only <cpml/cpml.h> can be included directly."
#endif


#ifndef __CPML_ARC_H__
#define __CPML_ARC_H__


CAIRO_BEGIN_DECLS

#ifdef CAIRO_PATH_ARC_TO
#define CPML_ARC        CAIRO_PATH_ARC_TO
#else
#define CPML_ARC        100
#endif


cairo_bool_t
        cpml_arc_info                   (const CpmlPrimitive    *arc,
                                         CpmlPair               *center,
                                         double                 *r,
                                         double                 *start,
                                         double                 *end);
void    cpml_arc_to_cairo               (const CpmlPrimitive    *arc,
                                         cairo_t                *cr);
void    cpml_arc_to_curves              (const CpmlPrimitive    *arc,
                                         CpmlSegment            *segment,
                                         size_t                  n_curves);

CAIRO_END_DECLS


#endif /* __CPML_ARC_H__ */