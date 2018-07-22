//
// Created by rfreytag on 04.05.18.
//

#ifndef TEST02_PARAMETERS_H
#define TEST02_PARAMETERS_H

#include <opencv2/opencv.hpp>

namespace HSMW {
    namespace Forensics {
        namespace BloodSplatterAngleEstimators {
            namespace Parameters {
                extern int CONTOUR_BACK_ITERATION_STEPS;
                extern int SUBCONTOUR_BACK_ITERATION_STEPS;
                extern int RED_INCREMENT;
                extern int RED_DECREMENT;
                extern int WHITE_INCREMENT;
                extern int WHITE_RATIO;
                struct ELLIPSE_PARAMETERS {
                    static const cv::Scalar COLOR;
                    static const int THICKNESS;
                };

                struct DIRECTION_INDICATOR_PARAMETERS {
                    static const cv::Scalar COLOR;
                    static const int THICKNESS;
                };
            }
        }
    }
}
#endif //TEST02_PARAMETERS_H
