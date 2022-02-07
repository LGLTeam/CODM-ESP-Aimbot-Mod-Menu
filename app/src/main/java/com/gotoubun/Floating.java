package com.gotoubun;

import android.annotation.SuppressLint;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.ColorStateList;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.PixelFormat;
import android.graphics.Point;
import android.graphics.Typeface;
import android.graphics.drawable.GradientDrawable;
import android.os.Build;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.util.Base64;
import android.util.TypedValue;
import android.view.Display;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.HorizontalScrollView;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.RelativeLayout;
import android.widget.ScrollView;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.Switch;
import android.widget.TextView;


import android.view.ViewConfiguration;
import android.app.AlertDialog;
import android.content.DialogInterface;
import java.util.Random;
import android.widget.CheckBox;
import android.widget.Toast;
import java.util.Map;

public class Floating extends Service {

    static {
        System.loadLibrary("nino");
    }

    WindowManager windowManager;

    int screenWidth, screenHeight, type, screenDpi;
    float density;


    WindowManager.LayoutParams iconLayoutParams, mainLayoutParams, canvasLayoutParams;
    RelativeLayout iconLayout;
    LinearLayout mainLayout;
    CanvasView canvasLayout;

    RelativeLayout closeLayout, maximizeLayout, minimizeLayout;
    RelativeLayout.LayoutParams closeLayoutParams, maximizeLayoutParams, minimizeLayoutParams;
	
    ImageView iconImg;

    SharedPreferences configPrefs;
    SharedPreferences.Editor configEditor;

    String[] listTab = new String[]{"Player Visual", "Aim Menu", "Settings"};
    LinearLayout[] pageLayouts = new LinearLayout[listTab.length];
    int lastSelectedPage = 0;
	
	String M_RAND_TITLE = "-";
	
    
    long sleepTime = 4000 / 60;
	TextView textTitle;
    boolean isMaximized = false;
    int lastMaximizedX = 0, lastMaximizedY = 0;
    int lastMaximizedW = 0, lastMaximizedH = 0;

    int layoutWidth = 100;
    int layoutHeight = 200;
    int iconSize = 40;
    int menuButtonSize = 30;
	int tabWidth = 150;
    int tabHeight = 50;

	
	//private native void onSendConfig(String s, long v);
	
   private native void onSendConfig(String s, String v);
//public static native void DrawOn(ESPView espView, Canvas canvas);
	
    public static native void onCanvasDraw(Canvas canvas, int w, int h, float d);
    
	static native void Switch(int i, boolean jboolean1);
	
	//ESPView canvasLayout;
	
