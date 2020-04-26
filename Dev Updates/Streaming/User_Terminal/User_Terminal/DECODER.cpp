#include "DECODER.h"

DECODER::DECODER()
{
    pkt = av_packet_alloc();
    if (!pkt)
        exit(1);

    //codec = avcodec_find_decoder_by_name("libx264rgb");
    codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    //codec = avcodec_find_encoder_by_name("libx264rgb");
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }

    parser = av_parser_init(codec->id);
    if (!parser) {
        fprintf(stderr, "parser not found\n");
        exit(1);
    }

    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }

    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }

    frame = av_frame_alloc();
    CopyFrame = av_frame_alloc();
    CopyFrame->format = AV_PIX_FMT_YUV420P;
    CopyFrame->width = 640;
    CopyFrame->height = 480;
    av_frame_get_buffer(CopyFrame, 32);
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }
}

DECODER::~DECODER()
{
    av_parser_close(parser);
    avcodec_free_context(&c);
    av_frame_free(&frame);
    av_frame_free(&CopyFrame);
    av_packet_free(&pkt);
}

void DECODER::decode(void* buffer, int data_size)
{
    if (!data_size)
        return;

    /* use the parser to split the data into frames */
    uint8_t* data = (uint8_t*)buffer;
    while (data_size > 0) {
        int ret = av_parser_parse2(parser, c, &pkt->data, &pkt->size,
            data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
        if (ret < 0) {
            fprintf(stderr, "Error while parsing\n");
            exit(1);
        }
        data += ret;
        data_size -= ret;

        if (pkt->size) {
            ret = avcodec_send_packet(c, pkt);
            if (ret < 0) {
                fprintf(stderr, "Error sending a packet for decoding\n");
                exit(1);
            }
        }
    }
}

AVFrame* DECODER::flush(void* buffer)
{
    int ret = avcodec_send_packet(c, NULL);
    if (ret < 0) {
        fprintf(stderr, "Error sending a packet for decoding\n");
        exit(1);
    }

    while (ret >= 0) {
        ret = avcodec_receive_frame(c, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        else if (ret < 0) {
            fprintf(stderr, "Error during decoding\n");
            exit(1);
        }

        //do something
        av_frame_copy(CopyFrame, frame);
    }
    avcodec_flush_buffers(c);
    return CopyFrame;
}
