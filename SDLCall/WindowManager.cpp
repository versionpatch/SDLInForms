#include "pch.h"
#include "WindowManager.h"
#include "SDL.h"
#include "thread"
#include "chrono"
#include <iostream>
#include "VideoPlayer.h"
#include <sstream>

static bool play_video_flag = false;

void play_video()
{
	play_video_flag = true;
	std::cout << "Pressed output\n";
}

std::string get_chunk_filepath(int i)
{
	std::stringstream str;
	str << "C:\\Users\\Hachem\\Documents\\WebServerTest\\chunk_" << i << ".mp4";
	return str.str();
}

void create_window(HWND hwnd)
{
	AllocConsole();
	freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);


	SDL_SetMainReady();
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window = SDL_CreateWindowFrom(hwnd);
	SDL_Surface* window_surface = SDL_GetWindowSurface(window);
	SDL_UpdateWindowSurface(window);
	SDL_Event e;
	auto last_tick = std::chrono::high_resolution_clock::now();
	auto start_tick = last_tick;

	VideoPlayer player1;
	VideoPlayer player2;

	VideoPlayer* current_player = &player1;
	VideoPlayer* hidden_player = &player2;

	int current_chunk = 0;
	current_player->open_video(get_chunk_filepath(current_chunk));
	hidden_player->open_video(get_chunk_filepath(current_chunk + 1));


	double frame_time = current_player->get_frame_duration().value_or(1.0) / 5.0;
	double clock = 0.0;
	bool paused = false;

	RGBFrame frame(window_surface->w, window_surface->h);

	while (true)
	{
		while (SDL_PollEvent(&e))
		{
		}
		if (play_video_flag)
		{
			play_video_flag = false;
			paused = !paused;
		}
		auto tick = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> dt = (tick - last_tick);
		std::chrono::duration<double> t = (tick - start_tick);
		if (!paused)
			clock += dt.count();
		if (clock >= frame_time)
		{
			clock = 0.0;
			if (current_player->get_next_frame())
			{
				frame.fill_with_frame(current_player->frame());
				SDL_ConvertPixels(window_surface->w, window_surface->h, SDL_PIXELFORMAT_RGB24, frame.pixels.data(), 3 * window_surface->w, window_surface->format->format, window_surface->pixels, window_surface->pitch);
			}
			else
			{
				current_chunk += 1;
				std::swap(current_player, hidden_player);
				hidden_player->open_video(get_chunk_filepath(current_chunk + 1));
			}
		}
		SDL_UpdateWindowSurface(window);
		last_tick = tick;
	}
}