#ifndef NATIVESLAVE_H_
#define NATIVESLAVE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <jni.h>
#include <assert.h>
#include "RingBuffer.h"

class NativeSlave {
private :
// engine interfaces
	SLObjectItf engineObject = NULL;
	SLEngineItf engineEngine;
// output mix interfaces
	SLObjectItf outputMixObject = NULL;

// buffer queue player interfaces
	SLObjectItf bqPlayerObject = NULL;
	SLObjectItf uriPlayerObject = NULL;
	SLPlayItf bqPlayerPlay;
	SLPlayItf uriPlayerPlay;
	SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
	SLEffectSendItf bqPlayerEffectSend;
	SLVolumeItf bqPlayerVolume;
	SLPlaybackRateItf PlaybackRate;
	SLVolumeItf uriPlayerVolume;
	SLSeekItf uriPlayerSeek;
	SLMuteSoloItf uriPlayerMuteSolo;

	char *nextBuffer;
	char *allzeroBuf;
public :
	RingBuffer *mBuffer;
	volatile bool pause;
	NativeSlave(size_t size);
	~NativeSlave();
	void createEngine();
	void enqueueBuffer();
	void clearQueueBuffer();
	static void playCallBack(SLAndroidSimpleBufferQueueItf bq,void *context){
		assert(bq == bqPlayerBufferQueue);
			assert(NULL != context);
			NativeSlave *mSlave=(NativeSlave *)context;
			mSlave->enqueueBuffer();
	};
	bool createAudioPlayer(NativeSlave * mSlave);
	void startPlay();
	void setPlayAudioPlayer(bool isPlay);
	void setMuteUriAudioPlayer(bool isMute);
	void setPlaybackRate(int rate);
};



#ifdef __cplusplus
}
#endif

#endif
