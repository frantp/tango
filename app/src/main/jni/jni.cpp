#include <jni.h>
#include <string>

#include "TangoApp.h"

static ftp::TangoApp app;

#ifdef __cplusplus
extern "C" {
#endif

#define JNIFUNC(name) \
    JNIEXPORT void JNICALL \
    Java_es_uvigo_ftroncoso_tango_TangoNative_ ## name

JNIFUNC(onCreate)(JNIEnv *env, jobject, jobject activity, jstring savePath) {
    const char *save_path_str = env->GetStringUTFChars(savePath, NULL);
    const auto save_path = std::string(save_path_str);
    env->ReleaseStringUTFChars(savePath, save_path_str);
    app.OnCreate(env, activity, save_path);
}

JNIFUNC(onTangoServiceConnected)(JNIEnv *env, jobject, jobject binder) {
    app.OnTangoServiceConnected(env, binder);
}

JNIFUNC(onPause)(JNIEnv *env, jobject) {
    app.OnPause();
}

JNIFUNC(onSurfaceCreated)(JNIEnv *env, jobject) {
    app.OnSurfaceCreated();
}

JNIFUNC(onSurfaceChanged)(JNIEnv *env, jobject, jint width, jint height) {
    app.OnSurfaceChanged(width, height);
}

JNIFUNC(onDrawFrame)(JNIEnv *env, jobject) {
    app.OnDrawFrame();
}

JNIFUNC(onDisplayChanged)(JNIEnv *env, jobject, jint display_rotation) {
    app.OnDisplayChanged(display_rotation);
}

JNIFUNC(setRecord)(JNIEnv *env, jobject, jboolean record) {
    app.SetRecord(record);
}

#ifdef __cplusplus
}
#endif

