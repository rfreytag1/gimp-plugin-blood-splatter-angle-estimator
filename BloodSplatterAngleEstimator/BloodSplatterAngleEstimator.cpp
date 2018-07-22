//
// Created by rfreytag on 04.05.18.
//
#include <vector>
#include <algorithm>

#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "BloodSplatterAngleEstimator.h"
#include "Helpers.h"
#include "OptimaFinder.h"

namespace HSMW {
    namespace Forensics {
        namespace BloodSplatterAngleEstimators {
            void estimateBloodSplatterAngle(cv::Mat const &src, cv::Mat &dst) {
                // get our image buffers ready
                cv::Mat srcGrayImage;
                cv::Mat srcGrayCannyImage;
                std::vector<std::vector<cv::Point>> contours;

                // create grayscale image of our source
                cv::cvtColor(src, srcGrayImage, cv::COLOR_BGR2GRAY);
                // canny edge detection
                cv::Canny(srcGrayImage, srcGrayCannyImage, 210, 210);
                // find contours of the edges
                cv::findContours(srcGrayCannyImage, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

                for (auto const &contour : contours) {
                    std::vector<cv::Point> convexHull;
                    cv::convexHull(contour, convexHull);
                    if (convexHull.size() > 4) {
                        // prepare the mask
                        cv::Mat mask = cv::Mat::zeros(src.size(), CV_8UC1);
                        cv::drawContours(mask, std::vector<std::vector<cv::Point>>({contour}), 0, 255, 0);
                        std::vector<cv::Point> completeContourPoints, pixelpoints, tmp_pp;
                        cv::findNonZero(mask, tmp_pp);
                        cv::transpose(tmp_pp, pixelpoints);

                        for (auto const &pnt : pixelpoints) {
                            //completeContourPoints.emplace_back(cv::Point(pnt.y, pnt.x)); // ??? found like this in original python, but why? Just seems to break things.
                            completeContourPoints.emplace_back(cv::Point(pnt.x, pnt.y));
                        }


                        cv::Point topEnd, bottomEnd;
                        Helpers::getPointPairWithMaxEuclideanDistance(completeContourPoints,
                                                             topEnd, bottomEnd);

                        // create copies of the original list
                        std::vector<cv::Point> pointChainFromTopPoint, pointChainFromBottomPoint;
                        std::copy(completeContourPoints.begin(), completeContourPoints.end(),
                                  std::back_inserter(pointChainFromTopPoint));
                        std::copy(completeContourPoints.begin(), completeContourPoints.end(),
                                  std::back_inserter(pointChainFromBottomPoint));

                        // sort in place by euclidean distance between points and respective end points
                        std::sort(pointChainFromTopPoint.begin(), pointChainFromTopPoint.end(),
                                  [topEnd](cv::Point const &a, cv::Point const &b) {
                                      return Helpers::euclideanDistance(topEnd, a) < Helpers::euclideanDistance(topEnd, b);
                                  });

                        std::sort(pointChainFromBottomPoint.begin(), pointChainFromBottomPoint.end(),
                                  [bottomEnd](cv::Point const &a, cv::Point const &b) {
                                      return Helpers::euclideanDistance(bottomEnd, a) < Helpers::euclideanDistance(bottomEnd, b);
                                  });

                        // determine winner hulls for final analysis
                        std::vector<cv::Point> winnerHullFromTop, winnerHullFromBottom;
                        winnerHullFromTop = OptimaFinders::findOptima(pointChainFromTopPoint, src);
                        winnerHullFromBottom = OptimaFinders::findOptima(pointChainFromBottomPoint, src);

                        // winner onto image buffer to be displayed in GIMP
                        if(!winnerHullFromTop.empty() && !winnerHullFromBottom.empty()) {
                            if(winnerHullFromTop.size() >= winnerHullFromBottom.size()) {
                                cv::ellipse(dst, cv::fitEllipse(winnerHullFromTop), cv::Scalar(255, 255, 0));
                                cv::drawContours(dst, std::vector<std::vector<cv::Point>>({winnerHullFromTop}), 0, cv::Scalar(255, 127, 0), 0);
                            } else {
                                cv::ellipse(dst, cv::fitEllipse(winnerHullFromBottom), cv::Scalar(255, 255, 0));
                                cv::drawContours(dst, std::vector<std::vector<cv::Point>>({winnerHullFromBottom}), 0, cv::Scalar(255, 127, 0), 0);
                            }
                        } else {
                            if(!winnerHullFromTop.empty()) {
                                cv::ellipse(dst, cv::fitEllipse(winnerHullFromTop), cv::Scalar(255, 255, 0));
                                cv::drawContours(dst, std::vector<std::vector<cv::Point>>({winnerHullFromTop}), 0, cv::Scalar(255, 127, 0), 0);
                            }
                            if(!winnerHullFromBottom.empty()) {
                                cv::ellipse(dst, cv::fitEllipse(winnerHullFromBottom), cv::Scalar(255, 255, 0));
                                cv::drawContours(dst, std::vector<std::vector<cv::Point>>({winnerHullFromBottom}), 0, cv::Scalar(255, 127, 0), 0);
                            }
                        }

                        // TODO: final analysis
                    }
                }
            }
        }
    }
}