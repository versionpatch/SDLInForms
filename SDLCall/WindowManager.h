#pragma once
#include "stdint.h"
#ifdef SDLCALL_EXPORTS
#define WINDOWMANAGER_API __declspec(dllexport)
#else
#define WINDOWMANAGER_API __declspec(dllimport)
#endif


extern "C" WINDOWMANAGER_API void create_window(HWND hwnd);
extern "C" WINDOWMANAGER_API void play_video();