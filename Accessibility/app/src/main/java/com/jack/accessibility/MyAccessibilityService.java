package com.jack.accessibility;

import java.util.List;

import android.accessibilityservice.AccessibilityService;
import android.accessibilityservice.AccessibilityServiceInfo;
import android.annotation.SuppressLint;
import android.util.Log;
import android.view.KeyEvent;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;


@SuppressLint("NewApi")
public class MyAccessibilityService extends AccessibilityService {

    public static int INVOKE_TYPE = 0;
    public static final int TYPE_KILL_APP = 1;
    public static final int TYPE_INSTALL_APP = 2;
    public static final int TYPE_UNINSTALL_APP = 3;

    public static void reset() {
        INVOKE_TYPE = 0;
    }
    
//    @Override
//    protected void onServiceConnected() {
//        AccessibilityServiceInfo accessibilityServiceInfo = new AccessibilityServiceInfo();
//        // 设置相应应用的包名，默认是监听全部应用
//         accessibilityServiceInfo.packageNames = new String[]{"com.android.packageinstaller", "com.android.settings"};
//        // 设置监听的事件类型
//        accessibilityServiceInfo.eventTypes = AccessibilityEvent.TYPES_ALL_MASK;
//        // 设置反馈方式
//        accessibilityServiceInfo.feedbackType = AccessibilityServiceInfo.FEEDBACK_HAPTIC;
//        // 设置监听间隔时间
//        accessibilityServiceInfo.notificationTimeout = 100;
//        setServiceInfo(accessibilityServiceInfo);
//    }    
    
    
    @Override
    public void onAccessibilityEvent(AccessibilityEvent event) {
        processAccessibilityEnvent(event);
    }

    private void processAccessibilityEnvent(AccessibilityEvent event) {
        if (getRootInActiveWindow() == null) {
            Log.d("test", "AccessibilityNodeInfo = null");
        } else {
            switch (INVOKE_TYPE) {
            case TYPE_KILL_APP:
                processKillApplication(event);
                break;
            case TYPE_INSTALL_APP:
                processinstallApplication(event);
                break;
            case TYPE_UNINSTALL_APP:
                processUninstallApplication(event);
                break;
            default:
                break;
            }
        }
    }

    @Override
    protected boolean onKeyEvent(KeyEvent event) {
        return true;

    }

    @Override
    public void onInterrupt() {
    }

    private void processUninstallApplication(AccessibilityEvent event) {

        if (matchingApplication(event)) {
            findAndPerformAction("确定", getRootInActiveWindow());
        }

    }

    
    private void processinstallApplication(AccessibilityEvent event) {
        if (event == null || event.getPackageName() == null || !matchingApplication(event)) {
            return;
        }
        

        traverseNode(getRootInActiveWindow());

        if (getRootInActiveWindow() != null) {
            findAndPerformAction("安装", getRootInActiveWindow());

            findAndPerformAction("下一步", getRootInActiveWindow());
            
            findAndPerformAction("打开", getRootInActiveWindow());            
        }

        Log.d("test", "--------------------------------------------------------------------------------");
    }


    private void traverseNode(AccessibilityNodeInfo node) {
        if (null == node)
            return;

        final int count = node.getChildCount();
        if (count > 0) {
            for (int i = 0; i < count; i++) {
                AccessibilityNodeInfo childNode = node.getChild(i);
                traverseNode(childNode);
            }
        } else {
            CharSequence text = node.getText();
            Log.d("test", "Node text = " + text);
        }

    }

    private void processKillApplication(AccessibilityEvent event) {

        if (event.getPackageName().equals("com.android.settings")) {
            
            findAndPerformAction("强行停止", getRootInActiveWindow());
            
            findAndPerformAction("确定", getRootInActiveWindow());
        }
    }
    
    private void findAndPerformAction(String text, AccessibilityNodeInfo source) {
        if (source == null) {
            return;
        }
        
        List<AccessibilityNodeInfo> nodes = source.findAccessibilityNodeInfosByText(text);
        if (nodes != null && !nodes.isEmpty()) {
            AccessibilityNodeInfo node;
            for (int i = 0; i < nodes.size(); i++) {
                node = nodes.get(i);
                performActionClick(node);
            }
        }
    }
    
    
    private void performActionClick(AccessibilityNodeInfo node) {
        if (node == null) {
            return;
        }

        if (isButton(node) || isTextView(node) || isView(node)) {
            node.performAction(AccessibilityNodeInfo.ACTION_CLICK);
        }
    }    
    
    private boolean isButton(AccessibilityNodeInfo node) {
        return node.getClassName().equals("android.widget.Button") && node.isEnabled();
    }
    
    private boolean isTextView(AccessibilityNodeInfo node) {
        return node.getClassName().equals("android.widget.TextView") && node.isEnabled();
    }
    
    private boolean isView(AccessibilityNodeInfo node) {
        return node.getClassName().equals("android.widget.View") && node.isEnabled();
    }
    
    private boolean matchingApplication(AccessibilityEvent event) {
        if (event == null) {
            return false;
        }
        
        if ("com.android.packageinstaller".equals(event.getPackageName())
                || "com.lenovo.safecenter".equals(event.getPackageName())) {
            
            return true;
        }
        
        return false;
    }
}
