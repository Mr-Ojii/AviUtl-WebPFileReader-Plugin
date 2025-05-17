#pragma once
#include <windows.h>
#include <aviutl/input.hpp>
#include <webp/demux.h>

AviUtl::InputHandle func_open(const char*);
BOOL func_close(AviUtl::InputHandle);
BOOL func_info_get(AviUtl::InputHandle, AviUtl::InputInfo*);
int32_t func_read_video(AviUtl::InputHandle, int32_t, void*);
BOOL func_is_keyframe(AviUtl::InputHandle, int32_t);

typedef struct {
    void* bytes;
    BITMAPINFOHEADER format;
    int32_t n;
    int32_t last_access;
    int32_t duration;

    WebPData data;
    WebPAnimDecoder* dec;
    WebPAnimDecoderOptions opt;
} input_handler_t;
