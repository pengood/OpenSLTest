#define LOG_TAG "OpenSLActivity"
#define DGB 1
#include "NativeSlave.h"
#include "log.h"

NativeSlave::NativeSlave(size_t size) {
	mBuffer = new RingBuffer(size);
	nextBuffer = (char*) malloc(8192);
	ALOGD("pzhao-->NativeSlave created");
}

NativeSlave::~NativeSlave() {
	if (bqPlayerObject != NULL) {
		(*bqPlayerObject)->Destroy(bqPlayerObject);
		bqPlayerPlay = NULL;
		bqPlayerBufferQueue = NULL;
		bqPlayerEffectSend = NULL;
		bqPlayerVolume = NULL;
		PlaybackRate = NULL;
	}

	// destroy output mix object, and invalidate all associated interfaces
	if (outputMixObject != NULL) {
		(*outputMixObject)->Destroy(outputMixObject);
		outputMixObject = NULL;
	}
	if (engineObject != NULL) {
		(*engineObject)->Destroy(engineObject);
		engineObject = NULL;
		engineEngine = NULL;
	}

	if (mBuffer != NULL)
		delete mBuffer;
	if (nextBuffer != NULL)
		free(nextBuffer);

	ALOGD("pzhao-->delete NativeSlave");
}
void NativeSlave::createEngine() {
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
	result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE,
			&engineEngine);
	assert(SL_RESULT_SUCCESS == result);
	(void) result;

	// create output mix,
	result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0,
			0, 0);
	assert(SL_RESULT_SUCCESS == result);
	(void) result;

	// realize the output mix
	result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
	assert(SL_RESULT_SUCCESS == result);
	(void) result;
	ALOGD("pzhao-->create engine");
}

void NativeSlave::enqueueBuffer() {
	while (mBuffer->getReadSpace() < 8192 * 2)
		;
	int nextSize = mBuffer->Read(nextBuffer, 8192);
	if (nextSize > 0) {
		SLresult result;
		result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue,
				nextBuffer, nextSize);
		assert(SL_RESULT_SUCCESS == result);
		(void) result;
	}
}

void NativeSlave::clearQueueBuffer() {
	SLresult result;
	result = (*bqPlayerBufferQueue)->Clear(bqPlayerBufferQueue);
	assert(SL_RESULT_SUCCESS == result);
	(void) result;
}

bool NativeSlave::createAudioPlayer(NativeSlave *mSlave) {
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
	const SLInterfaceID ids[4] = { SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND,
			SL_IID_VOLUME, SL_IID_PLAYBACKRATE };
	const SLboolean req[4] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
	SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };
	result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject,
			&audioSrc, &audioSnk, 4, ids, req);
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

	// register callback on the buffer queue
	result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue,
			NativeSlave::playCallBack, mSlave);
	assert(SL_RESULT_SUCCESS == result);
	(void) result;

	//get playback rate interface
	result = (*bqPlayerObject)->GetInterface(bqPlayerObject,
			SL_IID_PLAYBACKRATE, &PlaybackRate);
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
	ALOGD("pzhao-->createAudioPlayer finish");
	return 0;
}

void NativeSlave::startPlay() {
	ALOGD("pzhao-->startPlay");
	enqueueBuffer();
}

void NativeSlave::setPlayAudioPlayer(bool isPlay) {
	SLresult result;
	// make sure the URI audio player was created
	if (bqPlayerPlay != NULL) {
		// set the player's state
		result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay,
				isPlay ? SL_PLAYSTATE_PLAYING : SL_PLAYSTATE_PAUSED);
		assert(SL_RESULT_SUCCESS == result);
	}
	if (!isPlay) {
		clearQueueBuffer();
		mBuffer->Reset();
	}
}

void NativeSlave::setMuteUriAudioPlayer(bool isMute) {
	SLmillibel min = SL_MILLIBEL_MIN;
	SLmillibel max = SL_MILLIBEL_MIN;
	(*bqPlayerVolume)->GetMaxVolumeLevel(bqPlayerVolume, &max);
	ALOGD("pzhao-->max=%d,min=%d", max, min);
	SLresult result;
	if (NULL != bqPlayerVolume) {
		result = (*bqPlayerVolume)->SetMute(bqPlayerVolume, isMute);
		assert(SL_RESULT_SUCCESS == result);
	}
	//setPlaybackRate(1050);
}

void NativeSlave::setPlaybackRate(int rate) {
	/*
	SLpermille minRate, maxRate, stepSize;
	SLuint32 capa;
	(*PlaybackRate)->GetRateRange(PlaybackRate, 0, &minRate, &maxRate, &stepSize, &capa);
	SLpermille mRate;
	(*PlaybackRate)->GetRate(PlaybackRate, &mRate);
	ALOGD("pzhao-->minRate=%d,maxRate=%d,stepSize=%d,capa=%d,getRate=%d", minRate,maxRate,stepSize,capa,mRate);
	*/
	if (NULL != PlaybackRate) {
		SLresult result;
		result = (*PlaybackRate)->SetRate(PlaybackRate, rate);
		assert(SL_RESULT_SUCCESS == result);
	}
}

