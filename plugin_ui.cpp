//
// Created by rfreytag on 02.10.18.
//

#include <string>
#include <cstdlib>

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

#include <opencv2/opencv.hpp>

#include "plugin_ui.h"
#include "main.h"

/*
 * function that builds the plugin dialog with preview and parameter inputs
 */
gboolean blood_splash_dialog(GimpDrawable *drawable, gint32 image) {
    GtkWidget *dialog;
    GtkWidget *main_vbox;
    GtkWidget *params_vbox;
    GtkWidget *frame;
    GtkWidget *alignment;
    GtkWidget *frame_label;
    GtkWidget *ellipse_color_picker_hbox;
    GtkWidget *ellipse_color_picker_label;
    GtkWidget *ellipse_color_picker_button;
    GtkWidget *dirind_color_picker_hbox;
    GtkWidget *dirind_color_picker_label;
    GtkWidget *dirind_color_picker_button;
    //GtkWidget *preview;

    GtkWidget *white_ratio_hbox;
    GtkWidget *white_ratio_label;
    GtkWidget *white_ratio_scale;


    gboolean run;

    gimp_ui_init("bloodsplash", FALSE);

    dialog = gimp_dialog_new("Blood Splash Estimator", "bloodsplash",
                             nullptr, (GtkDialogFlags) 0,
                             gimp_standard_help_func, "blood-splash-analyzer",
                             GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                             GTK_STOCK_OK, GTK_RESPONSE_OK,
                             nullptr);

    main_vbox = gtk_vbox_new(FALSE, 6);
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), main_vbox);
    gtk_widget_show(main_vbox);

    //preview = gimp_drawable_preview_new_from_drawable_id(drawable->drawable_id);
    //gtk_box_pack_start(GTK_BOX (main_vbox), preview, TRUE, TRUE, 0);
    //gtk_widget_show(preview);

    frame = gtk_frame_new(nullptr);
    gtk_widget_show(frame);
    gtk_box_pack_start(GTK_BOX(main_vbox), frame, TRUE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(frame), 6);

    alignment = gtk_alignment_new(0.5, 0.5, 1, 1);
    gtk_widget_show(alignment);
    gtk_container_add(GTK_CONTAINER(frame), alignment);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 6, 6, 6, 6);

    params_vbox = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(params_vbox);
    gtk_container_add(GTK_CONTAINER(alignment), params_vbox);

    frame_label = gtk_label_new("<b>Modify Parameters...</b>");
    gtk_widget_show(frame_label);
    gtk_frame_set_label_widget(GTK_FRAME(frame), frame_label);
    gtk_label_set_use_markup(GTK_LABEL(frame_label), TRUE);

    // White/Red Ratio
    white_ratio_hbox = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(white_ratio_hbox);
    gtk_container_add(GTK_CONTAINER(params_vbox), white_ratio_hbox);
    white_ratio_label = gtk_label_new("White Ratio:");
    gtk_box_pack_start(GTK_BOX(white_ratio_hbox), white_ratio_label, FALSE, FALSE, 5);
    gtk_widget_show(white_ratio_label);

    white_ratio_scale = gtk_hscale_new_with_range(0.0, 100.0, 1.0);
    gtk_range_set_value(GTK_RANGE(white_ratio_scale),
                        pparams.white_ratio);
    gtk_box_pack_end(GTK_BOX(white_ratio_hbox), white_ratio_scale, TRUE, TRUE, 5);
    gtk_widget_show(white_ratio_scale);

    g_signal_connect(gtk_range_get_adjustment(GTK_RANGE(white_ratio_scale)), "value_changed",
                     G_CALLBACK(gimp_uint_adjustment_update),
                     &pparams.white_ratio);

    // ellipse color picker
    ellipse_color_picker_hbox = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(ellipse_color_picker_hbox);
    gtk_container_add(GTK_CONTAINER(params_vbox), ellipse_color_picker_hbox);

    ellipse_color_picker_label = gtk_label_new("Result Ellipse Color:");
    gtk_box_pack_start(GTK_BOX(ellipse_color_picker_hbox), ellipse_color_picker_label, FALSE, FALSE, 5);
    gtk_widget_show(ellipse_color_picker_label);

    ellipse_color_picker_button = gimp_color_button_new("Select Ellipse Color", 20, 15, &pparams.ellipse_color,
                                                        GIMP_COLOR_AREA_SMALL_CHECKS);
    gtk_box_pack_start(GTK_BOX(ellipse_color_picker_hbox), ellipse_color_picker_button, FALSE, FALSE, 5);
    gtk_widget_show(ellipse_color_picker_button);

    g_signal_connect(GIMP_COLOR_BUTTON(ellipse_color_picker_button), "color-changed",
                     G_CALLBACK(colorpicker_color_changed), &pparams.ellipse_color);

    // direction indicator color picker
    dirind_color_picker_hbox = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(dirind_color_picker_hbox);
    gtk_container_add(GTK_CONTAINER(params_vbox), dirind_color_picker_hbox);

    dirind_color_picker_label = gtk_label_new("Result Direction Indicator Color:");
    gtk_box_pack_start(GTK_BOX(dirind_color_picker_hbox), dirind_color_picker_label, FALSE, FALSE, 5);
    gtk_widget_show(dirind_color_picker_label);

    dirind_color_picker_button = gimp_color_button_new("Select Direction Indicator Color", 20, 15,
                                                       &pparams.dirind_color, GIMP_COLOR_AREA_SMALL_CHECKS);
    gtk_box_pack_start(GTK_BOX(dirind_color_picker_hbox), dirind_color_picker_button, FALSE, FALSE, 5);
    gtk_widget_show(dirind_color_picker_button);

    g_signal_connect(GIMP_COLOR_BUTTON(dirind_color_picker_button), "color-changed",
                     G_CALLBACK(colorpicker_color_changed), &pparams.dirind_color);

    gtk_widget_show(dialog);

    run = (gimp_dialog_run(GIMP_DIALOG(dialog)) == GTK_RESPONSE_OK);

    gtk_widget_destroy(dialog);

    return run;
}

void colorpicker_color_changed(GtkWidget *widget, gpointer callback_data) {
    gimp_color_button_get_color(GIMP_COLOR_BUTTON(widget), (GimpRGB *) callback_data);
}