//
//  main.cpp
//  FFMPEG_SDL Videoplayer
//
//  Created by Kirill Kryklyviy on 12/11/18.
//  Copyright © 2018 Kirill Kryklyviy. All rights reserved.
//

#include <iostream>
#include <string>
#include "VideoPlayer.hpp"

using namespace std;

////BMP saving function using SDL
//void static saveTexture(SDL_Renderer *renderer, SDL_Texture *texture, string filename)
//{
//    SDL_Texture* target = SDL_GetRenderTarget(renderer);
//    SDL_SetRenderTarget(renderer, texture);
//    int width, height;
//    SDL_QueryTexture(texture, NULL, NULL, &width, &height);
//    SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
//    SDL_RenderReadPixels(renderer, NULL, surface->format->format, surface->pixels, surface->pitch);
//    SDL_SaveBMP(surface, filename.c_str());
//    SDL_FreeSurface(surface);
//    SDL_SetRenderTarget(renderer, target);
//}

int ARGV_CheckRaw(const char * argv[])
{
    if ((string)argv[2] == "0.5x") return 15;
    else if ((string)argv[2] == "2x") return 60;
    else return 30;
}

string InVideoPath()
{
    string videoPath;
    cout<<"Please enter the videofile path: ";
    cin>>videoPath;
    return videoPath;
}

string InFramePath()
{
    string saveFramePath;
    cout<<"Please, enter the frame path: ";
    cin>>saveFramePath;
    return saveFramePath += "/screenshot.bmp";
}

//Main
int main(int argc, const char * argv[]) {
    
    //Data initialization
    int frameRaw = ARGV_CheckRaw(argv);
    string videoPath = InVideoPath();
    int saveFrame[2];
    saveFrame[0] = 0;
    cout<<"I using Mac and there no ways (at least i do not know) to catch the button tap.\nIf it was Windows function (kbhit) and (getch) easily solve the problem.\nSo, please enter the num of frame you want to save (It will save when you get to this frame): ";
    cin>>saveFrame[1];
    string saveFramePath = InFramePath();
    
    VideoPlayer *player = new VideoPlayer(videoPath, saveFramePath, frameRaw, saveFrame[1]);
    player->FindStreamAndDecode();
    player->CreateAndConvertFrames();
    player->CreateAndConfigureSDLWindow();
    player->PlayVideo();
    
    return 0;
}



