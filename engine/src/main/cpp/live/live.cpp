//
// Created by yuanhao on 20-6-12.
//
#include <opencv2/opencv.hpp>
#include <opencv2/dnn/dnn.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include "live.h"
#include "../android_log.h"
#include <iostream>

using namespace std;
using namespace cv;
using namespace cv::dnn;

Live::Live() {
    thread_num_ = 8;

    option_.lightmode = true;
    option_.num_threads = thread_num_;
}

Live::~Live() {
    for (int i = 0; i < nets_.size(); ++i) {
        nets_[i]->clear();
        delete nets_[i];
    }
    nets_.clear();
}

int Live::LoadModel(AAssetManager *assetManager, std::vector<ModelConfig> &configs) {
    configs_ = configs;
    model_num_ = static_cast<int>(configs_.size());
//    LOG_ERR("checkkkkk222.");

    for (int i = 0; i < model_num_; ++i) {
        ncnn::Net *net = new ncnn::Net();

        net->opt = option_;
        std::string param = "live/" + configs_[i].name + ".param";
        std::string model = "live/" + configs_[i].name + ".bin";
        int ret = net->load_param(assetManager, param.c_str());


        if (ret != 0) {
            LOG_ERR("LiveBody load param failed.");
            return -2 * (i) - 1;
        }

        ret = net->load_model(assetManager, model.c_str());
        if (ret != 0) {
            LOG_ERR("LiveBody load model failed.");
            return -2 * (i + 1);
        }
        nets_.emplace_back(net);

    }
    return 0;
}
float Live::Detect(cv::Mat &src, FaceBox &box, int orientation) {
    float confidence = 0.f;

    Mat scr_clone = src.clone();


    for (int i = 0; i < model_num_; i++) {
        cv::Mat roi;

        cv::Mat roi_pading;
        cv::Mat roi_pading_final;
        ////
        if (orientation == 8){
            if (box.x2 - box.x1 < 300){
                cv::Rect rect = CalculateBox(box, src.cols, src.rows, configs_[i], 2.0F);
                cv::resize(scr_clone(rect), roi_pading, cv::Size(rect.width, rect.height));

                //equalize/////////////////////
                cv::Mat img_ycrcb;
                //equalize
                cv::cvtColor(roi_pading, img_ycrcb, cv::COLOR_BGR2YCrCb);
                // Split the YCrCb channels
                std::vector<cv::Mat> channels;
                cv::split(img_ycrcb, channels);

                // Apply histogram equalization to the Y channel
                cv::equalizeHist(channels[0], channels[0]);

                // Merge the channels back into a YCrCb image
                cv::merge(channels, img_ycrcb);

                // Convert the YCrCb image back to BGR color space
                cv::cvtColor(img_ycrcb, roi_pading_final, cv::COLOR_YCrCb2BGR);

                cv::Rect rect2 = CalculateBox(box, src.cols, src.rows, configs_[i], 0.9F);
                LOG_ERR("son_checkkkkk___model3_face size============rect3333: %d x %d x %d x %d\n",  rect2.x , rect2.y, rect.x, rect.y);

                int x = rect2.x - rect.x;
                int y = rect2.y - rect.y;

                cv::Rect rect3(x, y, rect2.width, rect2.height);
                cv::resize(roi_pading_final(rect3), roi, cv::Size(configs_[i].width, configs_[i].height));
            }

            else{

                Mat dst = src.clone();
                cv::Mat img_ycrcb;

                //equalize
                cv::cvtColor(dst, img_ycrcb, cv::COLOR_BGR2YCrCb);
                // Split the YCrCb channels
                std::vector<cv::Mat> channels;
                cv::split(img_ycrcb, channels);

                // Apply histogram equalization to the Y channel
                cv::equalizeHist(channels[0], channels[0]);

                // Merge the channels back into a YCrCb image
                cv::merge(channels, img_ycrcb);

                // Convert the YCrCb image back to BGR color space
                cv::cvtColor(img_ycrcb, dst, cv::COLOR_YCrCb2BGR);

                cv::Rect rect5 = CalculateBox(box, src.cols, src.rows, configs_[i], 0.9F);

                cv::resize(dst(rect5), roi, cv::Size(configs_[i].width, configs_[i].height));

            }
        }
        else{

            cv::Rect rect5 = CalculateBox(box, src.cols, src.rows, configs_[i], 0.8F);

            cv::resize(src(rect5), roi, cv::Size(configs_[i].width, configs_[i].height), 0, 0 , cv::INTER_CUBIC);

        }
        //  check color of pixel=
//        cv::Vec3b rgb = roi.at<cv::Vec3b>(0, 0);
//        LOG_ERR("son_checkkkkk___model3===22222==check rgb: %d x %d x %d\n",  rgb[0] , rgb[1], rgb[2]);
//        if (i == 2) {
//
//            cv::cvtColor(roi, roi, cv::COLOR_BGR2RGB);
//            cv::Vec3b rgb2 = roi.at<cv::Vec3b>(0, 0);
//            LOG_ERR("son_checkkkkk___model3===22222==check rgb: %d x %d x %d\n",  rgb2[0] , rgb2[1], rgb2[2]);
//
//        }

        ncnn::Mat in = ncnn::Mat::from_pixels(roi.data, ncnn::Mat::PIXEL_BGR, roi.cols, roi.rows);

        if (i == 2) {
            // Convert BGR to RGB
            cv::cvtColor(roi, roi, cv::COLOR_BGR2RGB);
            ncnn::Mat rgb_in = ncnn::Mat::from_pixels(roi.data, ncnn::Mat::PIXEL_RGB, roi.cols, roi.rows);
            in = rgb_in;
        }

        // inference
        ncnn::Extractor extractor = nets_[i]->create_extractor();
        extractor.set_light_mode(true);
        extractor.set_num_threads(thread_num_);

        extractor.input(net_input_name_.c_str(), in);

        ncnn::Mat out;

        extractor.extract(net_output_name_.c_str(), out); //bug
        // edit ratio output of multi model
        if (i < 2) {
            confidence += out.row(0)[1];
//            if (i == 0){
//                confidence += out.row(0)[1] * 0.5F ;
//            }
        }else{
            if (confidence > 2.998F){
                confidence += 1.8F;
            }
            else{
                confidence += 2 *  out.row(0)[1] ;
            }
        }
        LOG_ERR("son_checkkkkk___model3===22222==model num = %d==%f===%f==scale%f", i, out.row(0)[1], out.row(0)[0], confidence);

    }
    confidence /= ( model_num_ + 1) ;
    LOG_ERR("son_checkkkkk_result_of_all_model____out=%f", confidence);

    box.confidence = confidence;
    return confidence;
}

