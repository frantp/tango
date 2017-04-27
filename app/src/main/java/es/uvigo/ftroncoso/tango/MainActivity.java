package es.uvigo.ftroncoso.tango;

import android.content.ComponentName;
import android.content.ServiceConnection;
import android.hardware.display.DisplayManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Environment;
import android.os.IBinder;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.view.WindowManager;
import android.widget.ToggleButton;

import com.projecttango.examples.cpp.util.TangoInitializationHelper;

import java.io.File;

public class MainActivity extends AppCompatActivity {
    private GLSurfaceView mSurfaceView;
    private ToggleButton mRecordSwitcher;
    private File mOutFolder;

    private ServiceConnection mTangoServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder binder) {
            TangoNative.onTangoServiceConnected(binder);
            setDisplayRotation();
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            // Handle this if you need to gracefully shutdown/retry
            // in the event that Tango itself crashes/gets upgraded while running.
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);

        mOutFolder = new File(Environment.getExternalStorageDirectory(), "tango_tests");
        TangoNative.onCreate(this, mOutFolder.getPath());

        // Register for display orientation change updates.
        DisplayManager displayManager = (DisplayManager) getSystemService(DISPLAY_SERVICE);
        if (displayManager != null) {
            displayManager.registerDisplayListener(new DisplayManager.DisplayListener() {
                @Override
                public void onDisplayAdded(int displayId) {
                }

                @Override
                public void onDisplayChanged(int displayId) {
                    synchronized (this) {
                        setDisplayRotation();
                    }
                }

                @Override
                public void onDisplayRemoved(int displayId) {
                }
            }, null);
        }

        mSurfaceView = (GLSurfaceView) findViewById(R.id.surfaceview);
        mSurfaceView.setEGLContextClientVersion(2);
        mSurfaceView.setRenderer(new VideoRenderer());
        mRecordSwitcher = (ToggleButton) findViewById(R.id.rec_switcher);
    }

    @Override
    protected void onResume() {
        super.onResume();
        mSurfaceView.onResume();
        TangoInitializationHelper.bindTangoService(this, mTangoServiceConnection);
    }

    @Override
    protected void onPause() {
        super.onPause();
        mRecordSwitcher.setChecked(false);
        mSurfaceView.onPause();
        TangoNative.onPause();
        unbindService(mTangoServiceConnection);
    }

    public void recordClicked(View view) {
        mOutFolder.mkdirs();
        TangoNative.setRecord(mRecordSwitcher.isChecked());
    }

    private void setDisplayRotation() {
        TangoNative.onDisplayChanged(getWindowManager().getDefaultDisplay().getRotation());
    }
}
