//
// Created by rfreytag on 04.05.18.
//

#ifndef TEST02_HELPERS_H
#define TEST02_HELPERS_H

#include <exception>

#include <opencv2/opencv.hpp>

namespace HSMW {
    namespace Forensics {
        namespace BloodSplatterAngleEstimators {
            namespace Helpers {
                class NotImplementedException: public std::logic_error {
                public:
                    NotImplementedException() : std::logic_error("Function not implemented!") {};
                };

                template<typename T>
                bool isDominantlyRed(T const& bgr) {
                    throw NotImplementedException();
                }

                template<typename T = cv::Vec3b>
                bool isDominantlyRed(cv::Vec3b const& bgr) {
                    return (bgr[2] > bgr[0] && bgr[2] > bgr[1]);
                }

                double euclideanDistance(cv::Point const &a, cv::Point const &b);
                void getPointPairWithMaxEuclideanDistance(
                        std::vector<cv::Point> completeContours, cv::Point &topEndPoint, cv::Point &bottomEndPoint);
            }
        }
    }
}

#endif //TEST02_HELPERS_H
