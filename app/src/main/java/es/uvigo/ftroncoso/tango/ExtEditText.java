package es.uvigo.ftroncoso.tango;

import android.content.Context;
import android.support.v7.widget.AppCompatEditText;
import android.util.AttributeSet;
import android.view.KeyEvent;

public class ExtEditText extends AppCompatEditText {

    public ExtEditText(Context context) {
        super(context);
    }

    public ExtEditText(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public ExtEditText(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    @Override
    public boolean onKeyPreIme(int keyCode, KeyEvent event) {
        if (event.getKeyCode() == KeyEvent.KEYCODE_BACK) {
            clearFocus();
            return true;
        }
        return super.dispatchKeyEvent(event);
    }
}
