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

std::string to_string(JNIEnv *env, jstring jstr) {
    const char *cstr = env->GetStringUTFChars(jstr, NULL);
    const auto str = std::string(cstr);
    env->ReleaseStringUTFChars(jstr, cstr);
    return str;
}

JNIFUNC(void, onCreate)(JNIEnv *env, jobject, jobject activity) {
    app.onCreate(env, activity);
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

JNIFUNC(void, writeCalData)(JNIEnv *env, jobject, jstring savePath) {
    app.writeCalData(to_string(env, savePath));
}

JNIFUNC(void, start)(JNIEnv *env, jobject, jboolean rgb, jstring savePath) {
    app.start(rgb, to_string(env, savePath));
}

JNIFUNC(void, stop)(JNIEnv *env, jobject) {
    app.stop();
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

    constexpr int N = 8;
    jdouble poseArray[N];
    poseArray[0] = pose->timestamp;
    for (int i = 0; i < 3; i++) poseArray[i + 1] = pose->translation[i];
    for (int i = 0; i < 4; i++) poseArray[i + 4] = pose->orientation[i];
    auto values = env->NewDoubleArray(N);
    env->SetDoubleArrayRegion(values, 0, N, poseArray);
    env->CallVoidMethod(jobj, jmid, values);
    env->DeleteLocalRef(values);
}

#ifdef __cplusplus
}
#endif
