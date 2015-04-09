#define LOG_TAG "OpenSLActivity"
#define DGB 1

#include "log.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>

#include "com_pzhao_opensltest_MainActivity.h"
#include "thread.h"
#include"CycleBuffer.h"

class PlaybackThread;
class WriteThread;
// engine interfaces
static SLObjectItf engineObject = NULL;
static SLEngineItf engineEngine;

// output mix interfaces
static SLObjectItf outputMixObject = NULL;

// buffer queue player interfaces
static SLObjectItf bqPlayerObject = NULL;
static SLObjectItf uriPlayerObject = NULL;
static SLPlayItf bqPlayerPlay;
static SLPlayItf uriPlayerPlay;
static SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
static SLEffectSendItf bqPlayerEffectSend;
static SLVolumeItf bqPlayerVolume;
static SLVolumeItf uriPlayerVolume;
static SLSeekItf uriPlayerSeek;
static SLMuteSoloItf uriPlayerMuteSolo;

static FILE *fp;
static short *nextBuffer;
static unsigned nextSize;
static bool isPlay;

static PlaybackThread* mThread;



class PlaybackThread {
private:
	FILE *mFile;
	void* mBuffer;
	size_t mSize;
	bool read;
public:
	PlaybackThread(const char* uri) :
			mFile(NULL), mBuffer(NULL), mSize(0), read(true) {
		mFile = fopen((char*) uri, "r");
		if (mFile == NULL) {
			ALOGD("open file error %s", uri);
			return;
		}
		mBuffer = malloc(8192);
	}

	void start() {
		if (mFile == NULL) {
			return;
		}
		ALOGD("pzhao-->start first");
		enqueueBuffer();
	}

	// release file buffer
	void release() {
		if (mFile != NULL) {
			fclose(mFile);
			mFile == NULL;
		}
		if (mBuffer != NULL) {
			free(mBuffer);
			mBuffer == NULL;
		}
	}

	~PlaybackThread() {
		release();
		ALOGD("~PlaybackThread");
	}

	void enqueueBuffer() {
		if (bqPlayerBufferQueue == NULL) {
			return;
		}
		// for streaming playback, replace this test by logic to find and fill the next buffer
		while (true) {
			if (read) {
				mSize = fread(mBuffer, 1, 8192, mFile);
			}
			if (mSize > 0) {
				SLresult result;
				// enqueue another buffer
				result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue,
						mBuffer, mSize);
				if (result == SL_RESULT_BUFFER_INSUFFICIENT) {
					read = false;
					return;
				}
				read = true;
			} else {
				return;
			}
		}
	}

	// this callback handler is called every time a buffer finishes playing
	static void playerCallback(SLAndroidSimpleBufferQueueItf bq,
			void *context) {
		assert(NULL != context);

		PlaybackThread* thread = (PlaybackThread *) context;
		if (thread != NULL) {
			ALOGD("pzhao-->playerCallback is called first");
			thread->enqueueBuffer();
		}
	}
};

