/*
 *  ofxiPhoneVideoPlayer.cpp
 *  ffmpeg
 *
 *  Created by theo on 13/12/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "ofxAndroidVideoPlayer.h"

//most of this code comes from http://www.gamedev.net/community/forums/topic.asp?topic_id=552132

ofxAndroidVideoPlayer::ofxAndroidVideoPlayer(void){
	bActive			= true;
	data			= 0;
	dwNext			= 0;
	dNext			= 0;
	lastFrame 		= 0;
	width			= 0;
	height			= 0;
	nwidth			= 0;
	nheight			= 0;
	dwFrame			= 0;
	dwFrameSys		= 0;
	dPts			= 0.0;
	bLoad			= false;
	bLoop			= true;
	bResize			= false;	
	microsLastFrame = 0;
	bPlaying    	= false;
	bIsNewFrame 	= false;
}

ofxAndroidVideoPlayer::~ofxAndroidVideoPlayer(void){
	CloseAVI();
}

bool ofxAndroidVideoPlayer::loadMovie(string filename){
	bLoaded = OpenAVI(filename);
	microsLastFrame = ofGetElapsedTimeMicros();
	
	if( bLoaded ){
		pixels.allocate(width, height, 3);

		//get some pixels in there
		bPlaying = true;
		update();
		bPlaying = false;
			
		SetZeroCount();
	}
	
	return bLoaded;
}

void ofxAndroidVideoPlayer::play(){
	bPlaying = true;
	startThread(true,false);
}

void ofxAndroidVideoPlayer::stop(){
	bPlaying = false;
	stopThread();
}

void ofxAndroidVideoPlayer::setPaused(bool bPaused){
	bPlaying = !bPaused;
}

void ofxAndroidVideoPlayer::update(){


	if( bLoaded && bPlaying ){
		
		if(newFrame ){
			newFrame = false;
			
			pixels.setFromPixels(buffer,width,height,3);

			frameFinished = false;
			bIsNewFrame = true;
		}else{
			bIsNewFrame = false;
		}
	}
	
}

bool ofxAndroidVideoPlayer::isFrameNew(){
	return bIsNewFrame;
}

unsigned char * ofxAndroidVideoPlayer::getPixels(){

	return pixels.getPixels();
}


void ofxAndroidVideoPlayer::threadedFunction(){
	while(isThreadRunning()){
		unsigned long micros = ofGetElapsedTimeMicros();
		if(bPlaying) {
			unsigned long deltaMicros = micros - microsLastFrame;
			//deltaMillis = ofClamp(deltaMillis, 1, 20000);
			Update(deltaMicros);
			GrabAVIFrame();
		}

		microsLastFrame = micros;
	}
}

//protected stuff

bool ofxAndroidVideoPlayer::OpenAVI(string szFile)
{

 // must be called before using avcodec lib 
    //avcodec_init();

    // register all the codecs 
    av_register_all();

	chFile = ofToDataPath(szFile);
	
	if(!CreateContext()) return false;
	pFrameRGB=avcodec_alloc_frame();
    if(pFrameRGB==NULL)
	{
		printf("FFMPEG: Can't Alloc Frame! \"%s\"\n", chFile.c_str());
		DestroyContext();
        return 0;
	}

	nwidth = pCodecCtx->width;//ofNextPow2(pCodecCtx->width);
	nheight = pCodecCtx->height;//ofNextPow2(pCodecCtx->height);

	width  = pCodecCtx->width;
	height = pCodecCtx->height;

    numBytes=avpicture_get_size(PIX_FMT_RGB24, nwidth, nheight);
    buffer=new uint8_t[numBytes];

    avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24, nwidth, nheight);

	SetResize();
	dwFrame=0;
	dwFrameSys = 0;
	dwNext=0;
	dNext=0;
	dPts=0;
	bLoad = true;
	bActive = true;
	return true;
}

#define MAX_VIDEOQ_SIZE (5 * 256 * 1024)

void ofxAndroidVideoPlayer::GrabAVIFrame()
{
	if(!bActive) return;
	int nDifFrame = dwFrame - dwFrameSys;
	if(nDifFrame > 0)
	{
		for(int k = 0; k < nDifFrame; k++)
		{
			if(av_read_frame(pFormatCtx, &packet)<0) 
			{
				bActive = false;
				break;
			}
			if(k != (nDifFrame-1)) av_free_packet(&packet);
		}
		if(!bActive) 
		{
			if(bLoop) SetZeroCount();
			return;
		}
		if(packet.stream_index==videoStream)
		{
			dPts = 0;
			// Decode video frame
			avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished,
				&packet);

			if(packet.dts == AV_NOPTS_VALUE 
			   && pFrame->opaque && *(uint64_t*)pFrame->opaque != AV_NOPTS_VALUE) 
			{
				dPts = double(*(uint64_t *)pFrame->opaque);
			} 
			else if(packet.dts != AV_NOPTS_VALUE) 
			{
				dPts = double(packet.dts);
			} 
			else 
			{
				// No se sabe el tiempo, entonces se asigna al tiempo actual
				dPts = 0.0;
			}
			dPts *= av_q2d(video_st->time_base);
			if(dPts == 0.0) dPts = dNext;

			// Did we get a video frame?
			if(frameFinished)
			{
				sws_scale (img_convert_ctx, pFrame->data, pFrame->linesize,
							0, pCodecCtx->height,
							pFrameRGB->data,pFrameRGB->linesize);
				data = pFrameRGB->data[0];
				newFrame = true;
			}
		}
		av_free_packet(&packet);
		dwFrameSys = DWORD(dPts/dTimeBase);
	}
}

int ofxAndroidVideoPlayer::Update(DWORD microseconds)
{
	dwNext += microseconds;
	dNext = double(dwNext)/1000000.;	// Convertir a segundos
	dwFrame = DWORD(dNext/dTimeBase);
	//dwFrameSys = lastFrame;
	return 0;
}
void ofxAndroidVideoPlayer::SetZeroCount(void)
{
	dwFrame=0;
	dwFrameSys = 0;
	dwNext=0;
	dNext=0;
	dPts=0;
	DestroyContext();
	CreateContext();
	bActive = true;
}

void ofxAndroidVideoPlayer::CloseAVI(void)
{
	if(bLoad) 
	{
		DestroyContext();
		delete [] buffer;
		if(pFrameRGB != NULL) av_free(pFrameRGB);
	}
	bLoad = false;
	if(bResize) ClearResize();
}

void ofxAndroidVideoPlayer::SetResize(void)
{
	static int sws_flags = SWS_BICUBIC;
	img_convert_ctx = sws_getContext(pCodecCtx->width, 
									pCodecCtx->height,
									pCodecCtx->pix_fmt,
									nwidth, 
									nheight,
									PIX_FMT_RGB24,
									sws_flags, NULL, NULL, NULL);
	bResize = true;
}

void ofxAndroidVideoPlayer::ClearResize(void)
{
	sws_freeContext(img_convert_ctx);
	bResize = false;
}

// ** Funciones privadas
bool ofxAndroidVideoPlayer::CreateContext(void)
{
	//avformat_open_input(&pFormatCtx, chFile.c_str(),NULL,NULL);//
	int result = av_open_input_file(&pFormatCtx, chFile.c_str(), NULL, 0, NULL);
	if(result!=0)
	{
		ofLogError() << "FFMPEG: Can't open file! error:" << result <<  " " << chFile;
        return false;
	}

    if(av_find_stream_info(pFormatCtx)<0)//avformat_find_stream_info(pFormatCtx,NULL)<0)//
	{
    	ofLogError() << "FFMPEG: Couldn't find stream information! "<< chFile;
        return false;
	}
    //av_dump_format(pFormatCtx, 0, chFile.c_str(), false);
    dump_format(pFormatCtx, 0, chFile.c_str(), false);
    videoStream=-1;
	int i;
    for(i=0; i<int(pFormatCtx->nb_streams); i++)
        if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
        {
            videoStream=i;
            break;
        }
    if(videoStream==-1)
	{
    	ofLogError() << "FFMPEG: Didn't find a video stream! " << chFile;
        return false;
	}

    // Get a pointer to the codec context for the video stream
	video_st=pFormatCtx->streams[videoStream];
    pCodecCtx=pFormatCtx->streams[videoStream]->codec;

    pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec==NULL)
	{
    	ofLogError()<<"FFMPEG: Codec not found! " <<  chFile;
        return false;
	}

    if(avcodec_open(pCodecCtx, pCodec)<0)
	{
    	ofLogError()<<"FFMPEG: Could not open codec! "<<chFile;
        return false;
	}

    pFrame=avcodec_alloc_frame();

	dTimeBase = av_q2d(video_st->time_base);
	dDur = double(video_st->duration)*dTimeBase;
	dwDur = DWORD(dDur*1000);

	GrabAVIFrame();

	return true;
}

void ofxAndroidVideoPlayer::DestroyContext(void)
{
    av_free(pFrame);
    avcodec_close(pCodecCtx);
    av_close_input_file(pFormatCtx);
}


bool ofxAndroidVideoPlayer::isPaused(){
	return !bPlaying;
}

bool ofxAndroidVideoPlayer::isLoaded(){
	return bLoaded;
}

bool ofxAndroidVideoPlayer::isPlaying(){
	return bPlaying;
}
