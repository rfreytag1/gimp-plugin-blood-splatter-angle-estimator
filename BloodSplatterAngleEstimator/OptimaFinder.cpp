//
// Created by rfreytag on 04.05.18.
//

#include <cinttypes>
#include <exception>

#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>


#include "OptimaFinder.h"
#include "Helpers.h"
#include "Parameters.h"

namespace HSMW {
    namespace Forensics {
        namespace BloodSplatterAngleEstimators {
            namespace OptimaFinders {
                std::vector<std::vector<cv::Point>> collectSubContours(std::vector<cv::Point> const& pointChains) {
                    std::vector<std::vector<cv::Point>> subContours;
                    for(int i = 0; i < pointChains.size(); i += Parameters::CONTOUR_BACK_ITERATION_STEPS)
                    {
                        std::vector<cv::Point> currentContour;
                        std::copy_n(pointChains.begin(), i, std::back_inserter(currentContour));
                        if(currentContour.size() > 4) {
                            subContours.emplace_back(currentContour);
                            if(Parameters::SUBCONTOUR_BACK_ITERATION_STEPS > 1) {
                                std::vector<cv::Point> currentContourCopy;
                                std::copy(currentContour.begin(), currentContour.end(), std::back_inserter(currentContourCopy));
                                for(int j = 1; j < Parameters::SUBCONTOUR_BACK_ITERATION_STEPS; ++j) {
                                    std::vector<cv::Point> currentContourSliced;
                                    currentContourCopy.erase(currentContourCopy.begin(), currentContour.begin() + j);
                                    std::copy(currentContourCopy.begin(), currentContourCopy.end(), std::back_inserter(currentContourSliced));
                                    subContours.emplace_back(currentContourSliced);
                                    currentContourSliced.erase(currentContourSliced.begin(), currentContourSliced.end());
                                    std::copy_n(currentContour.begin(), currentContour.size() - j,
                                                std::back_inserter(currentContourSliced));
                                    subContours.emplace_back(currentContourSliced);
                                }
                            }
                        }
                    }

                    return subContours;
                }

                std::vector<cv::Point> findOptima(std::vector<cv::Point> const &distanceOrderedPointChains,
                                                                cv::Mat const& srcImg, int *maxRedsR, int *maxWhitesR)
                {
                    std::vector<cv::Point> winnerHull;
                    int maxReds = 0;
                    int maxWhites = 0;
                    cv::Mat srcGrayImg;
                    cv::cvtColor(srcImg, srcGrayImg, cv::COLOR_BGR2GRAY);

                    std::vector<std::vector<cv::Point>> subContours = collectSubContours(distanceOrderedPointChains);
                    for(auto const& subContour : subContours) {
                        int reds = 0;
                        int whites = 0;

                        std::vector<cv::Point> hull;
                        cv::Mat mask;

                        cv::convexHull(subContour, hull);

                        if(hull.empty())
                            continue;

                        mask = cv::Mat::zeros(srcGrayImg.size(), CV_8UC1);
                        cv::drawContours(mask, std::vector<std::vector<cv::Point>>({hull}), 0, 255, 0);

                        std::vector<cv::Point> pixelpoints, tmp_pp;
                        cv::findNonZero(mask, tmp_pp);
                        cv::transpose(tmp_pp, pixelpoints);

                        for(auto & pp : pixelpoints) {
                            if(Helpers::isDominantlyRed(srcImg.at<cv::Vec3b>(pp))) {
                                reds += Parameters::RED_INCREMENT;
                            } else {
                                //reds -= Parameters::RED_DECREMENT;
                                whites += Parameters::WHITE_INCREMENT;
                            }
                        }

                        if(reds > 0) {
                            int currentWhiteRatio = static_cast<int>(static_cast<float>(whites) / static_cast<float>(reds) * 100.0);
                            if(reds > maxReds && currentWhiteRatio < Parameters::WHITE_RATIO) {
                                maxReds = reds;
                                maxWhites = whites;
                                winnerHull = hull;
                            }
                        }
                    }

                    if(maxRedsR != nullptr)
                        *maxRedsR = maxReds;

                    if(maxWhitesR != nullptr)
                        *maxWhitesR = maxWhites;

                    return winnerHull;
                }
            }
        }
    }
}
