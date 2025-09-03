#include <cstdlib>
#include <numeric>
#include <webp/decode.h>
#include <webp/demux.h>
#include "main.hpp"
#include "config.h"
#include "FileHandle.hpp"

AviUtl::InputPluginDLL input_plugin {
    .flag = AviUtl::InputPluginDLL::Flag::Video,
    .name = "WebP File Reader",
    .filefilter = "WebP File (*.webp)\0*.webp\0",
    .information = "WebP File Reader r" GIT_REV " by Mr-Ojii",
    .func_open = func_open,
    .func_close = func_close,
    .func_info_get = func_info_get,
    .func_read_video = func_read_video,
    .func_is_keyframe = func_is_keyframe,
};

extern "C" AviUtl::InputPluginDLL* __stdcall GetInputPluginTable() {
    char exe_path[MAX_PATH * 2];
    if ( GetModuleFileNameA( NULL, exe_path, sizeof(exe_path) ) ) {
        char* p = exe_path;
        while(*p != '\0')
                p++;
        while(*p != '\\')
                p--;
        p++;
        if ( strcmp( p, "pipe32aui.exe" ) == 0 ) {
            MessageBoxA( HWND_DESKTOP, "Use webpinput.aui with AviUtl ExEdit2 is deprecated.\nUse webpinput.aui2 instead.", "webpinput", MB_OK );
        }
    }

    return &input_plugin;
}

AviUtl::InputHandle func_open(const char* path) {
    FileHandle fh(path);

    if (fh.getHandle() == INVALID_HANDLE_VALUE)
        return nullptr;

    DWORD dwSize = GetFileSize(fh.getHandle(), nullptr);
	if (dwSize == 0xFFFFFFFF) {
        return nullptr;
    }

    input_handler_t* hp = reinterpret_cast<input_handler_t*>(malloc(sizeof(input_handler_t)));
    if (!hp) {
        return nullptr;
    }
    WebPDataInit(&hp->data);

    hp->data.size = dwSize;
    hp->bytes = WebPMalloc(dwSize);
    if (!hp->bytes) {
        WebPFree(hp->bytes);
        return nullptr;
    }
    // hp->data.bytes は const なのでこうする。 const_cast を用いない。
    hp->data.bytes = reinterpret_cast<uint8_t*>(hp->bytes);

    DWORD dwReaded;
    if (!ReadFile(fh.getHandle(), hp->bytes, dwSize, &dwReaded, nullptr) || dwSize != dwReaded) {
        WebPFree(hp->bytes);
        free(hp);
        return nullptr;
    }

    // あってる？
    WebPAnimDecoderOptionsInit(&hp->opt);
    hp->opt.color_mode = MODE_bgrA;
    hp->opt.use_threads = 1;
    hp->dec = WebPAnimDecoderNew(&hp->data, &hp->opt);
    if (!hp->dec) {
        WebPFree(hp->bytes);
        free(hp);
        return nullptr;
    }

    WebPAnimInfo anim_info;
    if (!WebPAnimDecoderGetInfo(hp->dec, &anim_info)) {
        WebPFree(hp->bytes);
        free(hp);
        return nullptr;
    }

    // demuxer を用い、長さを
    const WebPDemuxer* demuxer = WebPAnimDecoderGetDemuxer(hp->dec);
    WebPIterator iter;
    hp->duration = 0;
    if (WebPDemuxGetFrame(demuxer, 1, &iter)) {
        do {
            hp->duration += iter.duration;
        } while (WebPDemuxNextFrame(&iter));
        WebPDemuxReleaseIterator(&iter);
    }
    WebPAnimDecoderReset(hp->dec);

    // ここで BITMAPINFOHEADER と n を set
    hp->n = anim_info.frame_count;
    hp->format = BITMAPINFOHEADER {
        .biSize = sizeof(BITMAPINFOHEADER),
        .biWidth = static_cast<LONG>(anim_info.canvas_width),
        .biHeight = static_cast<LONG>(anim_info.canvas_height),
        .biPlanes = 1,
        .biBitCount = 32,
        .biCompression = BI_RGB,
        .biSizeImage = static_cast<uint32_t>(anim_info.canvas_width * anim_info.canvas_height * 4),
        .biXPelsPerMeter = 0,
        .biYPelsPerMeter = 0,
        .biClrUsed = 0,
        .biClrImportant = 0,
    };

    hp->last_access = -1;

    return hp;
}

BOOL func_close(AviUtl::InputHandle ih) {
    input_handler_t* hp = reinterpret_cast<input_handler_t*>(ih);
    if (hp->dec) {
        WebPAnimDecoderDelete(hp->dec);
        hp->dec = nullptr;
    }
    if (hp->bytes) {
        WebPFree(hp->bytes);
        hp->bytes = nullptr;
        hp->data.size = 0;
        hp->data.bytes = nullptr;
    }
    if (hp) {
        free(hp);
    }
    return TRUE;
}

BOOL func_info_get(AviUtl::InputHandle ih, AviUtl::InputInfo* iip) {
    input_handler_t* hp = reinterpret_cast<input_handler_t*>(ih);

    iip->flag = AviUtl::InputInfo::Flag::Video | AviUtl::InputInfo::Flag::VideoRandomAccess;

    iip->n = hp->n;
    iip->format = &hp->format;
    iip->format_size = hp->format.biSize;
    iip->handler = 0;

    // 互いに素で出す
    iip->rate = hp->n * 1000;
    iip->scale = hp->duration;
    int32_t gcd = std::gcd(iip->scale, iip->rate);
    iip->scale /= gcd;
    iip->rate /= gcd;

    return TRUE;
}

int32_t func_read_video(AviUtl::InputHandle ih, int32_t frame, void* buf) {
    input_handler_t* hp = reinterpret_cast<input_handler_t*>(ih);

    // frame は 0-based
    if (hp->last_access >= frame) {
        WebPAnimDecoderReset(hp->dec);
        hp->last_access = -1;
    }
    size_t count = frame - hp->last_access;

    uint8_t* webp_buf;
    int timestamp;
    for (size_t i = 0; i < count; i++) {
        if (!WebPAnimDecoderHasMoreFrames(hp->dec)) {
            // エラー
            return 0;
        }
        hp->last_access++;
        WebPAnimDecoderGetNext(hp->dec, &webp_buf, &timestamp);
    }

    // ボトムアップに
    size_t step = hp->format.biWidth * 4;
    for (size_t i = 0; i < hp->format.biHeight; i++) {
        uint8_t* src = webp_buf + i * step;
        uint8_t* dst = reinterpret_cast<uint8_t*>(buf) + (hp->format.biHeight - i - 1) * step;
        memcpy(dst, src, step);
    }

    return hp->format.biSizeImage;
}

BOOL func_is_keyframe(AviUtl::InputHandle ih, int32_t frame) {
    return (frame == 0);
}
