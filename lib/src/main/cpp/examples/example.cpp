#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include "../include/depth_estimator.h"

// The example is expected to be run from the same directory this file is in
#define CALIB R"(images/calib_params.xml)"
#define L_IMG R"(images/clothes_l.png)"
#define R_IMG R"(images/clothes_r.png)"

#define STREAM_TO_CHARS(s) (new std::string(std::istreambuf_iterator<char>(s), std::istreambuf_iterator<char>()))->c_str()
#define STREAM_TO_VECTOR(s) std::vector<char>(std::istreambuf_iterator<char>(s), std::istreambuf_iterator<char>())

int main() {
    std::ifstream calibStream(CALIB), lImgStream(L_IMG), rImgStream(R_IMG);
    const char *calib = STREAM_TO_CHARS(calibStream);
    std::vector<char> lImg = STREAM_TO_VECTOR(lImgStream), rImg = STREAM_TO_VECTOR(rImgStream);

    DepthEstimator depthEstimator(calib);
    std::vector<float> depth = depthEstimator.calcDepth(lImg, rImg);

    for (auto i : depth) std::cout << i;
}
