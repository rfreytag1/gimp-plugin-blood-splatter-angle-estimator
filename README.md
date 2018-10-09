# GIMP Blood Splatter Angle Estimator Plugin

## Introduction

This Plug-In implements an algorithm which can estimate the direction of a blood splash from its shape and gradient,
which could be helpful in forensic analyses of crime scenes involving violence.

Initially implemented as a Python-script, the GIMP Plug-In offers improved accessibility
and an improved workflow allowing for a quicker transition from preparing the input image to
actually executing the algorithm on it, eliminating the need to adjust the input image in a
separate program and saving a copy to be processed.

## Getting Started

### Requirements

To compile this GIMP Plugin you will need the following libraries:

* GTK2 >= 2.24.0
* OpenCV >= 3.4.0
* libgimp & libgimpui >= 2.9.0 (included with GIMP)

As a build system `CMake` is used.

Furthermore a compiler supporting `C++17` is required. Using `gcc` is recommended.

### Compilation

First clone the project

    $ git clone https://github.com/rfreytag1/gimp-plugin-blood-splatter-angle-estimator.git
    $ cd gimp-plugin-blood-splatter-angle-estimator

Then run CMake to configure the build tree and compile the plugin

    $ mkdir -p build && cd build/
    $ cmake ../ -DCMAKE_BUILD_TYPE=Release
    $ cmake --build ./

### Installation
To install the plugin you can either use CMake which will do a system-wide installation

    $ sudo cmake --build ./ --target install

or use GIMP's own tool for a user installation

    $ gimptool-2.0 --install-bin  plug-in-blood-splash-analyzer

## Usage

After installing the Plug-In it should be available in `Filters > Misc` as `Blood Splash...`.
Clicking on this entry opens a dialog with various options concerning the algorithm and the appearance of the result.

When you are finished adjusting the settings clicking `Ok` will run the algorithm either on the whole image
or a selection. In the status bar on the bottom of GIMP the message "Blood Splatter Angle Estimation..."
will appear indicating it's working.

A new layer with the results will appear after the algorithm has finished. On this layer
you will find arrows encapsulated by ellipses. The ellipses indicate the detected blood splash impact
area, while the arrows indicate the angle and, provided there is a gradient, a direction.

The results might improve by preparing the input image by means of adjusting contrast, brightness and so on.

## Acknowledgements

Thanks to Steffen Grunert for providing the algorithm and the original implementation!