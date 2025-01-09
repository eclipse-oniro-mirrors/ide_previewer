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

#ifndef VIDEO_RECORDER_H
#define VIDEO_RECORDER_H

#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <cstdint>
#include <vector>

// Forward declarations for FFmpeg structures
struct AVCodec;
struct AVCodecContext;
struct AVFormatContext;
struct AVFrame;
struct AVPacket;
struct SwsContext;

class VideoRecorder {
public:
    // Get the singleton instance
    static VideoRecorder& GetInstance();

    // Delete copy constructor and assignment operator
    VideoRecorder(const VideoRecorder&) = delete;
    VideoRecorder& operator=(const VideoRecorder&) = delete;

    // Function to start video recording
    void Start(const std::string& filename, int w, int h, int f);

    // Function to stop video recording
    void Stop();

    // Function to pass a frame in RGB format
    void PushFrame(const uint8_t* rgbData, size_t rgbDataSize);

private:
    VideoRecorder();
    ~VideoRecorder();

    std::thread workerThread;
    std::atomic<bool> recording{false};
    std::mutex frameMutex;
    std::vector<uint8_t> lastFrame;

    std::string outputFilename;
    int width{0};
    int height{0};
    int fps{0};
    AVCodecContext* codecContext{nullptr};
    SwsContext* swsContext{nullptr};
    AVFormatContext* formatContext{nullptr};
    int64_t nextFramePts = 0;

    void InitCodecContext(const AVCodec* codec);
    // Encoding thread
    void EncodingThread();

    // Encoder initialization
    bool InitEncoder();

    // Encoding a single frame
    bool EncodeFrame(const uint8_t* rgbData);

    // Finalize the encoder
    void FinalizeEncoder();

    // Resource cleanup
    void Cleanup();
};

#endif // VIDEO_RECORDER_H
