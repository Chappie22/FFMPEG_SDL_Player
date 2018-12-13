//
//  main.cpp
//  FFMPEG_SDL Videoplayer
//
//  Created by Kirill Kryklyviy on 12/11/18.
//  Copyright © 2018 Kirill Kryklyviy. All rights reserved.
//

#include <iostream>
#include <string>
//FFMPEG
extern "C"
{
#include <libavdevice/avdevice.h>  //Encoding/Decoding Library
#include <libswscale/swscale.h>    //Color conversion and scaling
#include <libavutil/imgutils.h>    //Image utilities
}

//SDL
#include <SDL2/SDL.h>  //Displaying frames

using namespace std;

//BMP saving function using SDL
void static saveTexture(SDL_Renderer *renderer, SDL_Texture *texture, string filename)
{
    SDL_Texture* target = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, texture);
    int width, height;
    SDL_QueryTexture(texture, NULL, NULL, &width, &height);
    SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
    SDL_RenderReadPixels(renderer, NULL, surface->format->format, surface->pixels, surface->pitch);
    SDL_SaveBMP(surface, filename.c_str());
    SDL_FreeSurface(surface);
    SDL_SetRenderTarget(renderer, target);
}

//Main
int main(int argc, const char * argv[]) {
    
    int frameRaw;
    if ((string)argv[2] == "0.5x") frameRaw = 15;
    else if ((string)argv[2] == "2x") frameRaw = 60;
    else {
        cout<<"Incorrect --speed input!"<<endl;
        frameRaw = 30;
    }
    
    string filepath;
    cout<<"Please enter the videofile path: ";
    cin>>filepath;
    
    int saveFrame[2];
    saveFrame[0] = 0;
    cout<<"I using Mac and there no ways (at least i do not know) to catch the button tap.\nIf it was Windows function (kbhit) and (getch) easily solve the problem.\nSo, please enter the num of frame you want to save (It will save when you get to this frame): ";
    cin>>saveFrame[1];
    
    string saveFramePath;
    cout<<"Please, enter the frame path: ";
    cin>>saveFramePath;
    saveFramePath += "/screenshot.bmp";
    
    
    
    //Inits Display
    if (SDL_Init(SDL_INIT_VIDEO)) {
        cout << "SDL error" << endl;
        return -1;
    }
    
    const char *filename = filepath.c_str(); //videopath initialization
    
    //Format Data
    AVFormatContext *formatContext = NULL; //Saves info about founded formats
    int inf = avformat_open_input(&formatContext, filename, NULL, NULL); //Gets metadata of file (video)
    if (inf != 0){
        cout<<"Open file error";
        return -1;
    }
    
    //Info about all streams
    av_dump_format(formatContext, 0, filename, 0); //DW It should diasplay stream data
    
    //Find the number of videostream
    int videoStream;
    //searching for videostream /(nb_streams == all streams)/
    for (videoStream = 0; videoStream<formatContext->nb_streams; ++videoStream) {
        if (formatContext->streams[videoStream]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) break;
    }
    
    if (videoStream == formatContext->nb_streams)
    {
        cout<<"Unable to find video format"<<endl;
        return -1;
    }
    
    cout<<"\nVideo stream: "<<videoStream<< " | "<< "All streams: "<< formatContext->nb_streams<<endl;
    
    //Choose the correct codec for video stream
    AVCodecParameters *codecParam = formatContext->streams[videoStream]->codecpar;
    
    AVCodec *codec = avcodec_find_decoder(codecParam->codec_id); //try to finf decoder for video stream
    AVCodecContext *codecContext = avcodec_alloc_context3(codec); //geting info (context) about video
    
    inf = avcodec_parameters_to_context(codecContext, codecParam);
    if(inf < 0){
        cout<<"Failed to get video codec"<<endl;
        avformat_close_input(&formatContext);
        avcodec_free_context(&codecContext);
        return -1;
    }
    //Fill the codec context based on the values from the supplied codec parameters
    inf = avcodec_open2(codecContext, codec, NULL);
    if (inf < 0)
    {
        cout<<"Failed to initialize codec context and open video codec"<<endl;
        return -1;
    }
    
    //Creating structure of decoded video data
    AVFrame *frame = av_frame_alloc(); //Set to default values
    
    //GEtting bytes of buffer size
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, codecContext->width, codecContext->height, 1);
    
    //Getting needable buffer memory
    uint8_t *buffer = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
    
    //Converting buffer to a frame
    inf = av_image_fill_arrays(frame->data, frame->linesize, buffer, AV_PIX_FMT_RGB24, codecContext->width, codecContext->height, 1);
    if (inf <0)
    {
        cout <<"Converting image fail"<<endl;
        return -1;
    }
    
    
    
    //Creating screen using SDL
    SDL_Window *screen = SDL_CreateWindow("Video Player",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          codecContext->width, codecContext->height,
                                          SDL_WINDOW_OPENGL);
    
    
    
    if (!screen)
    {
        cout<<"Creating window error";
        return -1;
    }
    
    SDL_Renderer *renderer = SDL_CreateRenderer(screen, -1, 0);
    
    SDL_Texture *bmp = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STATIC, codecContext->width, codecContext->height);
    
    
    //video context
    
    
    //
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
                saveTexture(renderer, bmp, saveFramePath);
            }
            
        }
    }
    
    av_free(buffer);
    
    // Free the YUV frame
    av_free(frame);
    
    // Close the codecs
    avcodec_close(codecContext);
    
    // Close the video file
    avformat_close_input(&formatContext);
    
    
    
    return 0;
}



