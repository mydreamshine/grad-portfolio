#include "encoder.h"

#ifdef av_err2str
#undef av_err2str
av_always_inline char* av_err2str(int errnum)
{
    static char str[AV_ERROR_MAX_STRING_SIZE];
    memset(str, 0, sizeof(str));
    return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
}
#endif

void ENCODER::alloc_codec_context()
{
    avcodec_free_context(&c);
    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }

    av_opt_set(c->priv_data, "preset", "medium", 0);

    /* put sample parameters */
    c->bit_rate = bit_rate;
    /* resolution must be a multiple of two */
    c->width = width;
    c->height = height;
    /* frames per second */
    c->time_base = AVRational{ 1, frame_rate };
    c->framerate = AVRational{ frame_rate, 1 };

    c->gop_size = 1;
    c->max_b_frames = 0;
    c->pix_fmt = AV_PIX_FMT_YUV420P;

    int ret = avcodec_open2(c, codec, NULL);
    if (ret < 0) {
        fprintf(stderr, "Could not open codec: %s\n", av_err2str(ret));
        exit(1);
    }
}

ENCODER::ENCODER(int width, int height, int bit_rate, int frame_rate) :
    width(width),
    height(height),
    bit_rate(bit_rate),
    frame_rate(frame_rate)
{
	av_log_set_level(AV_LOG_ERROR);
    internal_buffer = new char[3 * 1024 * 1024]; //Internal Buffer for Encoded data - 3MB

    sws_ctx = sws_getContext(width, height, AV_PIX_FMT_RGBA, width, height, AV_PIX_FMT_YUV420P,
        SWS_BILINEAR, NULL, NULL, NULL);
    if (!sws_ctx) {
        fprintf(stderr,
            "Impossible to create scale context for the conversion\n");
        exit(1);
    }

    codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
        fprintf(stderr, "Codec H264 not found\n");
        exit(1);
    }

    pkt = av_packet_alloc();
    if (!pkt)
        exit(1);

    alloc_codec_context();

    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }
    frame->format = AV_PIX_FMT_YUV420P;
    frame->width = c->width;
    frame->height = c->height;

    int ret = av_frame_get_buffer(frame, 32);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate the video frame data\n");
        exit(1);
    }
}

ENCODER::~ENCODER()
{
    delete[] internal_buffer;
    avcodec_free_context(&c);
    av_frame_free(&frame);
    av_packet_free(&pkt);
}

void ENCODER::encode(const char* buffer)
{
    int ret;

    av_frame_make_writable(frame);
    /* convert to destination format */
    uint8_t* src[] = { (uint8_t*)buffer, NULL, NULL, NULL };
    int linesize[] = { width * sizeof(int), 0, 0, 0 };
    sws_scale(sws_ctx, (const uint8_t* const*)src, linesize, 0, height, frame->data, frame->linesize);

    for (int i = 0; i < 1; ++i) {
        frame->pts = i;
        ret = avcodec_send_frame(c, frame);
    }
    if (ret < 0) {
        fprintf(stderr, "Error sending a frame for encoding\n");
        exit(1);
    }
}

int ENCODER::flush(char* buffer)
{
    int total = 0;
    int ret = avcodec_send_frame(c, NULL);
    char* pot = internal_buffer;
    if (ret < 0) {
        fprintf(stderr, "Error sending a frame for encoding\n");
        exit(1);
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(c, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        else if (ret < 0) {
            fprintf(stderr, "Error during encoding\n");
            exit(1);
        }

        memcpy(pot, pkt->data, pkt->size);
        total += pkt->size;
        pot   += pkt->size;
        av_packet_unref(pkt);
    }
    memcpy(buffer, internal_buffer, total);
    alloc_codec_context(); // re-generate codec context for new encoding
    return total;
}