/*
 * Class:     com_pzhao_opensltest_MainActivity
 * Method:    createEngine
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_pzhao_opensltest_MainActivity_createEngine
(JNIEnv *env, jclass clazz) {
	SLresult result;

	// create engine
	result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
	assert(SL_RESULT_SUCCESS == result);
	(void) result;

	// realize the engine
	result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
	assert(SL_RESULT_SUCCESS == result);
	(void) result;

	// get the engine interface, which is needed in order to create other objects
	result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
	assert(SL_RESULT_SUCCESS == result);
	(void) result;

	// create output mix,
	result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, 0, 0);
	assert(SL_RESULT_SUCCESS == result);
	(void) result;

	// realize the output mix
	result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
	assert(SL_RESULT_SUCCESS == result);
	(void) result;
}
/*
 * Class:     com_pzhao_opensltest_MainActivity
 * Method:    createAudioPlayer
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_pzhao_opensltest_MainActivity_createAudioPlayer(
		JNIEnv *env, jclass clazz, jstring uri) {
	const char* utf8Uri = env->GetStringUTFChars(uri, NULL);

	SLresult result;
	// configure audio source
	SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {
			SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 3 };
	SLDataFormat_PCM format_pcm = { SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1,
			SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
			SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
			SL_BYTEORDER_LITTLEENDIAN };
	SLDataSource audioSrc = { &loc_bufq, &format_pcm };

	// configure audio sink
	SLDataLocator_OutputMix loc_outmix = { SL_DATALOCATOR_OUTPUTMIX,
			outputMixObject };
	SLDataSink audioSnk = { &loc_outmix, NULL };

	// create audio player
	const SLInterfaceID ids[3] = { SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND,
			SL_IID_VOLUME };
	const SLboolean req[3] =
			{ SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };
	result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject,
			&audioSrc, &audioSnk, 3, ids, req);
	assert(SL_RESULT_SUCCESS == result);
	(void) result;

	// realize the player
	result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
	assert(SL_RESULT_SUCCESS == result);
	(void) result;

	// get the play interface
	result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY,
			&bqPlayerPlay);
	assert(SL_RESULT_SUCCESS == result);
	(void) result;

	// get the buffer queue interface
	result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
			&bqPlayerBufferQueue);
	assert(SL_RESULT_SUCCESS == result);
	(void) result;

	mThread = new PlaybackThread(utf8Uri);
	// register callback on the buffer queue
	result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue,
			PlaybackThread::playerCallback, mThread);
	assert(SL_RESULT_SUCCESS == result);
	(void) result;

	// get the effect send interface
	result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_EFFECTSEND,
			&bqPlayerEffectSend);
	assert(SL_RESULT_SUCCESS == result);
	(void) result;

	// get the volume interface
	result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_VOLUME,
			&bqPlayerVolume);
	assert(SL_RESULT_SUCCESS == result);
	(void) result;

	// set the player's state to playing
	result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
	assert(SL_RESULT_SUCCESS == result);
	(void) result;
	//pthread_t id;

	mThread->start();
	env->ReleaseStringUTFChars(uri, utf8Uri);
	ALOGD("createAudioPlayer finish");
	return 0;
}

/*
 * add playCallBack
 * test
 *
 */
void bpPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
	assert(bq == bqPlayerBufferQueue);
	assert(NULL == context);
	ALOGD("pzhao-->I'm in new playcallback");
	nextSize = fread(nextBuffer, 1, 8192, fp);
		if (nextSize > 0) {
			SLresult result;
			result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue,nextBuffer, nextSize);
			if (SL_RESULT_SUCCESS != result) {
				return ;
			}
		}

}

