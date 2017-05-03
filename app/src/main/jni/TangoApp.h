#pragma once

#include <jni.h>
#include <string>

#include <tango_client_api.h>
#include <tango-gl/video_overlay.h>

namespace ftp {
    class TangoApp {
    public:
        void onCreate(JNIEnv *env, jobject caller_activity);

        void onTangoServiceConnected(JNIEnv *env, jobject binder);

        void onPause();

        void onSurfaceCreated();

        void onSurfaceChanged(int width, int height);

        void onDrawFrame();

        void onFrameAvailable(const TangoImageBuffer *buffer);

        void onDisplayChanged(int display_rotation);

        TangoPoseData getPose(double timestamp = 0);

        void writeCalData(const std::string &filename);

        inline void start(bool rgb, const std::string &dirname) {
            rgb_ = rgb;
            save_path_ = dirname;
        }

        inline void stop() {
            save_path_ = "";
        }

    private:
        TangoConfig tango_config_;
        tango_gl::VideoOverlay *video_overlay_drawable_;
        bool is_service_connected_;
        bool is_texture_id_set_;
        bool is_video_overlay_rotation_set_;
        bool rgb_;
        std::string save_path_;
        TangoSupportRotation display_rotation_;

        void deleteDrawables();
    };
}  // namespace ftp
