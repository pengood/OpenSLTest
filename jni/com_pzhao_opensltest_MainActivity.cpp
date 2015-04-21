#define LOG_TAG "OpenSLTest"
#define DGB 1
#include "log.h"

#include "com_pzhao_opensltest_MainActivity.h"
#include "NativeSlave.h"

void* buffer;
long bufferSize;
bool firstFlag;
NativeSlave *mSlave;

JNIEXPORT void JNICALL Java_com_pzhao_opensltest_MainActivity_createEngine
  (JNIEnv *env, jclass clazz, jobject jbuffer){
	mSlave=new NativeSlave(1024*1024);
	buffer = env->GetDirectBufferAddress(jbuffer);
	bufferSize=env->GetDirectBufferCapacity(jbuffer);
	firstFlag=true;
	mSlave->createEngine();
}

JNIEXPORT jboolean JNICALL Java_com_pzhao_opensltest_MainActivity_createAudioPlayer
  (JNIEnv *env, jclass clazz){
	mSlave->createAudioPlayer(mSlave);
}

JNIEXPORT void JNICALL Java_com_pzhao_opensltest_MainActivity_setPlayingUriAudioPlayer
  (JNIEnv *env, jclass clazz, jboolean isPlaying){
	mSlave->setPlayAudioPlayer(isPlaying);
}

JNIEXPORT void JNICALL Java_com_pzhao_opensltest_MainActivity_setMuteUriAudioPlayer
  (JNIEnv *env, jclass clazz, jboolean isMute){
	mSlave->setMuteUriAudioPlayer(isMute);
}

JNIEXPORT jboolean JNICALL Java_com_pzhao_opensltest_MainActivity_checkWrite
  (JNIEnv *env, jclass clazz){
	return mSlave->mBuffer->getWriteSpace()>=8192;
}

JNIEXPORT void JNICALL Java_com_pzhao_opensltest_MainActivity_write
  (JNIEnv *env, jclass clazz){
	int hasW=mSlave->mBuffer->Write((char*)buffer,bufferSize);
	if(firstFlag&&mSlave->mBuffer->getReadSpace()>=8192*2){
		firstFlag=false;
	mSlave->startPlay();
	}
}

JNIEXPORT void JNICALL Java_com_pzhao_opensltest_MainActivity_shutdown
  (JNIEnv *env, jclass clazz){
if(mSlave!=NULL)
	delete mSlave;
}
