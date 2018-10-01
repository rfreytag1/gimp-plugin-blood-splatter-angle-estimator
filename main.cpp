//
// Created by rfreytag on 12.06.18.
//

// TODO: cleanup a bit and move things in own modules, e.g. UI etc.
#include <string>
#include <cstdlib>

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

#include <opencv2/opencv.hpp>

#include "BloodSplatterAngleEstimator/BloodSplatterAngleEstimator.h"
#include "BloodSplatterAngleEstimator/Parameters.h"

using namespace HSMW::Forensics::BloodSplatterAngleEstimators;

static void init();

static void query();

static void run(const gchar *name, gint nparams, const GimpParam *param, gint *nreturn_vals, GimpParam **return_vals);

static void estimate_splash(GimpDrawable *drawable, GimpPreview *preview, gint32 image);

static gboolean blood_splash_dialog(GimpDrawable *drawable, gint32 image);

void add_layer (gint32 image, gint32 parent, cv::Mat src, std::string const& name,
                GimpLayerModeEffects mode, bool alpha = false);

struct PluginParams
{
    guint white_ratio;
};

PluginParams pparams = {
        100
};

GimpPlugInInfo PLUG_IN_INFO = {
        NULL, //init
        NULL, //quit
        query,
        run
};

MAIN()

static void init() {

}

static void query() {
    static GimpParamDef args[] = {
            {
                    GIMP_PDB_INT32,
                    (gchar*)"run-mode",
                    (gchar*)"Run mode"
            },
            {
                    GIMP_PDB_IMAGE,
                    (gchar*)"image",
                    (gchar*)"Input image"
            },
            {
                    GIMP_PDB_DRAWABLE,
                    (gchar*)"drawable",
                    (gchar*)"Input drawable"
            },
            {
                    GIMP_PDB_INT8,
                    (gchar*)"white-ratio",
                    (gchar*)"White/Red-Ratio"
            }
    };

    gimp_install_procedure(
            "plug-in-blood-splash-analyzer",
            "Blood Splash",
            "Estimates the splash direction of blood stains",
            "Steffen Grunert, Roy Freytag",
            "Copyright  Steffen Grunert, Roy Freytag",
            "2018",
            "_Blood Splash...",
            "RGB*, GRAY*",
            GIMP_PLUGIN,
            G_N_ELEMENTS (args), 0,
            args, nullptr);

    gimp_plugin_menu_register("plug-in-blood-splash-analyzer",
                              "<Image>/Filters/Misc");
}

static void run(const gchar *name, gint nparams, const GimpParam *param, gint *nreturn_vals, GimpParam **return_vals) {
    static GimpParam values[1];
    GimpPDBStatusType status = GIMP_PDB_SUCCESS;
    GimpRunMode run_mode;
    GimpDrawable *drawable;
    gint32 image;

    /* Setting mandatory output values */
    *nreturn_vals = 1;
    *return_vals = values;

    values[0].type = GIMP_PDB_STATUS;
    values[0].data.d_status = status;

    /* Getting run_mode - we won't display a dialog if
     * we are in NONINTERACTIVE mode */
    run_mode = (GimpRunMode)param[0].data.d_int32;

    drawable = gimp_drawable_get(param[2].data.d_drawable);
    image = param[1].data.d_image;
    if (run_mode == GIMP_RUN_INTERACTIVE) {
        gimp_get_data ("plug-in-blood-splash-estimator", &pparams);
        HSMW::Forensics::BloodSplatterAngleEstimators::Parameters::WHITE_RATIO = pparams.white_ratio;
        if(blood_splash_dialog(drawable, image) == TRUE) {
            estimate_splash(drawable, nullptr, image);
            gimp_set_data("plug-in-blood-splash-estimator", &pparams, sizeof(pparams));
        }
    } else if (run_mode == GIMP_RUN_WITH_LAST_VALS) {
        HSMW::Forensics::BloodSplatterAngleEstimators::Parameters::WHITE_RATIO = pparams.white_ratio;
        estimate_splash(drawable, nullptr, image);
        gimp_get_data ("plug-in-blood-splash-estimator", &pparams);
    } else if (run_mode == GIMP_RUN_NONINTERACTIVE) {
        if (nparams != 4) {
            status = GIMP_PDB_CALLING_ERROR;
        } else {
            HSMW::Forensics::BloodSplatterAngleEstimators::Parameters::WHITE_RATIO = values[3].data.d_int8;
            estimate_splash(drawable, nullptr, image);
        }
    }
    // redraw
    gimp_displays_flush ();
    gimp_drawable_detach (drawable);
}

