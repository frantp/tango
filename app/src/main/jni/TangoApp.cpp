#include <tango_support_api.h>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <sstream>
#include <glm/matrix.hpp>

#include "jni_interface.h"
#include "TangoApp.h"

namespace {
    constexpr int kTangoCoreMinimumVersion = 9377;

    void onFrameAvailableRouter(void *context, TangoCameraId, const TangoImageBuffer *buffer) {
        static_cast<ftp::TangoApp *>(context)->onFrameAvailable(buffer);
    }

    void onPoseAvailableRouter(void *context, const TangoPoseData *pose) {
        onPoseAvailable(pose);
    }
}  // namespace

namespace ftp {
    void TangoApp::onCreate(JNIEnv *env, jobject caller_activity) {
        int version = 0;
        const auto ret = TangoSupport_GetTangoVersion(env, caller_activity, &version);
        if (ret != TANGO_SUCCESS || version < kTangoCoreMinimumVersion) {
            LOGE("TangoApp::OnCreate, Tango Core version is out of date.");
            std::exit(EXIT_SUCCESS);
        }

        // Initialize variables
        is_service_connected_ = false;
        is_texture_id_set_ = false;
        video_overlay_drawable_ = NULL;
        is_video_overlay_rotation_set_ = false;
        rgb_ = false;
        save_path_ = "";
    }

    void TangoApp::onTangoServiceConnected(JNIEnv *env, jobject binder) {
        if (TangoService_setBinder(env, binder) != TANGO_SUCCESS) {
            LOGE("TangoApp::OnTangoServiceConnected, "
                         "TangoService_setBinder error");
            std::exit(EXIT_SUCCESS);
        }

        tango_config_ = TangoService_getConfig(TANGO_CONFIG_DEFAULT);
        if (tango_config_ == nullptr) {
            LOGE("TangoApp::OnTangoServiceConnected, "
                         "Failed to get default config form");
            std::exit(EXIT_SUCCESS);
        }

        int ret = TangoConfig_setBool(tango_config_, "config_enable_color_camera", true);
        if (ret != TANGO_SUCCESS) {
            LOGE("TangoApp::OnTangoServiceConnected, "
                         "config_enable_color_camera() failed with error code: %d", ret);
            std::exit(EXIT_SUCCESS);
        }

        ret = TangoService_connectOnFrameAvailable(TANGO_CAMERA_COLOR, this,
                                                   onFrameAvailableRouter);
        if (ret != TANGO_SUCCESS) {
            LOGE("TangoApp::OnTangoServiceConnected, "
                         "Error connecting color frame %d", ret);
            std::exit(EXIT_SUCCESS);
        }

        TangoCoordinateFramePair pair;
        pair.base = TANGO_COORDINATE_FRAME_START_OF_SERVICE;
        pair.target = TANGO_COORDINATE_FRAME_DEVICE;
        ret = TangoService_connectOnPoseAvailable(1, &pair, onPoseAvailableRouter);
        if (ret != TANGO_SUCCESS) {
            LOGE("TangoApp::OnTangoServiceConnected, "
                         "Error connecting pose %d", ret);
            std::exit(EXIT_SUCCESS);
        }

        // Connect to Tango Service
        ret = TangoService_connect(this, tango_config_);
        if (ret != TANGO_SUCCESS) {
            LOGE("TangoApp::OnTangoServiceConnected, "
                         "Failed to connect to the Tango service with error code: %d", ret);
            std::exit(EXIT_SUCCESS);
        }

        // Initialize TangoSupport context.
        TangoSupport_initializeLibrary();

        is_service_connected_ = true;
    }

    void TangoApp::onPause() {
        // Free TangoConfig structure
        if (tango_config_ != nullptr) {
            TangoConfig_free(tango_config_);
            tango_config_ = nullptr;
        }

        // Disconnect from the Tango service
        TangoService_disconnect();

        // Free buffer data
        save_path_ = "";
        rgb_ = false;
        is_service_connected_ = false;
        is_video_overlay_rotation_set_ = false;
        is_texture_id_set_ = false;
        this->deleteDrawables();
    }

