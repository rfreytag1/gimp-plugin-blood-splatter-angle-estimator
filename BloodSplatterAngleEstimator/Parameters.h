//
// Created by rfreytag on 04.05.18.
//

#ifndef TEST02_PARAMETERS_H
#define TEST02_PARAMETERS_H

#include <opencv2/opencv.hpp>

namespace HSMW {
    namespace Forensics {
        namespace BloodSplatterAngleEstimators {
            struct BaseAlgorithmParameters {
                cv::Scalar ellipse_color;
                uint ellipse_line_thickness;
                int ellipse_line_type;
                cv::Scalar direction_indicator_color;
                uint direction_indicator_line_thickness;
                int direction_indicator_line_type;

                BaseAlgorithmParameters() : ellipse_color(0, 0, 255, 255), ellipse_line_thickness(2),
                                            ellipse_line_type(CV_AA),
                                            direction_indicator_color(0, 255, 0, 255),
                                            direction_indicator_line_thickness(2),
                                            direction_indicator_line_type(CV_AA) {
                }
            };

            struct GrunertAlgorithmParameters : public BaseAlgorithmParameters {
                int red_weight;
                int white_weight;
                uint white_ratio;
                uint contour_back_iteration_steps;
                uint subcontour_back_iteration_steps;

                GrunertAlgorithmParameters()
                        : BaseAlgorithmParameters(), red_weight(1), white_weight(1), white_ratio(100),
                          contour_back_iteration_steps(1), subcontour_back_iteration_steps(1) {

                }
            };
        }
    }
}
#endif //TEST02_PARAMETERS_H
