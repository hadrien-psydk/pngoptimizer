///////////////////////////////////////////////////////////////////////////////
// This file is part of the PngOptimizerGtk application
// Copyright (C) Hadrien Nilsson - psydk.org
///////////////////////////////////////////////////////////////////////////////

#ifndef PO_GTK_STDAFX_H
#define PO_GTK_STDAFX_H

#include "../stdafx.h"

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib/gi18n.h>
#include <gdk/gdk.h>

// We build with -Wzero-as-null-pointer-constant
// This is for GTK macros we use in PngOptimizer

#undef NULL
#define NULL nullptr

// Unfortunately this macro use 0 instead of NULL
#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN    ((gchar*) NULL)

using namespace chustd;

#endif
