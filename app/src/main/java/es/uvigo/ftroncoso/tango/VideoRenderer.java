package es.uvigo.ftroncoso.tango;

import android.opengl.GLSurfaceView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

class VideoRenderer implements GLSurfaceView.Renderer {

    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        TangoNative.onSurfaceCreated();
    }

    public void onSurfaceChanged(GL10 gl, int width, int height) {
        TangoNative.onSurfaceChanged(width, height);
    }

    public void onDrawFrame(GL10 gl) {
        TangoNative.onDrawFrame();
    }
}
