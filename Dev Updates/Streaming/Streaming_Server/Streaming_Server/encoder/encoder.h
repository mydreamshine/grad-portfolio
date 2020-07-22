#pragma once
#pragma comment(lib, "avcodec")
#pragma comment(lib, "avutil")
#pragma comment(lib, "swscale")

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}

class ENCODER
{
private:
    const AVCodec* codec;
    AVCodecContext* c = NULL;
    AVFrame* frame;
    AVPacket* pkt;
    char* internal_buffer;
    int packet_size;
    struct SwsContext* sws_ctx;

    int width;
    int height;
    int frame_rate;
    int bit_rate;

    void alloc_codec_context();

public:
    ENCODER(int width, int height, int bit_rate, int frame_rate);
    ~ENCODER();
    void encode(const char* buffer);
    int flush();
    char* buffer();
    int size();
};