    void CreateCanvas() {
		
		int LAYOUT_FLAG;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            LAYOUT_FLAG = WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY;
        } else {
            LAYOUT_FLAG = WindowManager.LayoutParams.TYPE_SYSTEM_OVERLAY;
        }
        final WindowManager.LayoutParams canvasLayoutParams = new WindowManager.LayoutParams(
            WindowManager.LayoutParams.MATCH_PARENT,
            WindowManager.LayoutParams.MATCH_PARENT, 0, getNavigationBarHeight(),
            LAYOUT_FLAG,
            WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
            | WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE | WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN | WindowManager.LayoutParams.FLAG_FULLSCREEN,
            PixelFormat.RGBA_8888);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P)
            canvasLayoutParams.layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;

        canvasLayoutParams.gravity = Gravity.TOP | Gravity.START;        //Initially view will be added to top-left corner
        canvasLayoutParams.x = 0;
        canvasLayoutParams.y = 0;
        windowManager = (WindowManager) getSystemService(WINDOW_SERVICE);
		canvasLayout = new CanvasView(this);
		windowManager.addView(canvasLayout, canvasLayoutParams);
		
    }
	private int getNavigationBarHeight() {
        boolean hasMenuKey = ViewConfiguration.get(this).hasPermanentMenuKey();
        int resourceId = getResources().getIdentifier("navigation_bar_height", "dimen", "android");
        if (resourceId > 0 && !hasMenuKey) {
            return getResources().getDimensionPixelSize(resourceId);
        }
        return 0;
    }
    private class CanvasView extends View {
        public CanvasView(Context context) {
            super(context);
        }
		
        @Override
        protected void onDraw(Canvas canvas) {
            try {
                onCanvasDraw(canvas, screenWidth, screenHeight, density);
            } catch (Exception ex) {
                ex.printStackTrace();
            }
        }
    }

	private void UpdateConfiguration(String s, Object v) {
        try {
            onSendConfig(s, v.toString());
            SharedPreferences.Editor configEditor = configPrefs.edit();
            configEditor.putString(s, v.toString());
            configEditor.apply();
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }
    

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (mUpdateCanvas.isAlive()) {
            mUpdateCanvas.interrupt();
        }
        if (mUpdateThread.isAlive()) {
            mUpdateThread.interrupt();
        }
        if (iconLayout != null) {
            windowManager.removeView(iconLayout);
        }
        if (mainLayout != null) {
            windowManager.removeView(mainLayout);
        }
        if (canvasLayout != null) {
            windowManager.removeView(canvasLayout);
        }
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        super.onStartCommand(intent, flags, startId);
        return START_NOT_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();	
		    configPrefs = getSharedPreferences("config", MODE_PRIVATE);
          //  configEditor = configPrefs.edit();
            windowManager = (WindowManager) getSystemService(WINDOW_SERVICE);
            Point screenSize = new Point();
            Display display = windowManager.getDefaultDisplay();
            display.getRealSize(screenSize);
            screenWidth = screenSize.x;
            screenHeight = screenSize.y;
            screenDpi = getResources().getDisplayMetrics().densityDpi;
            density = getResources().getDisplayMetrics().density;
            layoutWidth = convertSizeToDp(400);
            layoutHeight = convertSizeToDp(225);
            iconSize = convertSizeToDp(40);
            menuButtonSize = convertSizeToDp(30);
            tabWidth = convertSizeToDp(0);
            tabHeight = convertSizeToDp(0);
		    
		
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                type = 2038;
            } else {
                type = 2002;
            }
            CreateIcon();
            CreateLayout();
            CreateCanvas();
            mUpdateThread.start();
            mUpdateCanvas.start();
        }
	

    void AddFeatures() {    
        AddText(0, "Menu ESP :", 10, Typeface.BOLD, "#FFFFFFFF");
		AddSwitch(0, "ESP Player Line", false, new CompoundButton.OnCheckedChangeListener() {
				@Override
				public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
					UpdateConfiguration("ESP::LINE", isChecked ? 1 : 0);
				}
			});
		
        AddSwitch(0, "ESP Player Box", false, new CompoundButton.OnCheckedChangeListener() {
				@Override
				public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
					UpdateConfiguration("ESP::BOX", isChecked ? 1 : 0);
				}
			});

        AddSwitch(0, "ESP Player Health", false, new CompoundButton.OnCheckedChangeListener() {
				@Override
				public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
					UpdateConfiguration("ESP::HEALTH", isChecked ? 1 : 0);
				}
			});

        AddSwitch(0, "ESP Player Name", false, new CompoundButton.OnCheckedChangeListener() {
				@Override
				public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
					UpdateConfiguration("ESP::NAME", isChecked ? 1 : 0);
				}
			});

        AddSwitch(0, "ESP Player Distance", false, new CompoundButton.OnCheckedChangeListener() {
				@Override
				public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
					UpdateConfiguration("ESP::DISTANCE", isChecked ? 1 : 0);
				}
			});
			   AddText(0, "AIM Menu :", 13, Typeface.BOLD, "#FFFFFFFF");
        AddSwitch(0, "Aim Bot", false, new CompoundButton.OnCheckedChangeListener() {
				@Override
				public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
					UpdateConfiguration("AIM::AIMBOT", isChecked ? 1 : 0);
				}
			});

        AddText2(0, "Location : ", 5.0f, Color.BLACK);
        AddRadioButton(0, new String[]{"Head", "Chest", "Body"}, 0, new RadioGroup.OnCheckedChangeListener() {
				@Override
				public void onCheckedChanged(RadioGroup group, int checkedId) {
					UpdateConfiguration("AIM::LOCATION", checkedId);
				}
			});

        AddText2(0, "Target : ", 5.0f, Color.BLACK);
        AddRadioButton(0, new String[]{"Closest To Distance", "Inside POV"}, 0, new RadioGroup.OnCheckedChangeListener() {
				@Override
				public void onCheckedChanged(RadioGroup group, int checkedId) {
					UpdateConfiguration("AIM::TARGET", checkedId);
				}
			});
        AddSeekbar(0, "Size POV", 0, 500, 0, "", "", new SeekBar.OnSeekBarChangeListener() {
				@Override
				public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
					UpdateConfiguration("AIM::SIZE", (progress));
				}

				@Override
				public void onStartTrackingTouch(SeekBar seekBar) {

				}

				@Override
				public void onStopTrackingTouch(SeekBar seekBar) {

				}
			});
        AddText2(0, "Trigger : ", 5.0f, Color.BLACK);
        AddRadioButton(0, new String[]{"None", "Shooting", "Scoping"}, 0, new RadioGroup.OnCheckedChangeListener() {
				@Override
				public void onCheckedChanged(RadioGroup group, int checkedId) {
					UpdateConfiguration("AIM::TRIGGER", checkedId);
				}
			});
		
        
        
        AddText(0, "", 14, Typeface.BOLD, "#FFFFFFFF");
        AddText(0, "Main Settings :", 10, Typeface.BOLD, "#FFFFFFFF");
		
        AddSeekbar(0, "Menu Opacity", 1, 100, 100, "", "%", new SeekBar.OnSeekBarChangeListener() {
				@Override
				public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
					mainLayout.setAlpha((float) progress / 100.f);
				}

				@Override
				public void onStartTrackingTouch(SeekBar seekBar) {

				}

				@Override
				public void onStopTrackingTouch(SeekBar seekBar) {
				}
			});

        AddSeekbar(0, "Icon Size", 50, 200, 100, "", "%", new SeekBar.OnSeekBarChangeListener() {
				@Override
				public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
					ViewGroup.LayoutParams iconParams = iconImg.getLayoutParams();
					iconParams.width = (int) ((float) iconSize * ((float) progress / 100.f));
					iconParams.height = (int) ((float) iconSize * ((float) progress / 100.f));
					iconImg.setLayoutParams(iconParams);
				}

				@Override
				public void onStartTrackingTouch(SeekBar seekBar) {
					iconLayout.setVisibility(View.VISIBLE);
				}

				@Override
				public void onStopTrackingTouch(SeekBar seekBar) {
					iconLayout.setVisibility(View.GONE);
				}
			});
        AddSeekbar(0, "Icon Opacity", 0, 100, 100, "", "%", new SeekBar.OnSeekBarChangeListener() {
				@Override
				public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
					iconLayout.setAlpha((float) progress / 100.f);
				}

				@Override
				public void onStartTrackingTouch(SeekBar seekBar) {
					iconLayout.setVisibility(View.VISIBLE);
				}

				@Override
				public void onStopTrackingTouch(SeekBar seekBar) {
					iconLayout.setVisibility(View.GONE);
				}
			});
    }
	
    @SuppressLint("ClickableViewAccessibility")
    void CreateLayout() {
        mainLayoutParams = new WindowManager.LayoutParams(layoutWidth, layoutHeight, type, WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE | WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS | WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS, PixelFormat.RGBA_8888);

        mainLayoutParams.x = 0;
        mainLayoutParams.y = 0;
        mainLayoutParams.gravity = Gravity.START | Gravity.TOP;

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            mainLayoutParams.layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
        }

        mainLayout = new LinearLayout(this);
        mainLayout.setOrientation(LinearLayout.VERTICAL);
        mainLayout.setLayoutParams(new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));

        GradientDrawable mainLayoutBg = new GradientDrawable();
        //mainLayoutBg.setColor(Color.parseColor("#A39FC4"));
        mainLayoutBg.setColor(Color.parseColor("#D5151416"));
        mainLayout.setBackground(mainLayoutBg);

        RelativeLayout headerLayout = new RelativeLayout(this);
        headerLayout.setLayoutParams(new RelativeLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, menuButtonSize + convertSizeToDp(4)));
        headerLayout.setClickable(true);
        headerLayout.setFocusable(true);
        headerLayout.setFocusableInTouchMode(true);
        //headerLayout.setBackgroundColor(Color.argb(255, 13, 13, 13));
        //headerLayout.setBackgroundColor(Color.parseColor("#D5151416"));
        headerLayout.setPadding(10, 5, 10, 5);
        headerLayout.setGravity(Gravity.CENTER_HORIZONTAL);
        headerLayout.setBackgroundColor(Color.BLACK);
        mainLayout.addView(headerLayout);

        /*TextView textTitle = new TextView(this);
        textTitle.setLayoutParams(new ViewGroup.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.MATCH_PARENT));
        textTitle.setGravity(Gravity.CENTER);
        textTitle.setClickable(true);
        textTitle.setFocusable(true);
        textTitle.setFocusableInTouchMode(true);
		textTitle.setText(Title());
		textTitle.setTypeface(Typeface.createFromAsset(getAssets(),"fonts/fox.ttf"));
        textTitle.setTextSize(convertSizeToDp(11.f));
        textTitle.setTextColor(Color.BLACK);
        headerLayout.addView(textTitle);*/
		
		textTitle = new TextView(this);
        textTitle.setTextSize(16);
        textTitle.setGravity(Gravity.CENTER_HORIZONTAL);
		textTitle.setText(Title());
		//textTitle.setTypeface(Typeface.DEFAULT_BOLD);
        textTitle.setTypeface(Typeface.createFromAsset(getAssets(), "fonts/fox.ttf"));
        textTitle.setTextColor(Color.WHITE);
        headerLayout.addView(textTitle);
        
        textTitle.setOnClickListener(new View.OnClickListener() {
				@Override
				public void onClick(View v) {
					mainLayout.setVisibility(View.GONE);
					iconLayout.setVisibility(View.VISIBLE);
				}
			});
			
        View.OnTouchListener onTitleListener = new View.OnTouchListener() {
            float pressedX;
            float pressedY;
            float deltaX;
            float deltaY;
            float newX;
            float newY;
            float maxX;
            float maxY;

            @Override
            public boolean onTouch(View v, MotionEvent event) {
                switch (event.getActionMasked()) {
                    case MotionEvent.ACTION_DOWN:

                        deltaX = mainLayoutParams.x - event.getRawX();
                        deltaY = mainLayoutParams.y - event.getRawY();

                        pressedX = event.getRawX();
                        pressedY = event.getRawY();

                        break;
                    case MotionEvent.ACTION_MOVE:
                        newX = event.getRawX() + deltaX;
                        newY = event.getRawY() + deltaY;

                        maxX = screenWidth - mainLayout.getWidth();
                        maxY = screenHeight - mainLayout.getHeight();

                        if (newX < 0)
                            newX = 0;
                        if (newX > maxX)
                            newX = (int) maxX;
                        if (newY < 0)
                            newY = 0;
                        if (newY > maxY)
                            newY = (int) maxY;

                        mainLayoutParams.x = (int) newX;
                        mainLayoutParams.y = (int) newY;
                        windowManager.updateViewLayout(mainLayout, mainLayoutParams);

                        break;

                    default:
                        break;
                }

                return false;
            }
        };

        headerLayout.setOnTouchListener(onTitleListener);
        textTitle.setOnTouchListener(onTitleListener);

        LinearLayout tabLayout = new LinearLayout(this);
        tabLayout.setLayoutParams(new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
        tabLayout.setOrientation(LinearLayout.HORIZONTAL);

        HorizontalScrollView tabScrollView = new HorizontalScrollView(this);
        tabScrollView.setLayoutParams(new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
        tabScrollView.setBackgroundColor(Color.argb(255, 230, 230, 230));

        tabScrollView.addView(tabLayout);

        mainLayout.addView(tabScrollView);

        final RelativeLayout[] tabButtons = new RelativeLayout[listTab.length];
        for (int i = 0; i < tabButtons.length; i++) {
            pageLayouts[i] = new LinearLayout(this);
            pageLayouts[i].setLayoutParams(new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
            pageLayouts[i].setOrientation(LinearLayout.VERTICAL);

            ScrollView scrollView = new ScrollView(this);
            scrollView.setLayoutParams(new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
            scrollView.addView(pageLayouts[i]);

            tabButtons[i] = new RelativeLayout(this);
            tabButtons[i].setLayoutParams(new RelativeLayout.LayoutParams(tabWidth, tabHeight));
            if (i != 0) {
                tabButtons[i].setBackgroundColor(Color.argb(255, 230, 230, 230));
                pageLayouts[i].setVisibility(View.GONE);
            } else {
                tabButtons[i].setBackgroundColor(Color.WHITE);
                pageLayouts[i].setVisibility(View.VISIBLE);
            }

            TextView tabText = new TextView(this);
            tabText.setLayoutParams(new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));
            tabText.setGravity(Gravity.CENTER);
            tabText.setText(listTab[i]);
            tabText.setTextSize(convertSizeToDp(11.f));
            tabText.setTextColor(Color.BLACK);
            tabButtons[i].addView(tabText);

            final int curTab = i;
            tabButtons[i].setOnClickListener(new View.OnClickListener() {
					@Override
					public void onClick(View v) {
						if (curTab != lastSelectedPage) {
							tabButtons[curTab].setBackgroundColor(Color.WHITE);
							pageLayouts[curTab].setVisibility(View.VISIBLE);

							pageLayouts[lastSelectedPage].setVisibility(View.GONE);
							lastSelectedPage = curTab;

							for (int j = 0; j < tabButtons.length; j++) {
								if (j != curTab) {
									tabButtons[j].setBackgroundColor(Color.argb(255, 230, 230, 230));
								}
							}
						}
					}
				});

            tabLayout.addView(tabButtons[i]);
            mainLayout.addView(scrollView);
        }

        windowManager.addView(mainLayout, mainLayoutParams);

        AddFeatures();
}

    @SuppressLint("ClickableViewAccessibility")
    void CreateIcon() {
        iconLayout = new RelativeLayout(this);
        RelativeLayout.LayoutParams iconParams = new RelativeLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        iconLayout.setLayoutParams(iconParams);

        iconImg = new ImageView(this);
        ViewGroup.LayoutParams iconImgParams = new ViewGroup.LayoutParams(iconSize, iconSize);

        iconImg.setLayoutParams(iconImgParams);

        iconLayout.addView(iconImg);

        try {
            String iconBase64 = "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";
            byte[] iconData = Base64.decode(iconBase64, Base64.DEFAULT);

            Bitmap bmp = BitmapFactory.decodeByteArray(iconData, 0, iconData.length);
            iconImg.setImageBitmap(bmp);
        } catch (Exception ex) {
            ex.printStackTrace();
        }

        iconLayoutParams = new WindowManager.LayoutParams(WindowManager.LayoutParams.WRAP_CONTENT, WindowManager.LayoutParams.WRAP_CONTENT, type, WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE | WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS, PixelFormat.TRANSLUCENT);
        iconLayoutParams.gravity = Gravity.START | Gravity.TOP;

        iconLayoutParams.x = 0;
        iconLayoutParams.y = 0;

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            iconLayoutParams.layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
        }

        iconLayout.setVisibility(View.GONE);

        iconLayout.setOnTouchListener(new View.OnTouchListener() {
				float pressedX;
				float pressedY;
				float deltaX;
				float deltaY;
				float newX;
				float newY;

				@Override
				public boolean onTouch(View v, MotionEvent event) {

					switch (event.getActionMasked()) {
						case MotionEvent.ACTION_DOWN:

							deltaX = iconLayoutParams.x - event.getRawX();
							deltaY = iconLayoutParams.y - event.getRawY();

							pressedX = event.getRawX();
							pressedY = event.getRawY();

							break;
						case MotionEvent.ACTION_UP:
							int Xdiff = (int) (event.getRawX() - pressedX);
							int Ydiff = (int) (event.getRawY() - pressedY);

							if (Xdiff == 0 && Ydiff == 0) {
								mainLayout.setVisibility(View.VISIBLE);
								iconLayout.setVisibility(View.GONE);
							}
							return true;
						case MotionEvent.ACTION_MOVE:
							newX = event.getRawX() + deltaX;
							newY = event.getRawY() + deltaY;

							float maxX = screenWidth - v.getWidth();
							float maxY = screenHeight - v.getHeight();

							if (newX < 0)
								newX = 0;
							if (newX > maxX)
								newX = (int) maxX;
							if (newY < 0)
								newY = 0;
							if (newY > maxY)
								newY = (int) maxY;

							iconLayoutParams.x = (int) newX;
							iconLayoutParams.y = (int) newY;

							windowManager.updateViewLayout(iconLayout, iconLayoutParams);
							break;

						default:
							break;
					}

					return false;
				}
			});

        windowManager.addView(iconLayout, iconLayoutParams);
    }

    LinearLayout CreateHolder(Object data) {
        RelativeLayout parentHolder = new RelativeLayout(this);
        parentHolder.setLayoutParams(new RelativeLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
        LinearLayout childHolder = new LinearLayout(this);
        childHolder.setLayoutParams(new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
        childHolder.setOrientation(LinearLayout.HORIZONTAL);
        parentHolder.addView(childHolder);

        if (data instanceof Integer)
            pageLayouts[(Integer) data].addView(parentHolder);
        else if (data instanceof ViewGroup)
            ((ViewGroup) data).addView(parentHolder);

        return childHolder;
    }
	void AddText(Object data, String text, int size, int typeface, String color) {
        TextView textView = new TextView(this);
        textView.setText(text);
        textView.setTextColor(Color.parseColor(color));
        textView.setTypeface(null, typeface);
        textView.setPadding(convertSizeToDp(5), convertSizeToDp(5), convertSizeToDp(5), convertSizeToDp(5));
        textView.setTextSize(TypedValue.COMPLEX_UNIT_DIP, size);
        textView.setLayoutParams(new LinearLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT));

        if (data instanceof ViewGroup)
            ((ViewGroup) data).addView(textView);
        else if (data instanceof Integer)
            pageLayouts[(int) data].addView(textView);
    }
    void AddText2(Object data, String text, float size, int color) {
        TextView textView = new TextView(this);
        textView.setText(text);
        textView.setTextColor(Color.WHITE);
        textView.setPadding(15, 15, 15, 15);
        textView.setTextSize(convertSizeToDp(size));
        textView.setLayoutParams(new LinearLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT));

        if (data instanceof Integer)
            pageLayouts[(Integer) data].addView(textView);
        else if (data instanceof ViewGroup)
            ((ViewGroup) data).addView(textView);
    }
	void AddCheckbox(Object data, String text, boolean checked, CompoundButton.OnCheckedChangeListener listener) {
        CheckBox checkBox = new CheckBox(this);
        checkBox.setText(text);
        checkBox.setTextSize(TypedValue.COMPLEX_UNIT_DIP, 11.f);
        checkBox.setTextColor(Color.BLACK);
        checkBox.setChecked(checked);
        checkBox.setOnCheckedChangeListener(listener);
        checkBox.setLayoutParams(new LinearLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT));

        if (Build.VERSION.SDK_INT >= 21) {
            ColorStateList colorStateList = new ColorStateList(
				new int[][]{
					new int[]{-android.R.attr.state_checked}, // unchecked
					new int[]{android.R.attr.state_checked}  // checked
				},
				new int[]{
					Color.BLACK,
					Color.BLACK
				}
            );
            checkBox.setButtonTintList(colorStateList);
        }
        if (data instanceof ViewGroup)
            ((ViewGroup) data).addView(checkBox);
        else if (data instanceof Integer)
            pageLayouts[(int) data].addView(checkBox);
    }
	
    void AddCenteredText(Object data, String text, int size, int typeface, String color) {
        TextView textView = new TextView(this);
        textView.setText(text);
        textView.setTextColor(Color.parseColor(color));
        textView.setTypeface(null, typeface);
        textView.setPadding(15, 15, 15, 15);
        textView.setTextSize(convertSizeToDp(size));
        textView.setLayoutParams(new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
        textView.setGravity(Gravity.CENTER);

        if (data instanceof Integer)
            pageLayouts[(Integer) data].addView(textView);
        else if (data instanceof ViewGroup)
            ((ViewGroup) data).addView(textView);
    }

    void AddHeader(Object data, String text) {
        LinearLayout headerLayout = new LinearLayout(this);
        headerLayout.setLayoutParams(new WindowManager.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
        headerLayout.setOrientation(LinearLayout.VERTICAL);
        headerLayout.setBackgroundColor(Color.argb(255, 10, 10, 10));

        TextView textView = new TextView(this);
        textView.setText(text);
        textView.setTextColor(Color.WHITE);
        textView.setTypeface(Typeface.DEFAULT_BOLD);
        textView.setPadding(10, 10, 10, 10);
        textView.setTextSize(convertSizeToDp(7.5f));
        textView.setLayoutParams(new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
        headerLayout.addView(textView);

        if (data instanceof Integer)
            pageLayouts[(Integer) data].addView(headerLayout);
        else if (data instanceof ViewGroup)
            ((ViewGroup) data).addView(headerLayout);
    }

    void AddSwitch(Object data, String text, boolean checked, CompoundButton.OnCheckedChangeListener listener) {
        Switch toggle = new Switch(this);
        toggle.setText(text);
        toggle.setTextSize(convertSizeToDp(5.f));
        toggle.setTextColor(Color.WHITE);
        toggle.setChecked(checked);
        toggle.setPadding(15, 15, 15, 15);
        toggle.setOnCheckedChangeListener(listener);
        toggle.setLayoutParams(new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));

        if (Build.VERSION.SDK_INT >= 21) {
            ColorStateList colorStateList = new ColorStateList(
                    new int[][]{
                            new int[]{-android.R.attr.state_checked}, // unchecked
                            new int[]{android.R.attr.state_checked}  // checked
                    },
                    new int[]{
                            Color.BLACK,
                            Color.BLACK
                    }
            );
            toggle.setButtonTintList(colorStateList);
        }

        if (data instanceof Integer)
            pageLayouts[(Integer) data].addView(toggle);
        else if (data instanceof ViewGroup)
            ((ViewGroup) data).addView(toggle);
    }
	void AddFloatSeekbar(Object data, String text, int min, int max, int value, final String prefix, final String suffix, final SeekBar.OnSeekBarChangeListener listener) {
        LinearLayout linearLayout = new LinearLayout(this);
        linearLayout.setLayoutParams(new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
        linearLayout.setOrientation(LinearLayout.HORIZONTAL);

        TextView textV = new TextView(this);
        textV.setText(text + ":");
        textV.setTextSize(TypedValue.COMPLEX_UNIT_DIP, 11.f);
        textV.setPadding(convertSizeToDp(10), convertSizeToDp(5), convertSizeToDp(10), convertSizeToDp(5));
        textV.setTextColor(Color.BLACK);
        textV.setLayoutParams(new RelativeLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT));
        textV.setGravity(Gravity.LEFT);

        SeekBar seekBar = new SeekBar(this);
        seekBar.setMax(max);
        if (Build.VERSION.SDK_INT >= 26) {
            seekBar.setMin(min);
            seekBar.setProgress(min);
        }
        if (Build.VERSION.SDK_INT >= 21) {
            seekBar.setThumbTintList(ColorStateList.valueOf(Color.BLACK));
            seekBar.setProgressTintList(ColorStateList.valueOf(Color.BLACK));
        }
        seekBar.setPadding(convertSizeToDp(15), convertSizeToDp(5), convertSizeToDp(15), convertSizeToDp(5));

        final TextView textValue = new TextView(this);
        textValue.setText(prefix + String.valueOf((float) min / 10) + suffix);
        textValue.setGravity(Gravity.RIGHT);
        textValue.setTextSize(TypedValue.COMPLEX_UNIT_DIP, 11.f);
        textValue.setLayoutParams(new RelativeLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
        textValue.setPadding(convertSizeToDp(15), convertSizeToDp(5), convertSizeToDp(15), convertSizeToDp(5));
        textValue.setTextColor(Color.BLACK);

        if (value != 0) {
            if (value < min)
                value = min;
            if (value > max)
                value = max;

            textValue.setText(prefix + (float) value / max + suffix);
            seekBar.setProgress(value);
        }

        final int minimValue = min;
        seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
				@Override
				public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
					if (progress < minimValue) {
						progress = minimValue;
						seekBar.setProgress(progress);
					}

					if (listener != null) listener.onProgressChanged(seekBar, progress, fromUser);
					textValue.setText(prefix + String.valueOf((float) progress / 10) + suffix);
				}

				@Override
				public void onStartTrackingTouch(SeekBar seekBar) {
					if (listener != null) listener.onStartTrackingTouch(seekBar);
				}

				@Override
				public void onStopTrackingTouch(SeekBar seekBar) {
					if (listener != null) listener.onStopTrackingTouch(seekBar);
				}
			});

        linearLayout.addView(textV);
        linearLayout.addView(textValue);

        if (data instanceof ViewGroup) {
            ((ViewGroup) data).addView(linearLayout);
            ((ViewGroup) data).addView(seekBar);
        } else if (data instanceof Integer) {
            pageLayouts[(int) data].addView(linearLayout);
            pageLayouts[(int) data].addView(seekBar);
        }
    }
	
    void AddSeekbar(Object data, String text, int min, int max, int value, final String prefix, final String suffix, final SeekBar.OnSeekBarChangeListener listener) {
        LinearLayout linearLayout = new LinearLayout(this);
        linearLayout.setLayoutParams(new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
        linearLayout.setOrientation(LinearLayout.HORIZONTAL);

        TextView textV = new TextView(this);
        textV.setText(text + ":");
        textV.setTextSize(convertSizeToDp(6.f));
        textV.setPadding(15, 15, 15, 15);
        textV.setTextColor(Color.WHITE);
        textV.setLayoutParams(new RelativeLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT));
        textV.setGravity(Gravity.LEFT);

        SeekBar seekBar = new SeekBar(this);
        seekBar.setMax(max);
        if (Build.VERSION.SDK_INT >= 26) {
            seekBar.setMin(min);
            seekBar.setProgress(min);
        }
        if (Build.VERSION.SDK_INT >= 21) {
            seekBar.setThumbTintList(ColorStateList.valueOf(Color.WHITE));
            seekBar.setProgressTintList(ColorStateList.valueOf(Color.argb(255, 110, 110, 110)));
        }
        seekBar.setPadding(20, 15, 20, 15);

        final TextView textValue = new TextView(this);
        textValue.setText(prefix + min + suffix);
        textValue.setGravity(Gravity.RIGHT);
        textValue.setTextSize(convertSizeToDp(6.f));
        textValue.setLayoutParams(new RelativeLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
        textValue.setPadding(20, 15, 20, 15);
        textValue.setTextColor(Color.WHITE);

        final int minimValue = min;
        seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (progress < minimValue) {
                    progress = minimValue;
                    seekBar.setProgress(progress);
                }

                if (listener != null) listener.onProgressChanged(seekBar, progress, fromUser);
                textValue.setText(prefix + progress + suffix);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                if (listener != null) listener.onStartTrackingTouch(seekBar);
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                if (listener != null) listener.onStopTrackingTouch(seekBar);
            }
        });

        if (value != 0) {
            if (value < min)
                value = min;
            if (value > max)
                value = max;

            textValue.setText(prefix + value + suffix);
            seekBar.setProgress(value);
        }

        linearLayout.addView(textV);
        linearLayout.addView(textValue);

        if (data instanceof Integer) {
            pageLayouts[(Integer) data].addView(linearLayout);
            pageLayouts[(Integer) data].addView(seekBar);
        } else if (data instanceof ViewGroup) {
            ((ViewGroup) data).addView(linearLayout);
            ((ViewGroup) data).addView(seekBar);
        }
    }

    void AddRadioButton(Object data, String[] list, int defaultCheckedId, RadioGroup.OnCheckedChangeListener listener) {
        RadioGroup rg = new RadioGroup(this);
        RadioButton[] rb = new RadioButton[list.length];
        rg.setOrientation(RadioGroup.VERTICAL);
        for (int i = 0; i < list.length; i++) {
            rb[i] = new RadioButton(this);
            if (i == defaultCheckedId) rb[i].setChecked(true);
            rb[i].setPadding(15, 15, 15, 15);
            rb[i].setText(list[i]);
            rb[i].setTextSize(convertSizeToDp(6.f));
            rb[i].setId(i);
            rb[i].setGravity(Gravity.RIGHT);
            rb[i].setTextColor(Color.WHITE);
            rb[i].setButtonTintList(ColorStateList.valueOf(Color.WHITE));

            rg.addView(rb[i]);
        }
        rg.setOnCheckedChangeListener(listener);
        RelativeLayout.LayoutParams toggleP = new RelativeLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        rg.setLayoutParams(toggleP);

        if (data instanceof Integer)
            pageLayouts[(Integer) data].addView(rg);
        else if (data instanceof ViewGroup)
            ((ViewGroup) data).addView(rg);
    }

    void AddDropdown(Object data, String[] list, AdapterView.OnItemSelectedListener listener) {
        LinearLayout holderLayout = new LinearLayout(this);
        holderLayout.setOrientation(LinearLayout.VERTICAL);
        holderLayout.setLayoutParams(new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
        holderLayout.setPadding(15, 15, 15, 15);
        holderLayout.setGravity(Gravity.CENTER);

        Spinner sp = new Spinner(this, Spinner.MODE_DROPDOWN);

        GradientDrawable drawable = new GradientDrawable();
        drawable.setColor(Color.argb(255, 233, 233, 233));
        drawable.setStroke(1, Color.BLACK);
        sp.setPopupBackgroundDrawable(drawable);
        sp.setBackground(drawable);

        sp.setLayoutParams(new RelativeLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
        ArrayAdapter<String> dataAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, list) {
            @Override
            public View getView(int position, View convertView, ViewGroup parent) {
                View v = super.getView(position, convertView, parent);

                ((TextView) v).setTextColor(Color.WHITE);
                ((TextView) v).setTypeface(null, Typeface.BOLD);
                ((TextView) v).setGravity(Gravity.CENTER);

                return v;
            }

            @Override
            public View getDropDownView(int position, View convertView, ViewGroup parent) {
                View v = super.getDropDownView(position, convertView, parent);

                ((TextView) v).setTextColor(Color.WHITE);
                ((TextView) v).setTypeface(null, Typeface.BOLD);
                ((TextView) v).setGravity(Gravity.CENTER);

                return v;
            }
        };
        dataAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        sp.setAdapter(dataAdapter);
        sp.setOnItemSelectedListener(listener);
        sp.setPadding(0, 5, 0, 5);

        holderLayout.addView(sp);

        if (data instanceof Integer)
            pageLayouts[(Integer) data].addView(holderLayout);
        else if (data instanceof ViewGroup)
            ((ViewGroup) data).addView(holderLayout);
    }

    void AddButton(Object data, String text, int width, int height, int padding, View.OnClickListener listener) {
        LinearLayout holderLayout = new LinearLayout(this);
        holderLayout.setOrientation(LinearLayout.VERTICAL);
        holderLayout.setLayoutParams(new LinearLayout.LayoutParams(width, height));
        holderLayout.setPadding(padding, padding, padding, padding);
        holderLayout.setGravity(Gravity.CENTER);

        Button btn = new Button(this);
        btn.setText(text);
        btn.setTextColor(Color.WHITE);
        btn.setOnClickListener(listener);
        btn.setLayoutParams(new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));

        GradientDrawable drawable = new GradientDrawable();
        drawable.setColor(Color.argb(255, 244, 244, 244));
        drawable.setStroke(2, Color.argb(255, 0, 0, 0));

        btn.setBackground(drawable);

        holderLayout.addView(btn);

        if (data instanceof Integer)
            pageLayouts[(Integer) data].addView(holderLayout);
        else if (data instanceof ViewGroup)
            ((ViewGroup) data).addView(holderLayout);
    }

    float convertSizeToDp(float size) {
        return TypedValue.applyDimension(
                TypedValue.COMPLEX_UNIT_DIP,
                size,
                getResources().getDisplayMetrics()
        );
    }

    int convertSizeToDp(int size) {
        return Math.round(TypedValue.applyDimension(
                TypedValue.COMPLEX_UNIT_DIP,
                size,
                getResources().getDisplayMetrics()
        ));
    }
	@SuppressLint("HandlerLeak")
    Handler handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            if (msg.what == 0) {
                try {
                    Point screenSize = new Point();
                    Display display = windowManager.getDefaultDisplay();
                    display.getRealSize(screenSize);

                    screenWidth = screenSize.x;
                    screenHeight = screenSize.y;

                    android.view.ViewGroup.LayoutParams LayoutParams = null;
					windowManager.updateViewLayout(canvasLayout, LayoutParams);

                } catch (Exception ex) {
                    ex.printStackTrace();
                }
            }
            if (msg.what == 1) {
                Random random = new Random();
				int randIdx = random.nextInt(Title().length());
                int randChoose = random.nextInt(M_RAND_TITLE.length());
                char[] newTitle =Title().toCharArray();
                newTitle[randIdx] = M_RAND_TITLE.charAt(randChoose);
                textTitle.setText(String.valueOf(newTitle));
            }
        }
    };

	
 
	
	native String Title();
	
    Thread mUpdateCanvas = new Thread() {
        @Override
        public void run() {
            android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_DISPLAY);
            while (isAlive() && !isInterrupted()) {
                try {
                    long t1 = System.currentTimeMillis();
                    canvasLayout.postInvalidate();
                    long td = System.currentTimeMillis() - t1;
                    Thread.sleep(Math.max(Math.min(0, sleepTime - td), sleepTime));
                } catch (Exception ex) {
                    ex.printStackTrace();
                }
            }
        }
    };

    Thread mUpdateThread = new Thread() {
        @Override
        public void run() {
            android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_DISPLAY);
            while (isAlive() && !isInterrupted()) {
                try {
                    long t1 = System.currentTimeMillis();
                    Point screenSize = new Point();
                    Display display = windowManager.getDefaultDisplay();
                    display.getRealSize(screenSize);

                    if (screenWidth != screenSize.x || screenHeight != screenSize.y) {
                        handler.sendEmptyMessage(0);
                    }
                    handler.sendEmptyMessage(1);

                    long td = System.currentTimeMillis() - t1;
                    Thread.sleep(Math.max(Math.min(0, sleepTime - td), sleepTime));
                } catch (Exception ex) {
                    ex.printStackTrace();
                }
            }
        }
    };
}
