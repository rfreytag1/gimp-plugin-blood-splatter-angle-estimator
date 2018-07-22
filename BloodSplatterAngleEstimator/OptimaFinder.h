//
// Created by rfreytag on 04.05.18.
//

#ifndef TEST01_OPTIMAFINDER_H
#define TEST01_OPTIMAFINDER_H

#include <cstddef>
#include <vector>

#include <opencv2/opencv.hpp>

namespace HSMW {
    namespace Forensics {
        namespace BloodSplatterAngleEstimators {
            namespace OptimaFinders {
                std::vector<std::vector<cv::Point>> collectSubContours(std::vector<cv::Point> const &pointChains);

                std::vector<cv::Point>
                findOptima(std::vector<cv::Point> const &distanceOrderedPointChains, cv::Mat const &srcImg,
                           int *maxRedsR = nullptr, int *maxWhitesR = nullptr);
            }
        }
    }
}

#endif //TEST01_OPTIMAFINDER_H
