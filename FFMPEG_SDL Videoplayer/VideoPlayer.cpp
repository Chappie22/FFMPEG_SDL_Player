//
//  VideoPlayer.cpp
//  FFMPEG_SDL Videoplayer
//
//  Created by Kirill Kryklyviy on 12/13/18.
//  Copyright © 2018 Kirill Kryklyviy. All rights reserved.
//

#include <iostream>
#include "VideoPlayer.hpp"

using namespace std;

VideoPlayer::VideoPlayer(string videoPath, string saveFramePath, int frameRaw, int frameNum)
{
    this->videoPath = videoPath;
    this->saveFramePath = saveFramePath;
    this->frameRaw = frameRaw;
    this->saveFrame[1] = frameNum;
    saveFrame[0] = 0;
    
    screen = nullptr;
    renderer = nullptr;
    bmp = nullptr;
    
    formatContext = nullptr;
    codecParam = nullptr;
    codec = nullptr;
    codecContext = nullptr;
    frame = nullptr;
    buffer = nullptr;
}

VideoPlayer::~VideoPlayer()
{
    av_free(buffer); //Free buffer
    av_free(frame); // Free the YUV frame
    avcodec_close(codecContext); // Close the codecs
    avformat_close_input(&formatContext); // Close the video file
    
    delete screen, renderer, bmp;
    delete formatContext,codecParam,codec,codecContext,frame;
}

void VideoPlayer::FindStreamAndDecode()
{
    //Format Data
    int inf = avformat_open_input(&formatContext, videoPath.c_str(), NULL, NULL); //Gets metadata of file (video)
    if (inf != 0) throw invalid_argument("Open file error");
        
    //Info about all streams
    av_dump_format(formatContext, 0, videoPath.c_str(), 0); //Displays stream data
    
    //searching for videostream /(nb_streams == all streams)/
    for (videoStream = 0; videoStream<formatContext->nb_streams; ++videoStream) {
        if (formatContext->streams[videoStream]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) break;
    }
    if (videoStream == formatContext->nb_streams) throw invalid_argument("Unable to find video format");
    
    cout<<"\nVideo stream: "<<videoStream<< " | "<< "All streams: "<< formatContext->nb_streams<<endl;
    
    //Choose the correct codec for video stream
    codecParam = formatContext->streams[videoStream]->codecpar;
    
    codec = avcodec_find_decoder(codecParam->codec_id); //try to finf decoder for video stream
    codecContext = avcodec_alloc_context3(codec); //geting info (context) about video
    
    inf = avcodec_parameters_to_context(codecContext, codecParam);
    if(inf < 0){
        avformat_close_input(&formatContext);
        avcodec_free_context(&codecContext);
        throw invalid_argument("Failed to get video codec");        
    }
    
    //Fill the codec context based on the values from the supplied codec parameters
    inf = avcodec_open2(codecContext, codec, NULL);
    if (inf < 0) throw invalid_argument("Failed to initialize codec context and open video codec");
}

void VideoPlayer::CreateAndConvertFrames()
{
    //Creating structure of decoded video data
    frame = av_frame_alloc(); //Set to default values
    
    //GEtting bytes of buffer size
    numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, codecContext->width, codecContext->height, 1);
    
    //Getting needable buffer memory
    buffer = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
    
    //Converting buffer to a frame
    int inf = av_image_fill_arrays(frame->data, frame->linesize, buffer, AV_PIX_FMT_RGB24, codecContext->width, codecContext->height, 1);
    if (inf <0) throw invalid_argument("Converting image fail");
}

void VideoPlayer::CreateAndConfigureSDLWindow()
{
    //Inits Display
    if (SDL_Init(SDL_INIT_VIDEO)) throw invalid_argument("SDL error");
    
    //Creating screen using SDL
    screen = SDL_CreateWindow("Video Player",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          codecContext->width, codecContext->height,
                                          SDL_WINDOW_OPENGL);
    if (!screen) throw invalid_argument("Creating window error");
    renderer = SDL_CreateRenderer(screen, -1, 0);
    bmp = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STATIC, codecContext->width, codecContext->height);
}

void VideoPlayer::PlayVideo()
{
    AVPacket packet;
    
    //video context
    while (av_read_frame(formatContext, &packet) >= 0) {
        
        //If the stream i want to use, the same, as i found
        if (packet.stream_index == videoStream) {
            
            //The decode process
            int res = avcodec_send_packet(codecContext, &packet);
            if (res < 0) {
                continue;
            }
            
            res = avcodec_receive_frame(codecContext, frame);
            if (res < 0) {
                continue;
            }
            

            SDL_UpdateYUVTexture(bmp, NULL, frame->data[0], frame->linesize[0],
                                 frame->data[1], frame->linesize[1],
                                 frame->data[2], frame->linesize[2]);
            SDL_RenderCopy(renderer, bmp, NULL, NULL);
            SDL_RenderPresent(renderer);
            SDL_UpdateWindowSurface(screen);
            SDL_Delay(1000/frameRaw);
            
            saveFrame[0]++;
            if (saveFrame[0] == saveFrame[1])
            {
                
                this->SaveTexture();
            }
            
        }
    }
}

void VideoPlayer::SaveTexture()
{
    SDL_Texture* target = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, bmp);
    int width, height;
    SDL_QueryTexture(bmp, NULL, NULL, &width, &height);
    SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
    SDL_RenderReadPixels(renderer, NULL, surface->format->format, surface->pixels, surface->pitch);
    SDL_SaveBMP(surface, saveFramePath.c_str());
    SDL_FreeSurface(surface);
    SDL_SetRenderTarget(renderer, target);
}
    
    
    
    
    
    
    


