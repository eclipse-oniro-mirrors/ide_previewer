/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "VideoRecorder.h"
#include "PreviewerEngineLog.h"

#include <chrono>
#include <vector>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif  /* __cplusplus */

// List of popular codecs to try
static const std::vector<AVCodecID> codecIds = {
    AV_CODEC_ID_H264,    // H.264
    AV_CODEC_ID_HEVC,    // H.265
    AV_CODEC_ID_MPEG4,   // MPEG-4
    AV_CODEC_ID_VP8,     // VP8
    AV_CODEC_ID_VP9,     // VP9
    AV_CODEC_ID_AV1,     // AV1
    AV_CODEC_ID_THEORA,  // Theora
    AV_CODEC_ID_MJPEG,   // MJPEG
    AV_CODEC_ID_PRORES   // ProRes
};

VideoRecorder::VideoRecorder()
{
}

VideoRecorder::~VideoRecorder()
{
    Stop();
}

VideoRecorder& VideoRecorder::GetInstance()
{
    static VideoRecorder instance;
    return instance;
}

void VideoRecorder::Start(const std::string& filename, int w, int h, int f)
{
    if (recording.load()) {
        ELOG("Recording is already in progress.");
        return;
    }

    width = w;
    height = h;
    fps = f;
    outputFilename = filename;
    nextFramePts = 0;

    recording = true;
    workerThread = std::thread(&VideoRecorder::EncodingThread, this);
}

void VideoRecorder::Stop()
{
    if (recording.load()) {
        recording = false;

        if (workerThread.joinable()) {
            workerThread.join();
        }

        Cleanup();
    }
}

void VideoRecorder::PushFrame(const uint8_t* rgbData, size_t rgbDataSize)
{
    std::lock_guard<std::mutex> lock(frameMutex);
    lastFrame.resize(rgbDataSize);
    std::copy(rgbData, rgbData + rgbDataSize, lastFrame.data());
}

void VideoRecorder::EncodingThread()
{
    constexpr int defaultTimeout = 5;
    auto lastFrameTime = std::chrono::steady_clock::now();
    auto frameInterval = std::chrono::milliseconds(1000 / fps);

    if (!InitEncoder()) {
        Stop();  // Stop recording in case of initialization failure
        return;
    }

    while (recording.load()) {
        auto currentTime = std::chrono::steady_clock::now();

        {
            std::lock_guard<std::mutex> lock(frameMutex);
            if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastFrameTime) >= frameInterval) {
                lastFrameTime = currentTime;

                if (!EncodeFrame(lastFrame.data())) {
                    Stop();  // Stop recording in case of encoding failure
                    return;
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(defaultTimeout));
    }

    FinalizeEncoder();
}

static std::string GetFileExtension(AVCodecID codecId)
{
    switch (codecId) {
        case AV_CODEC_ID_H264:
        case AV_CODEC_ID_HEVC:
        case AV_CODEC_ID_MPEG4:
        default:
            return ".mp4"; // Default to MP4 if codec is not specifically handled
        case AV_CODEC_ID_VP8:
        case AV_CODEC_ID_VP9:
            return ".webm";
        case AV_CODEC_ID_AV1:
            return ".mkv";
        case AV_CODEC_ID_THEORA:
            return ".ogv";
        case AV_CODEC_ID_MJPEG:
            return ".avi";
        case AV_CODEC_ID_PRORES:
            return ".mov";
    }
}

void VideoRecorder::InitCodecContext(const AVCodec* codec)
{
    constexpr int defaultBitRate = 400000;
    constexpr int defaultGopSize = 10;
    codecContext = avcodec_alloc_context3(codec);
    codecContext->codec_id = codec->id;
    codecContext->bit_rate = defaultBitRate;
    codecContext->width = width;
    codecContext->height = height;
    codecContext->time_base = {1, fps};
    codecContext->framerate = {fps, 1};
    codecContext->gop_size = defaultGopSize;
    codecContext->max_b_frames = 1;
    codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
}