    void TangoApp::onFrameAvailable(const TangoImageBuffer *buffer) {
        if (save_path_ == "") {
            return;
        }

        if (buffer->format != TANGO_HAL_PIXEL_FORMAT_YCrCb_420_SP) {
            LOGE("TangoApp::OnFrameAvailable, "
                         "Texture format is not supported by this app");
            return;
        }

        const auto w = buffer->width;
        const auto h = buffer->height;
        const auto data_size = rgb_ ? w * h * 3 / 2 : w * h;
        const auto ts = lround(buffer->timestamp * 1e6);

        // Get pose
        const auto pose = this->getPose(buffer->timestamp);

        // Get image
        std::ostringstream ss;
        ss << save_path_ << "/" << ts << (rgb_ ? ".yuv" : ".gray");
        const auto imgfile = ss.str();

        // Store pose
        const auto posefile = save_path_ + "/poses.txt";
        std::fstream poseout(posefile, std::ios_base::app);
        if (poseout.tellg() == 0)
            poseout << "# t x y z qw qx qy qz" << std::endl;
        poseout << ts;
        for (int i = 0; i < 3; i++) poseout << " " << pose.translation[i];
        for (int i = 0; i < 4; i++) poseout << " " << pose.orientation[i];
        poseout << std::endl;
        poseout.close();

        // Store image
        std::ofstream imgout(imgfile, std::ios_base::binary);
        imgout.write(reinterpret_cast<const char *>(buffer->data), data_size * sizeof(uint8_t));
        imgout.close();
    }

    void TangoApp::onSurfaceCreated() {
        if (video_overlay_drawable_ != NULL) {
            this->deleteDrawables();
        }
        video_overlay_drawable_ = new tango_gl::VideoOverlay(GL_TEXTURE_EXTERNAL_OES,
                                                             display_rotation_);
    }

    void TangoApp::onSurfaceChanged(int width, int height) {
        glViewport(0, 0, width, height);
    }

    void TangoApp::onDrawFrame() {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        if (!is_service_connected_) {
            return;
        }

        if (!is_texture_id_set_) {
            is_texture_id_set_ = true;
            const int texture_id = static_cast<int>(video_overlay_drawable_->GetTextureId());
            const auto ret = TangoService_connectTextureId(
                    TANGO_CAMERA_COLOR, static_cast<unsigned int>(texture_id), nullptr, nullptr);
            if (ret != TANGO_SUCCESS) {
                LOGE("TangoApp:OnDrawFrame, "
                             "Failed to connect the texture id with error code: %d", ret);
            }
        }

        if (!is_video_overlay_rotation_set_) {
            video_overlay_drawable_->SetDisplayRotation(display_rotation_);
            is_video_overlay_rotation_set_ = true;
        }

        double timestamp;
        const int ret = TangoService_updateTexture(TANGO_CAMERA_COLOR, &timestamp);
        if (ret != TANGO_SUCCESS) {
            LOGE("TangoApp: RenderTextureId, "
                         "Failed to update the texture id with error code: %d", ret);
        }
        video_overlay_drawable_->Render(glm::mat4(1.0f), glm::mat4(1.0f));
    }

    void TangoApp::onDisplayChanged(int display_rotation) {
        display_rotation_ = static_cast<TangoSupportRotation>(display_rotation);
        is_video_overlay_rotation_set_ = false;
    }

    void TangoApp::deleteDrawables() {
        delete video_overlay_drawable_;
        video_overlay_drawable_ = NULL;
    }

    TangoPoseData TangoApp::getPose(double timestamp) {
        TangoCoordinateFramePair pair;
        pair.base = TANGO_COORDINATE_FRAME_START_OF_SERVICE;
        pair.target = TANGO_COORDINATE_FRAME_DEVICE;
        TangoPoseData pose;
        TangoService_getPoseAtTime(timestamp, pair, &pose);
        return pose;
    }

    void TangoApp::writeCalData(const std::string &filename) {
        // Get info
        TangoCameraIntrinsics intrinsics;
        TangoService_getCameraIntrinsics(TANGO_CAMERA_COLOR, &intrinsics);
        TangoCoordinateFramePair pair;
        pair.base = TANGO_COORDINATE_FRAME_IMU;
        pair.target = TANGO_COORDINATE_FRAME_CAMERA_COLOR;
        TangoPoseData pose;
        TangoService_getPoseAtTime(0, pair, &pose);

        // Store info
        std::ofstream camout(filename);
        camout << intrinsics.width << " " << intrinsics.height << std::endl
               << intrinsics.fx << " " << intrinsics.fy << " "
               << intrinsics.cx << " " << intrinsics.cy << std::endl;
        for (int i = 0; i < 5; i++) camout << (i > 0 ? " " : "") << intrinsics.distortion[i];
        camout << std::endl;
        for (int i = 0; i < 3; i++) camout << (i > 0 ? " " : "") << pose.translation[i];
        for (int i = 0; i < 4; i++) camout << " " << pose.orientation[i];
        camout << std::endl;
        camout.close();
    }
}  // namespace ftp