static void estimate_splash(GimpDrawable *drawable, GimpPreview *preview, gint32 image) {
    int width, height, channels;
    GimpPixelRgn rgn_in;
    guchar *line;
    gint32 layer;
    gint x1 = 0, y1 = 0, x2 = 0, y2 = 0;
    
    // selection support

    gimp_drawable_mask_bounds(drawable->drawable_id, &x1, &y1, &x2, &y2);
    width = x2 - x1;
    height = y2 - y1;
    /*
    width = gimp_drawable_width (drawable->drawable_id);
    height = gimp_drawable_height (drawable->drawable_id);
    */
    channels = gimp_drawable_bpp (drawable->drawable_id);

    // pixel access
    gimp_pixel_rgn_init (&rgn_in, drawable, x1, y1, width, height, FALSE, FALSE);
    gimp_tile_cache_ntiles (drawable->width / gimp_tile_width () + 1);

    // create buffer to store gimp image data in OpenCV format
    cv::Mat imgBuffer = cv::Mat::zeros(height, width, CV_8UC3);
    // output buffer to be drawn on, this will contain data for the new image layer
    cv::Mat imgOutBuffer = cv::Mat::zeros(height, width, CV_8UC4);


    gimp_progress_init ("Blood Splatter Angle Estimation...");

    // read the image into memory one pixel at a time :/
    // TODO: look into gegl buffers, should be much faster
    line = (guchar *) malloc (channels * width * sizeof (guchar));
    for(int i = 0; i < height; i++)
    {
        gimp_pixel_rgn_get_row (&rgn_in, line, x1, y1 + i, width);

        //something like this would be nicer:
        //imgBuffer.row(i) = cv::Mat(line);
        for(int x = 0; x < width; x++)
        {
            imgBuffer.at<cv::Vec3b>(i, x) = cv::Vec3b(((channels >= 3) ? line[x * channels + 2] : (uchar)0x00),
                                                    ((channels >= 2) ? line[x * channels + 1] : (uchar)0x00),
                                                    ((channels >= 1) ? line[x * channels + 0] : (uchar)0x00));
        }
    }

    // prepare imgOutBuffer to be drawn on
    // TODO: maybe don't copy image and keep it all black and replace black with alpha instead?
    // imgBuffer.copyTo(imgOutBuffer);

    estimateBloodSplatterAngle(imgBuffer, imgOutBuffer);

    // GIMP images are in RGB but OpenCV in BGR -> BGR2RGB
    cv::cvtColor(imgOutBuffer, imgOutBuffer, cv::COLOR_BGRA2RGBA);
    // black -> transparent
    /*
    imgOutBuffer.forEach<cv::Vec4b>([](cv::Vec4b &pixel, const int *position) -> void {
        double max = 0.0;
        cv::minMaxLoc(pixel, nullptr, &max);
        if(max == 0.0) {
            pixel[3] = 0;
        } else {
            pixel[3] = 255;
        }
    });
    */
    add_layer(image, drawable->drawable_id, imgOutBuffer, "Test", GIMP_NORMAL_MODE, true);
    gimp_progress_end();
}

/*
 * function that builds the plugin dialog with preview and parameter inputs
 */
