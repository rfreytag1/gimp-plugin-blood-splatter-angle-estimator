//
// Created by rfreytag on 04.05.18.
//

#ifndef TEST01_BLOODSPLATTERANGLEESTIMATOR_H
#define TEST01_BLOODSPLATTERANGLEESTIMATOR_H

namespace HSMW {
    namespace Forensics {
        namespace BloodSplatterAngleEstimators {
            void estimateBloodSplatterAngle(cv::Mat const& src, cv::Mat & dst);
        }
    }
}

#endif //TEST01_BLOODSPLATTERANGLEESTIMATOR_H
