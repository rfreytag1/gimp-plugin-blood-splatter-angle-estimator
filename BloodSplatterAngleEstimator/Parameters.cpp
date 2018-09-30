//
// Created by rfreytag on 04.05.18.
//

#include <opencv2/opencv.hpp>

#include "Parameters.h"

namespace HSMW {
    namespace Forensics {
        namespace BloodSplatterAngleEstimators {
            namespace Parameters {
                int CONTOUR_BACK_ITERATION_STEPS = 1;
                int SUBCONTOUR_BACK_ITERATION_STEPS = 1;
                int RED_INCREMENT = 1;
                int RED_DECREMENT = 1;
                int WHITE_INCREMENT = 1;
                int WHITE_RATIO = 100;
                cv::Scalar ELLIPSE_PARAMETERS::COLOR = cv::Scalar(255, 255, 0, 255);
                int ELLIPSE_PARAMETERS::THICKNESS = 3;
                int ELLIPSE_PARAMETERS::LINE_TYPE = CV_AA;
                cv::Scalar DIRECTION_INDICATOR_PARAMETERS::COLOR = cv::Scalar(255, 0, 255, 255);
                int DIRECTION_INDICATOR_PARAMETERS::THICKNESS = 4;
                int DIRECTION_INDICATOR_PARAMETERS::LINE_TYPE = CV_AA;
            }
        }
    }
}