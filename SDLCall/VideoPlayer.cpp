#include "pch.h"
#include "VideoPlayer.h"




int VideoPlayer::open_video(std::string filepath)
{
	cleanup();
	packet = av_packet_alloc();
	if (avformat_open_input(&format_ctx, filepath.data(), NULL, 0) != 0)
		return -1;
	DEBUG_LOG("Opened file " << filepath << " successfully.");
	if (avformat_find_stream_info(format_ctx, 0) != 0)
		return -1;
	DEBUG_LOG("Stream data loaded.");
	video_stream_idx = av_find_best_stream(format_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	if (video_stream_idx < 0)
		return -1;
	DEBUG_LOG("Found stream.");
	codec_params = format_ctx->streams[video_stream_idx]->codecpar;
	codec = avcodec_find_decoder(codec_params->codec_id);
	if (codec == nullptr)
		return -1;
	DEBUG_LOG("Found coded.");
	codec_ctx = avcodec_alloc_context3(codec);
	if (codec_ctx == nullptr)
		return -1;
	if (avcodec_parameters_to_context(codec_ctx, codec_params) < 0)
		return -1;
	DEBUG_LOG("Loaded codec successfully.");
	if (avcodec_open2(codec_ctx, codec, 0) < 0)
		return -1;
	DEBUG_LOG("Opened codec successfully.");
	return 0;
}

bool VideoPlayer::get_next_frame()
{
	//steps : 
	// read frame into packet, if no frames left, send null packet to flush
	// send packet and receive a frame, if error is AVERROR(EAGAIN), more packets
	// are required to receive a frame so send another packet, otherwise, 
	// handle edge cases and receive a frame
	int ret = AVERROR(EAGAIN);
	while (true)
	{
		if (!sent_packet)
		{
			ret = av_read_frame(format_ctx, packet);

			if (ret == AVERROR_EOF)
				eof = true;

			if (packet->stream_index != video_stream_idx)
				continue;

			AVPacket* to_send = (eof) ? nullptr : packet;
			ret = avcodec_send_packet(codec_ctx, to_send);

			if (packet)
				av_packet_unref(packet);

			if (ret == 0)
				sent_packet = true;
			else
				return false;
		}
		ret = avcodec_receive_frame(codec_ctx, cur_frame.frame_ptr);
		if (ret == AVERROR(EAGAIN) || (!eof && ret == AVERROR(EOF)))
		{
			sent_packet = false;
			continue;
		}
		else if (ret < 0 || (eof && ret == AVERROR(EOF)))
		{
			return false;
		}
		return true;
	}
}

void RGBFrame::fill_with_frame(const VideoFrame& frame)
{
	sws_ctx = sws_getCachedContext(sws_ctx, frame.frame_ptr->width, frame.frame_ptr->height, (AVPixelFormat)frame.frame_ptr->format,
							 width, height, AV_PIX_FMT_RGB24, SWS_BILINEAR, 0, 0, 0);
	uint8_t* prgb24[1] = { pixels.data() };
	int stride[1] = { 3 * width };
	sws_scale(sws_ctx, frame.frame_ptr->data, frame.frame_ptr->linesize, 0, frame.frame_ptr->height, prgb24, stride);
}

void VideoPlayer::replay()
{
	if (!codec_ctx || !format_ctx)
		return;
	av_seek_frame(format_ctx, video_stream_idx, 0, AVSEEK_FLAG_BACKWARD);
	avcodec_flush_buffers(codec_ctx);
	eof = false;
	sent_packet = false;
}

void VideoPlayer::cleanup()
{
	eof = false;
	sent_packet = false;
	if (packet != nullptr)
		av_packet_free(&packet);
	if (codec_ctx != nullptr)
		avcodec_free_context(&codec_ctx);
	if (format_ctx != nullptr)
		avformat_close_input(&format_ctx);
}

std::optional<int64_t> VideoPlayer::get_duration()
{
	if (format_ctx == nullptr)
		return std::nullopt;
	return format_ctx->duration;
}

std::optional<int> VideoPlayer::get_width()
{
	if (codec_ctx == nullptr)
		return std::nullopt;
	return codec_ctx->width;
}

std::optional<int> VideoPlayer::get_height()
{
	if (codec_ctx == nullptr)
		return std::nullopt;
	return codec_ctx->height;
}

std::optional<double> VideoPlayer::get_frame_duration()
{
	if (codec_ctx == nullptr)
		return std::nullopt;
	return 1.0 / av_q2d(format_ctx->streams[video_stream_idx]->avg_frame_rate);
}