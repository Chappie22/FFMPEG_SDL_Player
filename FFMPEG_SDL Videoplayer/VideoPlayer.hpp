//
//  VideoPlayer.hpp
//  FFMPEG_SDL Videoplayer
//
//  Created by Kirill Kryklyviy on 12/13/18.
//  Copyright © 2018 Kirill Kryklyviy. All rights reserved.
//
#include <string>

extern "C"
{
#include <libavdevice/avdevice.h>  //Encoding/Decoding Library
#include <libswscale/swscale.h>    //Color conversion and scaling
#include <libavutil/imgutils.h>    //Image utilities
}

//SDL
#include <SDL2/SDL.h>  //Displaying frames

using std::string;

class VideoPlayer
{
private:
    //Input data
    string videoPath;
    string saveFramePath;
    int frameRaw;
    int saveFrame[2];
    
    //SDL data
    SDL_Window *screen; //Showes the frames
    SDL_Renderer *renderer; //Renders the frames
    SDL_Texture* bmp; //Return texture to the renderer
    
    //FFMPEG data
    int videoStream; //Find the number of videostream
    AVFormatContext *formatContext; //Saves info about founded formats
    AVCodecParameters *codecParam; //Video codec params
    AVCodec *codec; //using to find decoder
    AVCodecContext *codecContext; //have a decoder info for a video stream
    AVFrame *frame; //Frame, used to set new frame data and to show
    
    //Convering frame into buffer
    int numBytes;
    uint8_t *buffer;
    
public:
    
    VideoPlayer(string videPath, string saveFramePath, int frameRaw, int frameNum);
    
    void FindStreamAndDecode();
    
    void CreateAndConvertFrames();
    
    void CreateAndConfigureSDLWindow();
    
    void PlayVideo();
    
    void SaveTexture();
    
    ~VideoPlayer();
};





