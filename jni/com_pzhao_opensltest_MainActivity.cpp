#define LOG_TAG "OpenSLTest"
#define DGB 1
#include <jni.h>
#include <pthread.h>
#include <unistd.h>
#include "log.h"

#include "com_pzhao_opensltest_MainActivity.h"
#include "NativeSlave.h"

void* buffer;
long bufferSize;
bool firstFlag;
NativeSlave *mSlave;

JavaVM *g_jvm = NULL;
jobject g_obj = NULL;

void *sendStartUdp(void *arg) {
	JNIEnv *env;
	jclass cls;
	jmethodID mid;
	ALOGD("pzhao-->sleep begin");
	usleep(300000);
	ALOGD("pzhao-->sleep end");
	if (g_jvm->AttachCurrentThread(&env, NULL) != JNI_OK) {
		ALOGD("pzhao-->AttachCurrentThread fail");
		return NULL;
	}
	cls = env->GetObjectClass(g_obj);
	if (cls == NULL) {
		ALOGD("pzhao-->find class error");
		goto error;
	}
	mid = env->GetMethodID(cls, "fromJni", "(I)V");
	if (mid == NULL) {
		ALOGD("pzhao-->find method error");
		goto error;
	}

	env->CallVoidMethod(g_obj, mid, 4);

	error: if (g_jvm->DetachCurrentThread() != JNI_OK)
		ALOGD("pzhao-->DetachCurrentThread fail");
	pthread_exit(0);
}

JNIEXPORT void JNICALL Java_com_pzhao_opensltest_MainActivity_setJniEnv(
		JNIEnv *env, jobject obj) {
	env->GetJavaVM(&g_jvm);
	g_obj = env->NewGlobalRef(obj);
	pthread_t pt;
	pthread_create(&pt, NULL, sendStartUdp, NULL);
//	jclass cls=env->GetObjectClass(obj);
//	jmethodID mid=env->GetMethodID(cls,"fromJni","(I)V");
//	env->CallVoidMethod(obj,mid,3);

}

JNIEXPORT void JNICALL Java_com_pzhao_opensltest_MainActivity_createEngine(
		JNIEnv *env, jclass clazz, jobject jbuffer) {
	mSlave = new NativeSlave(1024 * 1024);
	buffer = env->GetDirectBufferAddress(jbuffer);
	bufferSize = env->GetDirectBufferCapacity(jbuffer);
	firstFlag = true;
	mSlave->createEngine();

}

JNIEXPORT jboolean JNICALL Java_com_pzhao_opensltest_MainActivity_createAudioPlayer(
		JNIEnv *env, jclass clazz) {
	mSlave->createAudioPlayer(mSlave);
}

JNIEXPORT void JNICALL Java_com_pzhao_opensltest_MainActivity_setPlayingUriAudioPlayer(
		JNIEnv *env, jclass clazz, jboolean isPlaying) {
	mSlave->setPlayAudioPlayer(isPlaying);
}

JNIEXPORT void JNICALL Java_com_pzhao_opensltest_MainActivity_setMuteUriAudioPlayer(
		JNIEnv *env, jclass clazz, jboolean isMute) {
	mSlave->setMuteUriAudioPlayer(isMute);
}

JNIEXPORT jboolean JNICALL Java_com_pzhao_opensltest_MainActivity_checkWrite(
		JNIEnv *env, jclass clazz) {
	return mSlave->mBuffer->getWriteSpace() >= 8192;
}

JNIEXPORT void JNICALL Java_com_pzhao_opensltest_MainActivity_write(JNIEnv *env,
		jclass clazz) {
	int hasW = mSlave->mBuffer->Write((char*) buffer, bufferSize);
	if (firstFlag && mSlave->mBuffer->getReadSpace() >= 8192 * 2) {
		firstFlag = false;
		mSlave->startPlay();
	}
}

JNIEXPORT void JNICALL Java_com_pzhao_opensltest_MainActivity_shutdown(
		JNIEnv *env, jclass clazz) {
	if (mSlave != NULL)
		delete mSlave;
}
