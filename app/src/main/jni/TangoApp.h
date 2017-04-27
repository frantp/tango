#pragma once

#include <jni.h>
#include <string>

#include <tango_client_api.h>
#include <tango-gl/video_overlay.h>

namespace ftp {
    class TangoApp {
    public:
        void OnCreate(JNIEnv *env, jobject caller_activity, const std::string &save_path);

        void OnTangoServiceConnected(JNIEnv *env, jobject binder);

        void OnPause();

        void OnSurfaceCreated();

        void OnSurfaceChanged(int width, int height);

        void OnDrawFrame();

        void OnFrameAvailable(const TangoImageBuffer *buffer);

        void OnDisplayChanged(int display_rotation);

        void SetRecord(bool record) { record_ = record; }

    private:
        TangoConfig tango_config_;
        tango_gl::VideoOverlay *video_overlay_drawable_;
        bool is_service_connected_;
        bool is_texture_id_set_;
        bool is_video_overlay_rotation_set_;
        bool record_;
        std::string save_path_;
        TangoSupportRotation display_rotation_;

        void DeleteDrawables();
    };
}  // namespace ftp
