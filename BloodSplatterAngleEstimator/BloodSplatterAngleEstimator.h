//
// Created by rfreytag on 04.05.18.
//

#ifndef TEST01_BLOODSPLATTERANGLEESTIMATOR_H
#define TEST01_BLOODSPLATTERANGLEESTIMATOR_H

#include <cstdint>
#include <opencv2/opencv.hpp>

#include "Parameters.h"

namespace HSMW {
    namespace Forensics {
        namespace BloodSplatterAngleEstimators {
            void estimateBloodSplatterAngle(cv::Mat const& src, cv::Mat & dst, GrunertAlgorithmParameters const* params);
            void estimateBloodSplatterAngle2(cv::Mat const& src, cv::Mat & dst, BaseAlgorithmParameters const* params);
        }
    }
}

#endif //TEST01_BLOODSPLATTERANGLEESTIMATOR_H
