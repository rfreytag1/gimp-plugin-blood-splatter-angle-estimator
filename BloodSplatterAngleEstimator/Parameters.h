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
                    static cv::Scalar COLOR;
                    static int THICKNESS;
                    static int LINE_TYPE;
                };

                struct DIRECTION_INDICATOR_PARAMETERS {
                    static cv::Scalar COLOR;
                    static int THICKNESS;
                    static int LINE_TYPE;
                };
            }
        }
    }
}
#endif //TEST02_PARAMETERS_H
