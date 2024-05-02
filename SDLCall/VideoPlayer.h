#pragma once

#include <iostream>
#include <optional>
#include <vector>

extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libswscale/swscale.h>
}

class VideoFrame
{
public:
	VideoFrame()
	{
		frame_ptr = av_frame_alloc();
	}
	~VideoFrame()
	{
		if (frame_ptr != nullptr)
			av_frame_free(&frame_ptr);
	}
public:
	AVFrame* frame_ptr = nullptr;
};

class RGBFrame
{
public:
	RGBFrame(int w, int h) : width(w), height(h)
	{
		pixels.resize(w * h * 3);
	}

	void fill_with_frame(const VideoFrame& frame);

	~RGBFrame()
	{
		if (sws_ctx != nullptr)
			sws_freeContext(sws_ctx);
	}
public:
	const int width;
	const int height;
	std::vector<uint8_t> pixels;
private:
	SwsContext* sws_ctx = nullptr;
};

class VideoPlayer
{
public:
	VideoPlayer() : format_ctx(nullptr), codec_params(nullptr), codec(nullptr), codec_ctx(nullptr), packet(nullptr), cur_frame()
	{
	}
	~VideoPlayer()
	{
		cleanup();
	}

	int open_video(std::string filepath);
	bool get_next_frame();
	std::optional<int64_t> get_duration();
	std::optional<int> get_width();
	std::optional<int> get_height();
	std::optional<double> get_frame_duration();
	void replay();
	const VideoFrame& frame()
	{
		return cur_frame;
	}
private:
	void cleanup();
	bool eof = false;
	bool sent_packet = false;
	size_t video_stream_idx = -1;
	AVFormatContext* format_ctx;
	AVCodecParameters* codec_params;
	const AVCodec* codec;
	AVCodecContext* codec_ctx;
	AVPacket* packet;
	VideoFrame cur_frame;
};