bool VideoRecorder::InitEncoder()
{
    const AVCodec* codec = nullptr;
    for (const auto& codecId : codecIds) {
        codec = avcodec_find_encoder(codecId);
        if (codec) {
            outputFilename += GetFileExtension(codecId);
            break;  // Stop after finding the first working codec
        }
    }
    if (!codec) {
        return false;
    }
    if (avformat_alloc_output_context2(&formatContext, nullptr, nullptr, outputFilename.c_str()) < 0) {
        return false;
    }
    AVStream* stream = avformat_new_stream(formatContext, codec);
    if (!stream) {
        return false;
    }
    InitCodecContext(codec);
    formatContext->streams[0]->time_base = codecContext->time_base;
    if (formatContext->oformat->flags & AVFMT_GLOBALHEADER) {
        codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    if (avcodec_open2(codecContext, codec, nullptr) < 0) {
        return false;
    }
    avcodec_parameters_from_context(stream->codecpar, codecContext);
    if (!(formatContext->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&formatContext->pb, outputFilename.c_str(), AVIO_FLAG_WRITE) < 0) {
            ELOG("Failed to open file.");
            return false;
        }
    }
    if (avformat_write_header(formatContext, nullptr) < 0) {
        ELOG("Failed to write header.");
        return false;
    }
    swsContext = sws_getContext(width, height, AV_PIX_FMT_RGB24, width, height, AV_PIX_FMT_YUV420P, SWS_BILINEAR,
        nullptr, nullptr, nullptr);
    if (!swsContext) {
        ELOG("Failed to initialize the scaling context.");
        return false;
    }
    return true;
}

bool VideoRecorder::EncodeFrame(const uint8_t* rgbData)
{
    AVFrame* rgbFrame = av_frame_alloc();
    rgbFrame->format = AV_PIX_FMT_RGB24;
    rgbFrame->width = width;
    rgbFrame->height = height;
    av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, rgbData, AV_PIX_FMT_RGB24, width, height, 1);

    AVFrame* yuvFrame = av_frame_alloc();
    yuvFrame->format = AV_PIX_FMT_YUV420P;
    yuvFrame->width = width;
    yuvFrame->height = height;
    av_image_alloc(yuvFrame->data, yuvFrame->linesize, width, height, AV_PIX_FMT_YUV420P, 1);

    sws_scale(swsContext, rgbFrame->data, rgbFrame->linesize, 0, height, yuvFrame->data, yuvFrame->linesize);

    yuvFrame->pts = nextFramePts++;

    av_frame_free(&rgbFrame);

    AVPacket packet;
    av_init_packet(&packet);
    packet.data = nullptr;
    packet.size = 0;

    int ret = avcodec_send_frame(codecContext, yuvFrame);
    if (ret < 0) {
        ELOG("Error sending frame to encoder.");
        av_frame_free(&yuvFrame);
        return false;
    }

    ret = avcodec_receive_packet(codecContext, &packet);
    if (ret == 0) {
        packet.stream_index = 0;
        av_packet_rescale_ts(&packet, codecContext->time_base, formatContext->streams[0]->time_base);

        if (av_interleaved_write_frame(formatContext, &packet) < 0) {
            ELOG("Error writing frame.");
            av_packet_unref(&packet);
            av_frame_free(&yuvFrame);
            return false;
        }
        av_packet_unref(&packet);
    } else if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
        ELOG("Error receiving packet from encoder.");
        av_frame_free(&yuvFrame);
        return false;
    }

    av_frame_free(&yuvFrame);
    return true;
}

void VideoRecorder::FinalizeEncoder()
{
    av_write_trailer(formatContext);

    if (!(formatContext->oformat->flags & AVFMT_NOFILE)) {
        avio_closep(&formatContext->pb);
    }

    sws_freeContext(swsContext);
    avcodec_free_context(&codecContext);
}

void VideoRecorder::Cleanup()
{
    avformat_free_context(formatContext); // Free the format context
}
