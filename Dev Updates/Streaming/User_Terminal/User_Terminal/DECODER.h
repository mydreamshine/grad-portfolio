#pragma once
#pragma comment(lib, "avcodec")
#pragma comment(lib, "avutil")

extern "C" {
#include <libavcodec/avcodec.h>

//#include <libavutil/opt.h>
//#include <libavutil/imgutils.h>
}

#include <iostream>


class DECODER
{
private:
    const AVCodec* codec;
    AVCodecParserContext* parser;
    AVCodecContext* c = NULL;
    AVFrame* frame;
    AVFrame* CopyFrame;
    AVPacket* pkt;

public:
    DECODER();
    ~DECODER();

    void decode(void* buffer, int data_size);
    AVFrame* flush(void* buffer);
    void free_frame(AVFrame** frame);
};