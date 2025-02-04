/*
 * RTSP Client for Streaming Video
 *
 * Author: Kshitij Mistry
 *
 * This program is an RTSP client that receives and decodes an RTSP stream using FFmpeg
 * and displays it using SDL2. It supports different transport protocols (TCP, UDP, HTTP).
 *
 * Libraries Used:
 * - SDL2: Used for rendering video frames.
 * - FFmpeg (libavformat, libavcodec, libavutil): Used for handling and decoding the RTSP stream.
 *
 * Usage:
 *   ./rtsp_client <ip_address> <transport_type> <stream_path>
 *   Example: ./rtsp_client 192.168.101.47 tcp /unicaststream/2
 *
 */

#include <SDL2/SDL.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/log.h>
#include <libavutil/opt.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        printf("Usage: %s <ip_address> <transport_type> <stream_path>\n", argv[0]);
        printf("Example: %s 192.168.101.47 tcp /unicaststream/2\n", argv[0]);
        return -1;
    }

    const char *ipAddress = argv[1];      // IP address (e.g., 192.168.101.47)
    const char *transportType = argv[2];  // Transport type (tcp, udp, http)
    const char *streamPath = argv[3];     // Stream path (e.g., /unicaststream/2)

    /* Construct RTSP URL dynamically */
    char rtspUrl[256];
    char username[50] = {0};
    char password[50] = {0};

    /* Get Username */
    printf("Enter username (Press Enter to skip authentication): ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0;  // Remove trailing newline

    /* Get Password */
    printf("Enter password (Press Enter to skip authentication): ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = 0;  // Remove trailing newline

    /* Construct RTSP URL dynamically */
    if (strlen(username) > 0 && strlen(password) > 0)
    {
        snprintf(rtspUrl, sizeof(rtspUrl), "rtsp://%s:%s@%s%s", username, password, ipAddress, streamPath);
    }
    else
    {
        snprintf(rtspUrl, sizeof(rtspUrl), "rtsp://%s%s", ipAddress, streamPath);
    }

    /* Print the URL and transport type for debugging */
    printf("RTSP URL: %s\n", rtspUrl);
    printf("Transport Type: %s\n", transportType);

    AVFormatContext *fmtCtx = NULL;
    AVCodecContext  *decCtx = NULL;
    AVCodec         *dec = NULL;
    int              videoStreamIndex = -1;
    int              ret;
    AVDictionary    *options = NULL;

    /* Set the RTSP transport protocol (TCP, UDP, or HTTP) dynamically based on command-line argument */
    av_dict_set(&options, "rtsp_transport", transportType, 0);

    /* Initialize libavformat and register all codecs */
    avformat_network_init();

    /* Open the input RTSP stream */
    if ((ret = avformat_open_input(&fmtCtx, rtspUrl, NULL, &options)) < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Failed to open input stream: %s\n", av_err2str(ret));
        return -1;
    }

    /* Retrieve stream information */
    if ((ret = avformat_find_stream_info(fmtCtx, NULL)) < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Failed to retrieve stream information: %s\n", av_err2str(ret));
        return -1;
    }

    /* Find the first video stream */
    for (int i = 0; i < fmtCtx->nb_streams; i++)
    {
        if (fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStreamIndex = i;
            break;
        }
    }

    if (videoStreamIndex == -1)
    {
        av_log(NULL, AV_LOG_ERROR, "Could not find a video stream\n");
        return -1;
    }

    /* Find the decoder for the video stream */
    dec = avcodec_find_decoder(fmtCtx->streams[videoStreamIndex]->codecpar->codec_id);
    if (!dec)
    {
        av_log(NULL, AV_LOG_ERROR, "Failed to find video decoder\n");
        return -1;
    }

    /* Create a codec context for the decoder */
    decCtx = avcodec_alloc_context3(dec);
    if (!decCtx)
    {
        av_log(NULL, AV_LOG_ERROR, "Failed to allocate codec context\n");
        return -1;
    }

    /* Initialize the codec context with the stream parameters */
    if ((ret = avcodec_parameters_to_context(decCtx, fmtCtx->streams[videoStreamIndex]->codecpar)) < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Failed to copy codec parameters to codec context: %s\n", av_err2str(ret));
        return -1;
    }

    /* Open the decoder */
    if ((ret = avcodec_open2(decCtx, dec, NULL)) < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Failed to open codec: %s\n", av_err2str(ret));
        return -1;
    }

    /* Initialize SDL2 for rendering */
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "SDL2 initialization failed: %s\n", SDL_GetError());
        return -1;
    }

    /* Create SDL window */
    SDL_Window *window = SDL_CreateWindow("Video Player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
    if (!window)
    {
        av_log(NULL, AV_LOG_ERROR, "SDL2 window creation failed: %s\n", SDL_GetError());
        return -1;
    }

    /* Create SDL renderer */
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        av_log(NULL, AV_LOG_ERROR, "SDL2 renderer creation failed: %s\n", SDL_GetError());
        return -1;
    }

    /* Create SDL texture for rendering frames */
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING, decCtx->width, decCtx->height);
    if (!texture)
    {
        av_log(NULL, AV_LOG_ERROR, "SDL2 texture creation failed: %s\n", SDL_GetError());
        return -1;
    }

    /* SDL event handling loop */
    SDL_Event event;
    int       quit = 0;
    AVPacket  avPacket = {0};
    AVFrame  *frame;

    /* Main loop to read frames and display them */
    while (!quit)
    {
        /* Process events */
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                /* Exit the loop when the close button is pressed */
                quit = 1;
            }
        }

        /* Read a frame from the RTSP stream */
        ret = av_read_frame(fmtCtx, &avPacket);
        if (ret < 0)
        {
            /* End of stream or error */
            break;
        }

        /* Check if the packet is from the video stream */
        if (avPacket.stream_index == videoStreamIndex)
        {
            /* Send the packet to the decoder */
            ret = avcodec_send_packet(decCtx, &avPacket);
            if (ret < 0)
            {
                av_log(NULL, AV_LOG_ERROR, "Error sending packet to decoder: %s\n", av_err2str(ret));
                break;
            }

            /* Receive a decoded frame */
            frame = av_frame_alloc();
            if (!frame)
            {
                av_log(NULL, AV_LOG_ERROR, "Failed to allocate frame\n");
                break;
            }
            ret = avcodec_receive_frame(decCtx, frame);
            if (ret >= 0)
            {
                /* Successfully received a frame; render it using SDL2 */
                SDL_UpdateYUVTexture(texture, NULL, frame->data[0], frame->linesize[0], frame->data[1], frame->linesize[1], frame->data[2],
                                     frame->linesize[2]);
                SDL_RenderClear(renderer);
                SDL_RenderCopy(renderer, texture, NULL, NULL);
                SDL_RenderPresent(renderer);
                SDL_Delay(40);  // Delay to control the playback speed
            }
            else if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            {
                /* No frame could be decoded, continue reading */
                av_frame_free(&frame);
                continue;
            }
            else
            {
                av_log(NULL, AV_LOG_ERROR, "Error receiving frame: %s\n", av_err2str(ret));
                break;
            }
            av_frame_free(&frame);
        }

        /* Free the packet and continue */
        av_packet_unref(&avPacket);
    }

    /* Clean up */
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    avcodec_free_context(&decCtx);
    avformat_close_input(&fmtCtx);

    return 0;
}
