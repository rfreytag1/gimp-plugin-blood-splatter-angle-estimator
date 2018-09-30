//
// Created by rfreytag on 04.05.18.
//

#include <iostream>
#include <vector>
#include <algorithm>

#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "BloodSplatterAngleEstimator.h"
#include "Helpers.h"
#include "OptimaFinder.h"
#include "Parameters.h"

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
                        std::vector<cv::Point> winnerHullFromTop, winnerHullFromBottom, totalWinner;
                        winnerHullFromTop = OptimaFinders::findOptima(pointChainFromTopPoint, src);
                        winnerHullFromBottom = OptimaFinders::findOptima(pointChainFromBottomPoint, src);

                        // draw winner onto image buffer to be displayed in GIMP
                        if(!winnerHullFromTop.empty() && !winnerHullFromBottom.empty()) {
                            if(winnerHullFromTop.size() >= winnerHullFromBottom.size()) {
                                totalWinner = winnerHullFromTop;
                            } else {
                                totalWinner = winnerHullFromBottom;
                            }
                        } else {
                            if(!winnerHullFromTop.empty()) {
                                totalWinner = winnerHullFromTop;
                            }
                            if(!winnerHullFromBottom.empty()) {
                                totalWinner = winnerHullFromBottom;
                            }
                        }

                        if(totalWinner.empty()) {
                            continue;
                        }

                        // draw winner ellipse
                        cv::RotatedRect winnerEllipse = cv::fitEllipse(totalWinner);
                        cv::ellipse(dst, winnerEllipse, Parameters::ELLIPSE_PARAMETERS::COLOR, Parameters::ELLIPSE_PARAMETERS::THICKNESS, Parameters::ELLIPSE_PARAMETERS::LINE_TYPE);

                        // determine longest ellipse axism which is our splatter angle
                        std::vector<cv::Point3d> ellipseSkewness;
                        cv::Point2f rrVertices[4];
                        winnerEllipse.points(rrVertices);
                        std::vector<cv::Point> angleLine1, angleLine2, finalAngleLine;

                        // calculate both axis
                        angleLine1.emplace_back(cv::Point2f((rrVertices[0] + rrVertices[1]) / 2.0f));
                        angleLine1.emplace_back(cv::Point2f((rrVertices[2] + rrVertices[3]) / 2.0f));

                        angleLine2.emplace_back(cv::Point2f((rrVertices[1] + rrVertices[2]) / 2.0f));
                        angleLine2.emplace_back(cv::Point2f((rrVertices[3] + rrVertices[0]) / 2.0f));

                        // choose the longest
                        if(Helpers::euclideanDistance(angleLine1[0], angleLine1[1]) > Helpers::euclideanDistance(angleLine2[0], angleLine2[1])) {
                            finalAngleLine = angleLine1;
                        } else {
                            finalAngleLine = angleLine2;
                        }

                        // get all pixel values along the axis line
                        cv::LineIterator srcImgLineIterator(src, finalAngleLine[0], finalAngleLine[1]);
                        cv::Mat relevantValues = cv::Mat::zeros(0, 1, CV_8UC1);
                        for(int i = 0; i < srcImgLineIterator.count; ++i, ++srcImgLineIterator) {
                            cv::Vec3b const& pixel = src.at<cv::Vec3b>(srcImgLineIterator.pos());
                            // only choose dominantly red pixels
                            if(Helpers::isDominantlyRed(pixel)) {
                                double value;
                                cv::minMaxLoc(cv::Mat(pixel), nullptr, &value); // Value of Pixel -> HSV
                                relevantValues.push_back(static_cast<uchar>(value));
                            }
                        }

                        // split axis line in half an calculate the averages of the pixel values along those halves
                        cv::Scalar halfAverage[2];
                        halfAverage[0] = cv::mean(relevantValues.rowRange(0, relevantValues.rows / 2));
                        halfAverage[1] = cv::mean(relevantValues.rowRange(relevantValues.rows / 2 + 1, relevantValues.rows - 1));

                        // depending on which half is brighter, direction can be deduced
                        if(halfAverage[0][0] > halfAverage[1][0]) {
                            cv::arrowedLine(dst, finalAngleLine[0], finalAngleLine[1], Parameters::DIRECTION_INDICATOR_PARAMETERS::COLOR, Parameters::DIRECTION_INDICATOR_PARAMETERS::THICKNESS, Parameters::DIRECTION_INDICATOR_PARAMETERS::LINE_TYPE);
                        } else if (halfAverage[0][0] < halfAverage[1][0]) {
                            cv::arrowedLine(dst, finalAngleLine[1], finalAngleLine[0], Parameters::DIRECTION_INDICATOR_PARAMETERS::COLOR, Parameters::DIRECTION_INDICATOR_PARAMETERS::THICKNESS, Parameters::DIRECTION_INDICATOR_PARAMETERS::LINE_TYPE);
                        } else {
                            // double arrow if splatter direction is not unambiguous
                            cv::arrowedLine(dst, finalAngleLine[0], finalAngleLine[1], Parameters::DIRECTION_INDICATOR_PARAMETERS::COLOR, Parameters::DIRECTION_INDICATOR_PARAMETERS::THICKNESS, Parameters::DIRECTION_INDICATOR_PARAMETERS::LINE_TYPE);
                            cv::arrowedLine(dst, finalAngleLine[1], finalAngleLine[0], Parameters::DIRECTION_INDICATOR_PARAMETERS::COLOR, Parameters::DIRECTION_INDICATOR_PARAMETERS::THICKNESS, Parameters::DIRECTION_INDICATOR_PARAMETERS::LINE_TYPE);
                        }

                        //cv::drawContours(dst, std::vector<std::vector<cv::Point>>({totalWinner}), 0, cv::Scalar(255, 127, 0), 0);
                    }
                }
            }

            void estimateBloodSplatterAngle2(cv::Mat const &src, cv::Mat &dst) {
                cv::Mat srcGrayImage;
                cv::Mat srcGrayCannyImage;
                std::vector<std::vector<cv::Point>> contours;

                // create grayscale image of our source
                cv::cvtColor(src, srcGrayImage, cv::COLOR_BGR2GRAY);

                cv::dilate(srcGrayImage, srcGrayImage, cv::getStructuringElement(cv::MORPH_DILATE, cv::Size(3,3)), cv::Point(-1, -1), 100);
                // canny edge detection
                cv::Canny(srcGrayImage, srcGrayCannyImage, 210, 210);
                // find contours of the edges
                cv::findContours(srcGrayCannyImage, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

                std::vector<cv::Point> testBlurContour;
                cv::blur(contours[0], testBlurContour, cv::Size(250, 250));
                //cv::blur(contours[0], testBlurContour, cv::Size(1, 1));

                cv::drawContours(dst, std::vector<std::vector<cv::Point>>({testBlurContour}), 0, cv::Scalar(255, 0, 0), 5);

                cv::RotatedRect ellipse = cv::fitEllipse(testBlurContour);

                cv::ellipse(dst, ellipse, cv::Scalar(0, 255, 0));
            }
        }
    }
}