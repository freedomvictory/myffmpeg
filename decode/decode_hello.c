//
// Created by victory on 2020/5/3.
//

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <stdio.h>
#include <stdarg.h>


static void logging(const char*fmt, ...);

int main(int argc, const char* argv[])
{
    //allocate memory for this component
    AVFormatContext* pFormatContext = avformat_alloc_context();
    if(!pFormatContext)
    {
        logging("ERROR : couldn't allocate memory for AVFormatContext");
        return -1;
    }

    if(avformat_open_input(&pFormatContext, "/home/victory/test/652d19565ffc9337a2f2b0f2d0806013.mp4", NULL, NULL) != 0)
    {
        logging("ERROR: could open video file");
        return -1;
    }

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
