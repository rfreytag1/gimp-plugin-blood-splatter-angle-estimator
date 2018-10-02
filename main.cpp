//
// Created by rfreytag on 12.06.18.
//

// TODO: cleanup a bit and move things in own modules, e.g. UI etc.
#include <string>
#include <cstdlib>

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

#include <opencv2/opencv.hpp>

#include "main.h"

#include "plugin_ui.h"

#include "BloodSplatterAngleEstimator/BloodSplatterAngleEstimator.h"
#include "BloodSplatterAngleEstimator/Parameters.h"

using namespace HSMW::Forensics::BloodSplatterAngleEstimators;

static void init();

static void query();

static void run(const gchar *name, gint nparams, const GimpParam *param, gint *nreturn_vals, GimpParam **return_vals);

static void estimate_splash(GimpDrawable *drawable, GimpPreview *preview, gint32 image);

void add_layer (gint32 image, gint32 parent, cv::Mat src, std::string const& name,
                GimpLayerModeEffects mode, bool alpha = false);

cv::Scalar gimpColorToCVScalar(GimpRGB const& srcColor)
{
    return cv::Scalar(srcColor.b * 255, srcColor.g * 255, srcColor.r * 255, srcColor.a * 255);
}

PluginParams pparams = {
        100,
        {0.0, 0.0, 1.0, 1.0},
        {0.0, 1.0, 0.0, 1.0},
};

GimpPlugInInfo PLUG_IN_INFO = {
        nullptr, //init
        nullptr, //quit
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
            },
            {
                    GIMP_PDB_COLOR,
                    (gchar*)"ellipse-color",
                    (gchar*)"Color of Ellipses"
            },
            {
                    GIMP_PDB_COLOR,
                    (gchar*)"dirind-color",
                    (gchar*)"Color of Direction Indicators"
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
        if(blood_splash_dialog(drawable, image) == TRUE) {
            HSMW::Forensics::BloodSplatterAngleEstimators::Parameters::WHITE_RATIO = pparams.white_ratio;
            Parameters::ELLIPSE_PARAMETERS::COLOR = gimpColorToCVScalar(pparams.ellipse_color);
            Parameters::DIRECTION_INDICATOR_PARAMETERS::COLOR = gimpColorToCVScalar(pparams.dirind_color);
            estimate_splash(drawable, nullptr, image);
            gimp_set_data("plug-in-blood-splash-estimator", &pparams, sizeof(pparams));
        }
    } else if (run_mode == GIMP_RUN_WITH_LAST_VALS) {
        gimp_get_data ("plug-in-blood-splash-estimator", &pparams);
        HSMW::Forensics::BloodSplatterAngleEstimators::Parameters::WHITE_RATIO = pparams.white_ratio;
        Parameters::ELLIPSE_PARAMETERS::COLOR = gimpColorToCVScalar(pparams.ellipse_color);
        Parameters::DIRECTION_INDICATOR_PARAMETERS::COLOR = gimpColorToCVScalar(pparams.dirind_color);
        estimate_splash(drawable, nullptr, image);
    } else if (run_mode == GIMP_RUN_NONINTERACTIVE) {
        if (nparams != 4) {
            status = GIMP_PDB_CALLING_ERROR;
        } else {
            HSMW::Forensics::BloodSplatterAngleEstimators::Parameters::WHITE_RATIO = param[3].data.d_int8;
            Parameters::ELLIPSE_PARAMETERS::COLOR = gimpColorToCVScalar(param[4].data.d_color);
            Parameters::DIRECTION_INDICATOR_PARAMETERS::COLOR = gimpColorToCVScalar(param[5].data.d_color);
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