/*
 * Class:     com_pzhao_opensltest_MainActivity
 * Method:    createUriAudioPlayer
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_pzhao_opensltest_MainActivity_createUriAudioPlayer(
		JNIEnv *env, jclass clazz, jstring uri) {
	const char* utf8Uri = env->GetStringUTFChars(uri, NULL);

	SLresult result;
	// configure audio source
	SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {
			SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 3 };
	SLDataFormat_PCM format_pcm = { SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1,
			SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
			SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
			SL_BYTEORDER_LITTLEENDIAN };
	SLDataSource audioSrc = { &loc_bufq, &format_pcm };

	// configure audio sink
	SLDataLocator_OutputMix loc_outmix = { SL_DATALOCATOR_OUTPUTMIX,
			outputMixObject };
	SLDataSink audioSnk = { &loc_outmix, NULL };

	// create audio player
	const SLInterfaceID ids[3] = { SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND,
			SL_IID_VOLUME };
	const SLboolean req[3] =
			{ SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };
	result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject,
			&audioSrc, &audioSnk, 3, ids, req);
	assert(SL_RESULT_SUCCESS == result);
	(void) result;

	// realize the player
	result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
	assert(SL_RESULT_SUCCESS == result);
	(void) result;

	// get the play interface
	result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY,
			&bqPlayerPlay);
	assert(SL_RESULT_SUCCESS == result);
	(void) result;

	// get the buffer queue interface
	result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
			&bqPlayerBufferQueue);
	assert(SL_RESULT_SUCCESS == result);
	(void) result;

//	mThread = new PlaybackThread(utf8Uri);
	// register callback on the buffer queue
	result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue,
			bpPlayerCallback, NULL);
	assert(SL_RESULT_SUCCESS == result);
	(void) result;

	// get the effect send interface
	result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_EFFECTSEND,
			&bqPlayerEffectSend);
	assert(SL_RESULT_SUCCESS == result);
	(void) result;

	// get the volume interface
	result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_VOLUME,
			&bqPlayerVolume);
	assert(SL_RESULT_SUCCESS == result);
	(void) result;

	// set the player's state to playing
	result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
	assert(SL_RESULT_SUCCESS == result);
	(void) result;
	//pthread_t id;

//	mThread->start();
	fp = fopen((char*) utf8Uri, "r");
	if (fp == NULL) {
		ALOGD("open file error %s", utf8Uri);
		return JNI_FALSE;
	}
	nextBuffer = (short *) malloc(8192);
	nextSize = fread(nextBuffer, 1, 8192, fp);
	if (nextSize > 0) {
		SLresult result;
		result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue,nextBuffer, nextSize);
		if (SL_RESULT_SUCCESS != result) {
			return JNI_FALSE;
		}
	}
	env->ReleaseStringUTFChars(uri, utf8Uri);
	ALOGD("createAudioPlayer finish");
	return 0;
}

/*
 * Class:     com_pzhao_opensltest_MainActivity
 * Method:    setPlayingUriAudioPlayer
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL Java_com_pzhao_opensltest_MainActivity_setPlayingUriAudioPlayer
(JNIEnv *env, jclass clazz, jboolean isPlaying) {
	SLresult result;

	// make sure the URI audio player was created
	if (bqPlayerPlay != NULL) {
		// set the player's state
		result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, isPlaying ? SL_PLAYSTATE_PLAYING : SL_PLAYSTATE_PAUSED);
		assert(SL_RESULT_SUCCESS == result);
	}
}

/*
 * Class:     com_pzhao_opensltest_MainActivity
 * Method:    setMuteUriAudioPlayer
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL Java_com_pzhao_opensltest_MainActivity_setMuteUriAudioPlayer
(JNIEnv *env, jclass clazz, jboolean mute) {
	SLresult result;
	if (NULL != bqPlayerVolume) {
		result = (*bqPlayerVolume)->SetMute(bqPlayerVolume, mute);
		assert(SL_RESULT_SUCCESS == result);
	}
}

/*
 * Class:     com_pzhao_opensltest_MainActivity
 * Method:    shutdown
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_pzhao_opensltest_MainActivity_shutdown
(JNIEnv *env, jclass clazz) {
	//destory player object
	if (bqPlayerObject != NULL) {
		(*bqPlayerObject)->Destroy(bqPlayerObject);
		bqPlayerPlay = NULL;
		bqPlayerBufferQueue = NULL;
		bqPlayerEffectSend = NULL;
		bqPlayerVolume = NULL;
	}

	// destroy output mix object, and invalidate all associated interfaces
	if (outputMixObject != NULL) {
		(*outputMixObject)->Destroy(outputMixObject);
		outputMixObject = NULL;
	}

	// destroy engine object, and invalidate all associated interfaces
	if (engineObject != NULL) {
		(*engineObject)->Destroy(engineObject);
		engineObject = NULL;
		engineEngine = NULL;
	}

	if (mThread != NULL) {
		delete mThread;
		mThread = NULL;
	}
}