cv::Rect Live::CalculateBox(FaceBox &box, int w, int h, ModelConfig &config, float scale_) {
    int x = static_cast<int>(box.x1);
    int y = static_cast<int>(box.y1);
    int box_width = static_cast<int>(box.x2 - box.x1 + 1);
    int box_height = static_cast<int>(box.y2 - box.y1 + 1);

    int shift_x = static_cast<int>(box_width * config.shift_x);
    int shift_y = static_cast<int>(box_height * config.shift_y);

    float scale = std::min(
            config.scale,
            std::min((w - 1) / (float) box_width, (h - 1) / (float) box_height)
    );
    int box_center_x = box_width / 2 + x;
    int box_center_y = box_height / 2 + y;
    if (scale_ < scale){
        scale_ = scale;

    }


    int new_width = static_cast<int>(box_width * scale_);
    int new_height = static_cast<int>(box_height * scale_);

    int left_top_x = box_center_x - new_width / 2 + shift_x;
    int left_top_y = box_center_y - new_height / 2 + shift_y;
    int right_bottom_x = box_center_x + new_width / 2 + shift_x;
    int right_bottom_y = box_center_y + new_height / 2 + shift_y;

    if (left_top_x < 0) {
        right_bottom_x -= left_top_x;
        left_top_x = 0;
    }

    if (left_top_y < 0) {
        right_bottom_y -= left_top_y;
        left_top_y = 0;
    }

    if (right_bottom_x >= w) {
        int s = right_bottom_x - w + 1;
        left_top_x -= s;
        right_bottom_x -= s;
    }

    if (right_bottom_y >= h) {
        int s = right_bottom_y - h + 1;
        left_top_y -= s;
        right_bottom_y -= s;
    }

    return cv::Rect(left_top_x, left_top_y, new_width, new_height);
}
