//
// Created by yuanhao on 20-6-12.
//

#ifndef LIVEBODYEXAMPLE_LIVE_H
#define LIVEBODYEXAMPLE_LIVE_H

#include <opencv2/core/mat.hpp>
#include "../include/ncnn/net.h"
#include "../definition.h"
#include <opencv2/opencv.hpp>
#include <opencv2/dnn/dnn.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
class Live {
public:
    Live();

    ~Live();

    int LoadModel(AAssetManager *assetManager, std::vector<ModelConfig> &configs);

    float Detect(cv::Mat &src, FaceBox &box, int orientation);

private:
    cv::Rect CalculateBox(FaceBox &box, int w, int h, ModelConfig &config, float scale_);

private:
    std::vector<ncnn::Net *> nets_;
    std::vector<ModelConfig> configs_;
    const std::string net_input_name_ = "data";
    const std::string net_output_name_ = "softmax";
    int model_num_;
    int thread_num_;

    ncnn::Option option_;
};

#endif //LIVEBODYEXAMPLE_LIVE_H
