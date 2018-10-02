//
// Created by pfannkuchen on 02.10.18.
//

#ifndef TEST01_MAIN_H
#define TEST01_MAIN_H

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

struct PluginParams
{
    guint white_ratio;
    GimpRGB ellipse_color;
    GimpRGB dirind_color;
};

extern PluginParams pparams;

#endif //TEST01_MAIN_H