static gboolean blood_splash_dialog(GimpDrawable *drawable, gint32 image) {
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
                             NULL, (GtkDialogFlags)0,
                             gimp_standard_help_func, "blood-splash-analyzer",
                             GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                             GTK_STOCK_OK, GTK_RESPONSE_OK,

                             NULL);

    main_vbox = gtk_vbox_new(FALSE, 6);
    gtk_container_add(GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), main_vbox);
    gtk_widget_show(main_vbox);

    //preview = gimp_drawable_preview_new_from_drawable_id(drawable->drawable_id);
    //gtk_box_pack_start(GTK_BOX (main_vbox), preview, TRUE, TRUE, 0);
    //gtk_widget_show(preview);

    frame = gtk_frame_new(NULL);
    gtk_widget_show(frame);
    gtk_box_pack_start(GTK_BOX (main_vbox), frame, TRUE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER (frame), 6);

    alignment = gtk_alignment_new(0.5, 0.5, 1, 1);
    gtk_widget_show(alignment);
    gtk_container_add(GTK_CONTAINER (frame), alignment);
    gtk_alignment_set_padding(GTK_ALIGNMENT (alignment), 6, 6, 6, 6);

    params_vbox = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(params_vbox);
    gtk_container_add(GTK_CONTAINER (alignment), params_vbox);

    // TODO: actually add any parameters to modify...
    frame_label = gtk_label_new("<b>Modify Parameters...</b>");
    gtk_widget_show(frame_label);
    gtk_frame_set_label_widget(GTK_FRAME (frame), frame_label);
    gtk_label_set_use_markup(GTK_LABEL (frame_label), TRUE);

    white_ratio_hbox = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(white_ratio_hbox);
    gtk_container_add(GTK_CONTAINER(params_vbox), white_ratio_hbox);
    white_ratio_label = gtk_label_new("White Ratio:");
    gtk_box_pack_start(GTK_BOX(white_ratio_hbox), white_ratio_label, FALSE, FALSE, 5);
    gtk_widget_show(white_ratio_label);

    white_ratio_scale = gtk_hscale_new_with_range(0.0, 100.0 ,1.0);
    gtk_range_set_value(GTK_RANGE(white_ratio_scale), HSMW::Forensics::BloodSplatterAngleEstimators::Parameters::WHITE_RATIO);
    gtk_box_pack_end(GTK_BOX(white_ratio_hbox), white_ratio_scale, TRUE, TRUE, 5);
    gtk_widget_show(white_ratio_scale);

    g_signal_connect (gtk_range_get_adjustment(GTK_RANGE(white_ratio_scale)), "value_changed",
                      G_CALLBACK (gimp_uint_adjustment_update),
                      &pparams.white_ratio);

    // ellipse color picker
    // TODO: set default color; connect color signal to actually change colors
    ellipse_color_picker_hbox = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(ellipse_color_picker_hbox);
    gtk_container_add(GTK_CONTAINER(params_vbox), ellipse_color_picker_hbox);

    ellipse_color_picker_label = gtk_label_new("Result Ellipse Color:");
    gtk_box_pack_start(GTK_BOX(ellipse_color_picker_hbox), ellipse_color_picker_label, FALSE, FALSE, 5);
    gtk_widget_show(ellipse_color_picker_label);

    ellipse_color_picker_button = gtk_color_button_new();
    gtk_box_pack_start(GTK_BOX(ellipse_color_picker_hbox), ellipse_color_picker_button, FALSE, FALSE, 5);
    gtk_widget_show(ellipse_color_picker_button);

    // direction indicator color picker
    // TODO: set default color; connect color signal to actually change colors
    dirind_color_picker_hbox = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(dirind_color_picker_hbox);
    gtk_container_add(GTK_CONTAINER(params_vbox), dirind_color_picker_hbox);

    dirind_color_picker_label = gtk_label_new("Result Direction Indicator Color:");
    gtk_box_pack_start(GTK_BOX(dirind_color_picker_hbox), dirind_color_picker_label, FALSE, FALSE, 5);
    gtk_widget_show(dirind_color_picker_label);

    dirind_color_picker_button = gtk_color_button_new();
    gtk_box_pack_start(GTK_BOX(dirind_color_picker_hbox), dirind_color_picker_button, FALSE, FALSE, 5);
    gtk_widget_show(dirind_color_picker_button);

    // White-Ratio

    // TODO: draw preview
    //g_signal_connect_swapped (preview, "invalidated",
    //                          G_CALLBACK(estimate_splash),
    //                          drawable);

    //estimate_splash(drawable, GIMP_PREVIEW(preview), image);



    gtk_widget_show(dialog);

    run = (gimp_dialog_run(GIMP_DIALOG (dialog)) == GTK_RESPONSE_OK);

    gtk_widget_destroy(dialog);

    return run;
}

/*
 * simple function to add new layers.
 * Thanks Marco Rossini!
 * (taken straight from https://github.com/mrossini-ethz/gimp-wavelet-decompose with minor adaptations)
 */
void add_layer (gint32 image, gint32 parent, cv::Mat src, std::string const& name,
           GimpLayerModeEffects mode, bool alpha)
{
    gint offx, offy;
    gint32 layer;
    int x, y, width, height, channels, c;
    GimpPixelRgn rgn;
    GimpDrawable *drawable;
    guchar *line;
    
    gint x1 = 0, y1 = 0, x2 = 0, y2 = 0;
    
    // selection support

    gimp_drawable_mask_bounds(parent, &x1, &y1, &x2, &y2);
    width = x2 - x1;
    height = y2 - y1;

    //width = gimp_drawable_width (parent);
    //height = gimp_drawable_height (parent);
    //channels = gimp_drawable_bpp (parent);
    channels = src.channels();

    if(alpha && channels == 4){
        layer = gimp_layer_new (image, name.c_str(), width, height,
                                gimp_drawable_type_with_alpha (parent), 100.0, mode);
        //++channels;
    } else {
        layer = gimp_layer_new (image, name.c_str(), width, height,
                                gimp_drawable_type (parent), 100.0, mode);
    }


    drawable = gimp_drawable_get (layer);
    gimp_image_insert_layer (image, layer, -1, -1);

    gimp_pixel_rgn_init (&rgn, drawable, 0, 0, width, height, TRUE, FALSE);


    line = new guchar[(width * height * channels)];

    for (y = 0; y < height; y++)
    {
        if (channels % 2 != 0)
        {
            /* no alpha channel */
            for (x = 0; x < width; x++)
            {
                for (c = 0; c < channels; c++)
                {
                    line[x * channels + c] = src.at<cv::Vec3b>(y, x)[c];
                }
            }
        }
        else
        {
            for (x = 0; x < width; x++)
            {
                for (c = 0; c < channels; c++)
                {
                    line[x * channels + c] = src.at<cv::Vec4b>(y, x)[c];
                }
                //line[x * channels + (channels - 1)] = 127;
            }
        }
        gimp_pixel_rgn_set_row (&rgn, line, 0, y, width);
    }
    delete[] line;

    //gimp_drawable_offsets (parent, &offx, &offy);
    //gimp_layer_translate (layer, offx, offy);
    gimp_layer_translate (layer, x1, y1);
    gimp_image_set_active_layer (image, parent);
    gimp_drawable_flush (drawable);
    gimp_drawable_update (layer, x1, y1, width, height);
}