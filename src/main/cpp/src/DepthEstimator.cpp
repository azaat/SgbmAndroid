#include "opencv2/calib3d.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "../include/DepthEstimator.h"
#include "Conversions.h"
#include "Logging.h"

namespace sgbmandroid {

constexpr auto kTag = "DepthEstimator";

DepthEstimator::DepthEstimator(const std::string &calibPath) {
    // TODO(TimPushkin): call calibrate

    logI(kTag, "Loading calibration parameters from %s...", calibPath.c_str());

    cv::FileStorage fs(calibPath, cv::FileStorage::READ);

    if (!fs.isOpened()) {
        logE(kTag, "Failed to read calibration parameters from the received file");
        throw std::invalid_argument("Illegal calibration file format");
    }

    fs["leftMapX"] >> mLeftMap.first;
    fs["leftMapY"] >> mLeftMap.second;
    fs["rightMapX"] >> mRightMap.first;
    fs["rightMapY"] >> mRightMap.second;
    fs["Q"] >> mQ;

    logD(kTag, "Loaded calibration parameters:\n"
               "- LeftMapX has size %i x %i\n"
               "- LeftMapY has size %i x %i\n"
               "- RightMapX has size %i x %i\n"
               "- RightMapY has size %i x %i\n"
               "- Q has size %i x %i",
         mLeftMap.first.cols, mLeftMap.first.rows,
         mLeftMap.second.cols, mLeftMap.second.rows,
         mRightMap.first.cols, mRightMap.first.rows,
         mRightMap.second.cols, mRightMap.second.rows,
         mQ.cols, mQ.rows);
}

// NOLINTNEXTLINE
bool DepthEstimator::calibrate(const std::vector<char> &leftImage, const std::vector<char> &rightImage) {
    // TODO(TimPushkin): fill calibration parameters
    return true;
}

void DepthEstimator::getDisparity(cv::InputArray leftImage, cv::InputArray rightImage, cv::OutputArray dst) const {
    logI(kTag, "Computing disparity...");

    cv::Mat leftImageGray;
    cv::Mat rightImageGray;

    cv::cvtColor(leftImage, leftImageGray, cv::COLOR_BGR2GRAY);
    cv::cvtColor(rightImage, rightImageGray, cv::COLOR_BGR2GRAY);

    sgbm->compute(leftImageGray, rightImageGray, dst);
}

void DepthEstimator::getDepthFromDisparity(cv::InputArray disparity, cv::OutputArray dst) const {
    logI(kTag, "Getting depth from disparity...");

    cv::Mat image3d;
    cv::Mat depthMap;

    cv::reprojectImageTo3D(disparity, image3d, mQ);

    depthMap.create(image3d.size(), CV_32FC1);
    image3d.forEach<cv::Point3f>([&](cv::Point3f &point, const int *pos) -> void {
        depthMap.at<float>(pos[0], pos[1]) = minDepth <= point.z && point.z <= maxDepth ? point.z : 0;
    });

    dst.assign(depthMap);
}

std::vector<std::vector<float>> DepthEstimator::estimateDepth(const std::vector<char> &leftImageEncoded,
                                                              const std::vector<char> &rightImageEncoded) const {
    cv::Mat leftImage;
    cv::Mat rightImage;
    cv::Mat depthMap;

    logI(kTag, "Preparing images for depth calculation...");

    leftImage = cv::imdecode(leftImageEncoded, cv::IMREAD_COLOR);
    rightImage = cv::imdecode(rightImageEncoded, cv::IMREAD_COLOR);

    logD(kTag, "Loaded images:\n"
               "- Left image has size %i x %i\n"
               "- Right image has size %i x %i",
         leftImage.cols, leftImage.rows,
         rightImage.cols, rightImage.rows);

    cv::remap(leftImage, leftImage, mLeftMap.first, mLeftMap.second, cv::INTER_LINEAR);
    cv::remap(rightImage, rightImage, mRightMap.first, mRightMap.second, cv::INTER_LINEAR);

    if (imageScaleFactor != 1) {
        cv::Size imageSize(static_cast<int> (static_cast<float> (leftImage.cols) * imageScaleFactor),
                           static_cast<int> (static_cast<float> (leftImage.rows) * imageScaleFactor));

        logD(kTag, "Scaling images to: %i x %i", imageSize.width, imageSize.height);

        cv::resize(leftImage, leftImage, imageSize);
        cv::resize(rightImage, rightImage, imageSize);
    }

    getDisparity(leftImage, rightImage, depthMap);
    depthMap.convertTo(depthMap, CV_32F, internal::disparityCorrectionFactor);
    getDepthFromDisparity(depthMap, depthMap);

    logI(kTag, "Depth calculation finished");

    return twoDimMatToTwoDimVector<float>(depthMap);
}

void DepthEstimator::setMaxDisparity(int value) { sgbm->setNumDisparities(value - internal::minDisparity); }

void DepthEstimator::setBlockSize(int value) {
    sgbm->setBlockSize(value);
    sgbm->setP1(internal::p1Multiplier * value * value);
    sgbm->setP2(internal::p2Multiplier * value * value);
}

void DepthEstimator::setSpeckleWindowSize(int value) { sgbm->setSpeckleWindowSize(value); }

void DepthEstimator::setMinDepth(float value) { minDepth = value; }

void DepthEstimator::setMaxDepth(float value) { maxDepth = value; }

void DepthEstimator::setImageScaleFactor(float value) { imageScaleFactor = value; }

}  // namespace sgbmandroid
