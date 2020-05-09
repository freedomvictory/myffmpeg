//
// Created by victory on 2020/5/3.


#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <stdio.h>
#include <stdarg.h>


static void logging(const char*fmt, ...);
static int decode_packet(AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame);
static void save_gray_frame(unsigned char *buf, int wrap, int xsize, int ysize, char *filename);


int main(int argc, const char* argv[])
{
    //allocate memory for this component
    AVFormatContext* pFormatContext = avformat_alloc_context();
    if(!pFormatContext)
    {
        logging("ERROR : couldn't allocate memory for AVFormatContext");
        return -1;
    }

    /*open the video file and read it's header and fill the pFormatContext*/
    if(avformat_open_input(&pFormatContext, "/home/victory/test/652d19565ffc9337a2f2b0f2d0806013.mp4", NULL, NULL) != 0)
    {
        logging("ERROR: could open video file");
        return -1;
    }
    logging("opening the  video file and loading container header");
    logging("format:%s, duration:%lld, bit_rate %lld", pFormatContext->iformat->name, pFormatContext->duration, pFormatContext->bit_rate);

    logging("find stream and loading it");
    if(avformat_find_stream_info(pFormatContext, NULL) < 0)
    {
        logging("ERROR could not get the steam info");
        return -1;
    }
    /*there arguments that needed to save*/
    AVCodec *pCodec = NULL;
    AVCodecParameters *pCodecParameters = NULL;
    int video_stream_index = -1;

    for(int i = 0; i < pFormatContext->nb_streams; i++)
    {
        AVCodecParameters *pLocalCodecParameters = NULL;
        pLocalCodecParameters = pFormatContext->streams[i]->codecpar;
        logging("AVStream->time_base before open coded %d/%d", pFormatContext->streams[i]->time_base.num, pFormatContext->streams[i]->time_base.den);
        logging("AVStream->r_frame_rate before open coded %d/%d", pFormatContext->streams[i]->r_frame_rate.num, pFormatContext->streams[i]->r_frame_rate.den);
        logging("AVStream->start_time %" PRId64, pFormatContext->streams[i]->start_time);
        logging("AVStream->duration %" PRId64, pFormatContext->streams[i]->duration);

        logging("finding the proper decoder (CODEC)");

        AVCodec *pLocalCodec = NULL;
        pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);
        if(pLocalCodec == NULL)
        {
            logging("ERROR unsupported codec");
            return -1;
        }

        if(pLocalCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            if(video_stream_index == -1)
            {
                video_stream_index = i;
                pCodec = pLocalCodec;
                pCodecParameters = pLocalCodecParameters;
            }
            logging("Video codec : resolution %d * %d", pLocalCodecParameters->width, pLocalCodecParameters->height);
        }
        else if(pLocalCodecParameters->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            logging("Audio codec: channels %d, sample rate %d", pLocalCodecParameters->channels, pLocalCodecParameters->sample_rate);
        }

        logging("\tCodec %s ID %d bit rate %lld ", pLocalCodec->name, pLocalCodec->id, pLocalCodecParameters->bit_rate);
    }

    AVCodecContext* pCodecContext = avcodec_alloc_context3(pCodec);
    if(!pCodecContext)
    {
        logging("ERROR: fail to allocate memory for AVCodecContext");
        return -1;
    }

    if(avcodec_parameters_to_context(pCodecContext, pCodecParameters) < 0)
    {
        logging("ERROR: fail to copy CodecParameters to context");
        return -1;
    }

    if(avcodec_open2(pCodecContext, pCodec, NULL) < 0)
    {
        logging("ERROR: fail to open codec through avcodec_open2");
        return -1;
    }

    AVPacket* pAVPacket = av_packet_alloc();
    if(!pAVPacket)
    {
        logging("ERROR: fail to av_packet_alloc");
        return -1;
    }
    AVFrame* pAVFrame = av_frame_alloc();
    if(!pAVFrame)
    {
        logging("ERROR: fail to allocate memory for AVFrame");
        return -1;
    }

    int response = 0;
    int how_many_packets_to_process = 8;

    while(av_read_frame(pFormatContext, pAVPacket) >= 0)
    {
        if(pAVPacket->stream_index = video_stream_index)
        {
            logging("AVPacket->pts %", PRId64, pAVPacket->pts);
            response = decode_packet(pAVPacket, pCodecContext, pAVFrame);
            if(response < 0)
                break;
            while(--how_many_packets_to_process <= 0)
                break;
        }

        av_packet_unref(pAVPacket);
    }
    logging("releasing all the resources ");
    avformat_close_input(&pFormatContext);
    av_packet_free(&pAVPacket);
    av_frame_free(&pAVFrame);
    avcodec_free_context(&pCodecContext);
    return 0;



    getchar();




    return 0;
}

static void logging(const char *fmt, ...)
{
    va_list args;
    fprintf( stderr, "LOG: " );
    va_start( args, fmt );
    vfprintf( stderr, fmt, args );
    va_end( args );
    fprintf( stderr, "\n" );
}
static int decode_packet(AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame)
{
    // Supply raw packet data as input to a decoder
    // https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga58bc4bf1e0ac59e27362597e467efff3
    int response = avcodec_send_packet(pCodecContext, pPacket);

    if (response < 0) {
        logging("Error while sending a packet to the decoder: %s", av_err2str(response));
        return response;
    }

    while (response >= 0)
    {
        // Return decoded output data (into a frame) from a decoder
        // https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga11e6542c4e66d3028668788a1a74217c
        response = avcodec_receive_frame(pCodecContext, pFrame);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
            break;
        } else if (response < 0) {
            logging("Error while receiving a frame from the decoder: %s", av_err2str(response));
            return response;
        }

        if (response >= 0) {
            logging(
                    "Frame %d (type=%c, size=%d bytes) pts %d key_frame %d [DTS %d]",
                    pCodecContext->frame_number,
                    av_get_picture_type_char(pFrame->pict_type),
                    pFrame->pkt_size,
                    pFrame->pts,
                    pFrame->key_frame,
                    pFrame->coded_picture_number
            );

            char frame_filename[1024];
            snprintf(frame_filename, sizeof(frame_filename), "%s-%d.pgm", "frame", pCodecContext->frame_number);
            // save a grayscale frame into a .pgm file
            save_gray_frame(pFrame->data[0], pFrame->linesize[0], pFrame->width, pFrame->height, frame_filename);
        }
    }
    return 0;
}

static void save_gray_frame(unsigned char *buf, int wrap, int xsize, int ysize, char *filename)
{
    FILE *f;
    int i;
    f = fopen(filename,"w");
    // writing the minimal required header for a pgm file format
    // portable graymap format -> https://en.wikipedia.org/wiki/Netpbm_format#PGM_example
    fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);

    // writing line by line
    for (i = 0; i < ysize; i++)
        fwrite(buf + i * wrap, 1, xsize, f);
    fclose(f);
}
