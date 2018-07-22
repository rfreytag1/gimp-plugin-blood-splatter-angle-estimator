cmake_minimum_required(VERSION 3.11)
project(test01)

set(CMAKE_CXX_STANDARD 17)

include_directories(
        /usr/include/gimp-2.0
        /usr/include/cairo
        /usr/include/gegl-0.4
        /usr/include/glib-2.0
        /usr/lib/glib-2.0/include
        /usr/include/babl-0.1
        /usr/include/gdk-pixbuf-2.0
        /usr/include/gtk-2.0
        /usr/lib/gtk-2.0/include
        /usr/include/pango-1.0
        /usr/include/fribidi
        /usr/include/pixman-1
        /usr/include/freetype2
        /usr/include/libpng16
        /usr/include/harfbuzz
        /usr/include/uuid
        /usr/include/gdk-pixbuf-2.0
        /usr/include/atk-1.0
)

add_executable(plug-in-blood-splash-analyzer main.cpp BloodSplatterAngleEstimator/BloodSplatterAngleEstimator.cpp BloodSplatterAngleEstimator/OptimaFinder.cpp BloodSplatterAngleEstimator/Helpers.cpp BloodSplatterAngleEstimator/Parameters.cpp)

target_link_libraries(plug-in-blood-splash-analyzer
        glib-2.0
        gimp-2.0
        gimpbase-2.0
        gimpmath-2.0
        gimpcolor-2.0
        gimpconfig-2.0
        gimpmodule-2.0
        gimpui-2.0
        gimpthumb-2.0
        gimpwidgets-2.0
        gdk-x11-2.0
        pangocairo-1.0
        pango-1.0
        fribidi
        cairo
        gdk_pixbuf-2.0
        gobject-2.0
        glib-2.0
        gtk-x11-2.0
        gdk-x11-2.0
        pangocairo-1.0
        atk-1.0
        cairo
        gdk_pixbuf-2.0
        gio-2.0
        pangoft2-1.0
        fontconfig
        freetype
        opencv_imgcodecs
        opencv_imgproc
        opencv_core
)