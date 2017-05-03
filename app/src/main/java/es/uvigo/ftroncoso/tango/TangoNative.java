package es.uvigo.ftroncoso.tango;

import android.app.Activity;
import android.os.IBinder;
import android.util.Log;

import com.projecttango.examples.cpp.util.TangoInitializationHelper;

class TangoNative {
    static {
        if (TangoInitializationHelper.loadTangoSharedLibrary() ==
                TangoInitializationHelper.ARCH_ERROR) {
            Log.e("TangoNative", "ERROR! Unable to load libtango_client_api.so!");
        }
        System.loadLibrary("tangoc");
    }

    public static native void onCreate(Activity callerActivity);

    public static native void onTangoServiceConnected(IBinder binder);

    public static native void onPause();

    public static native void onSurfaceCreated();

    public static native void onSurfaceChanged(int width, int height);

    public static native void onDrawFrame();

    public static native void onDisplayChanged(int displayRotation);

    public static native void writeCalData(String filename);

    public static native void start(boolean rgb, String dirname);

    public static native void stop();

    public static native void registerCallback(Callback callback);

    interface Callback {
        void onPoseAvailable(double[] pose);
    }
}
