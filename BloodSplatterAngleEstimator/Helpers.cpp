//
// Created by rfreytag on 04.05.18.
//

#include <opencv2/opencv.hpp>

#include "Helpers.h"

namespace HSMW {
    namespace Forensics {
        namespace BloodSplatterAngleEstimators {
            namespace Helpers {
                double euclideanDistance(cv::Point const &a, cv::Point const &b) {
                    // slowest but most generic
                    // TODO: implement faster method for 2D points through templating
                    return cv::norm(a - b);
                }

                void getPointPairWithMaxEuclideanDistance(
                        std::vector<cv::Point> completeContours, cv::Point &topEndPoint, cv::Point &bottomEndPoint) {
                    double lastMaxDistance = 0;
                    // TODO: sort first then binary search instead?
                    for (int i = 0; i < completeContours.size(); ++i) {
                        for (int j = (i + 1); j < completeContours.size(); ++j) {
                            double currentDistance = euclideanDistance(
                                    completeContours[i], completeContours[j]);
                            if (currentDistance > lastMaxDistance) {
                                lastMaxDistance = currentDistance;
                                topEndPoint = completeContours[i];
                                bottomEndPoint = completeContours[j];
                            }
                        }
                    }
                }
            }
        }
    }
}