#include <jni.h>
#include <string>

#include "TangoApp.h"

static ftp::TangoApp app;

#ifdef __cplusplus
extern "C" {
#endif

#define TANGO_NATIVE_CLASS "es/uvigo/ftroncoso/tango/TangoNative"

#define JNIFUNC(type, name) \
    JNIEXPORT type JNICALL \
    Java_es_uvigo_ftroncoso_tango_TangoNative_ ## name

JNIFUNC(void, onCreate)(JNIEnv *env, jobject, jobject activity, jstring savePath) {
    const char *save_path_str = env->GetStringUTFChars(savePath, NULL);
    const auto save_path = std::string(save_path_str);
    env->ReleaseStringUTFChars(savePath, save_path_str);
    app.onCreate(env, activity, save_path);
}

JNIFUNC(void, onTangoServiceConnected)(JNIEnv *env, jobject, jobject binder) {
    app.onTangoServiceConnected(env, binder);
}

JNIFUNC(void, onPause)(JNIEnv *env, jobject) {
    app.onPause();
}

JNIFUNC(void, onSurfaceCreated)(JNIEnv *env, jobject) {
    app.onSurfaceCreated();
}

JNIFUNC(void, onSurfaceChanged)(JNIEnv *env, jobject, jint width, jint height) {
    app.onSurfaceChanged(width, height);
}

JNIFUNC(void, onDrawFrame)(JNIEnv *env, jobject) {
    app.onDrawFrame();
}

JNIFUNC(void, onDisplayChanged)(JNIEnv *env, jobject, jint display_rotation) {
    app.onDisplayChanged(display_rotation);
}

JNIFUNC(void, writeCalData)(JNIEnv *env, jobject, jboolean rgb) {
    app.writeCalData();
}

JNIFUNC(void, setRgb)(JNIEnv *env, jobject, jboolean rgb) {
    app.setRgb(rgb);
}

JNIFUNC(void, setRec)(JNIEnv *env, jobject, jboolean rec) {
    app.setRec(rec);
}

static JavaVM *jvm = nullptr;
static jobject jobj = nullptr;
static jmethodID jmid = nullptr;

JNIFUNC(void, registerCallback)(JNIEnv *env, jobject, jobject callback) {
    env->GetJavaVM(&jvm);
    jobj = env->NewGlobalRef(callback);
    if (jobj != nullptr) {
        const jclass cls = env->GetObjectClass(callback);
        jmid = env->GetMethodID(cls, "onPoseAvailable", "([D)V");
    }
}

void onPoseAvailable(const TangoPoseData *pose) {
    if (jobj == nullptr) {
        return;
    }

    JNIEnv *env;
    const int ret = jvm->GetEnv((void **) &env, JNI_VERSION_1_6);
    if (ret == JNI_EDETACHED) {
        if (jvm->AttachCurrentThread(&env, NULL) != JNI_OK) {
            return;
        }
    } else if (ret != JNI_OK) {
        return;
    }

    jdouble poseArray[7];
    for (int i = 0; i < 3; i++) poseArray[i] = pose->translation[i];
    for (int i = 0; i < 4; i++) poseArray[i + 3] = pose->orientation[i];
    auto values = env->NewDoubleArray(7);
    env->SetDoubleArrayRegion(values, 0, 7, poseArray);
    env->CallVoidMethod(jobj, jmid, values);
    env->DeleteLocalRef(values);
}

#ifdef __cplusplus
}
#endif
