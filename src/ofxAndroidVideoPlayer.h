/*
 *  ofxiPhoneVideoPlayer.h
 *  ffmpeg
 *
 *  Created by theo on 13/12/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "ofMain.h"

#define __STDC_CONSTANT_MACROS
typedef uint32_t DWORD;

#ifdef __cplusplus
extern "C" {
#endif

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"

#ifdef __cplusplus
}
#endif

#ifndef OF_LOOP_NORMAL

#define OF_LOOP_NONE					0x01
#define OF_LOOP_PALINDROME				0x02
#define OF_LOOP_NORMAL					0x03

#endif

class ofxAndroidVideoPlayer : public ofBaseVideoPlayer, public ofThread{
	
	public:
		ofxAndroidVideoPlayer(void);
		virtual ~ofxAndroidVideoPlayer(void);
	
		bool 				loadMovie(string name);
		void 				closeMovie(){}
		void 				close(){}

		void				update();			//same as idleMovie
		void 				idleMovie(){}		// rename to updateMovie?
		void 				play();
		void 				stop();

		int 				width, height;
		float  				speed;
		bool 				bLoaded;

		bool 				isFrameNew();
		unsigned char * 	getPixels();
		float 				getPosition(){ return 0.0; }
		float 				getSpeed(){ return 1.0; }
		float 				getDuration(){ return 1.0; }
		bool				getIsMovieDone(){ return false; }

		void 				setPosition(float pct){}
		void 				setVolume(int volume){}
		void 				setLoopState(int state){ if(state == OF_LOOP_NORMAL) bLoop = true; else bLoop = false; }
		void   				setSpeed(float speed){}
		void				setFrame(int frame){}  // frame 0 = first frame...

		void 				setPaused(bool bPause);

		int					getCurrentFrame(){return 0;}
		int					getTotalNumFrames(){return 0;}

		void				firstFrame(){}
		void				nextFrame(){}
		void				previousFrame(){}

		float 				getHeight(){ return height; }
		float 				getWidth(){ return width; }

		ofPixels 			& getPixelsRef(){ return pixels; };

		bool isPaused();
		bool isLoaded();
		bool isPlaying();
	
	//from ofVideoPlayer
	public: 
		int					nFrames;				// number of frames
		bool 				bHavePixelsChanged;
		bool				allocated;				// so we know to free pixels or not
		bool				bPlaying;

	private:

		void				threadedFunction();
		bool OpenAVI(string filename);
		void GrabAVIFrame();
		int Update(DWORD milliseconds);
		
		unsigned long		microsLastFrame;

		uint8_t 			*data;
		int					nwidth;
		int					nheight;
		DWORD dwNext;
		DWORD lastFrame;
		double dNext;
		double dDur;
		DWORD dwDur;
		DWORD dwFrame;
		DWORD dwFrameSys;
		void SetZeroCount(void);
		void CloseAVI(void);
		bool bLoad;
		bool bLoop;
		bool bResize;
		bool bActive;
		void SetResize(void);
		void ClearResize(void);

	//private:
		string chFile;
		bool CreateContext();
		void DestroyContext();
		AVFormatContext *pFormatCtx;
		int             videoStream;
		AVCodecContext  *pCodecCtx;
		AVCodec         *pCodec;
		AVFrame         *pFrame; 
		AVFrame         *pFrameRGB;
		AVPacket        packet;
		AVStream* video_st;
		struct SwsContext *img_convert_ctx;
		int             frameFinished;
		int             numBytes;
		uint8_t         *buffer;
		double dTimeBase;
		double dPts;

		ofPixels	pixels;

		bool		newFrame;
		bool bIsNewFrame;
};


