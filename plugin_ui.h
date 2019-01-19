//
// Created by rfreytag on 02.10.18.
//

#ifndef TEST01_PLUGIN_UI_H
#define TEST01_PLUGIN_UI_H

#include <string>
#include <cstdlib>

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

gboolean blood_splash_dialog(GimpDrawable *drawable, gint32 image);

void colorpicker_color_changed(GtkWidget *widget, gpointer callback_data);

#endif //TEST01_PLUGIN_UI_H
