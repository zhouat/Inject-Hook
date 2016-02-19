package com.xposedhook;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.os.Bundle;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.View;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.TextView;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Objects;

import de.robv.android.xposed.IXposedHookLoadPackage;
import de.robv.android.xposed.XC_MethodHook;
import de.robv.android.xposed.XposedBridge;
import de.robv.android.xposed.XposedHelpers;
import de.robv.android.xposed.callbacks.XC_LoadPackage;

import static de.robv.android.xposed.XposedHelpers.findAndHookMethod;

public class MyXposedModule implements IXposedHookLoadPackage {


    Class string_to_class(String type){

        switch(type){
            case "byte":    return byte.class;
            case "short":   return short.class;
            case "int":     return int.class;
            case "long":    return long.class;
            case "float":   return float.class;
            case "double":  return double.class;
            case "String":  return String.class;
            case "Map":     return Map.class;
            case "boolean": return boolean.class;
            case "char":    return char.class;

            default:        return null;
        }

    }

    XC_MethodHook callback_fun=new XC_MethodHook() {
        @Override
        protected void beforeHookedMethod(MethodHookParam param) throws Throwable {
            // this will be called before the clock was updated by the original methodt

            for (int i = 0; i < param.args.length; i++) {
                XposedBridge.log(" zhouat arg[" + i + "] is:" + param.args[i]);
            }

            Exception e = new Exception("this is a log");
            e.printStackTrace();
        }

        @Override
        protected void afterHookedMethod(MethodHookParam param) throws Throwable {
            // this will be called after the clock was updated by the original method
            XposedBridge.log(" zhouat ret is:" + param.getResult());

        }
    };

    @Override
    public void handleLoadPackage(XC_LoadPackage.LoadPackageParam lpparam) throws Throwable {

        if(lpparam.appInfo == null || (lpparam.appInfo.flags & (ApplicationInfo.FLAG_SYSTEM | ApplicationInfo.FLAG_UPDATED_SYSTEM_APP)) !=0){
            return;
        }

        String path = "/sdcard/config.txt";
        String package_name=null;
        String class_name=null;
        String function_name=null;
        String args_type=null;

        Object[] args_obj=null;

        File file = new File(path);
        if (file.isDirectory())
        {
            Log.d("hack", "File [/sdcard/config.txt] not exist");
        }
        else
        {
            try {
                InputStream instream = new FileInputStream(file);
                if (instream != null)
                {
                    InputStreamReader inputreader = new InputStreamReader(instream);
                    BufferedReader buffreader = new BufferedReader(inputreader);
                    String line;
                    while (( line = buffreader.readLine()) != null) {
                        if(line.indexOf("package_name")!=-1){
                            package_name=line.substring(line.indexOf(":")+1);
                            Log.e("hack","package_name:"+package_name);
                        }else if(line.indexOf("class_name")!=-1){
                            class_name=line.substring(line.indexOf(":")+1);
                            Log.e("hack","class_name:"+class_name);
                        }else if(line.indexOf("function_name")!=-1){
                            function_name=line.substring(line.indexOf(":")+1);
                            Log.e("hack","function_name:"+function_name);
                        }else if(line.indexOf("args_type")!=-1){
                            args_type=line.substring(line.indexOf(":") + 1);
                            Log.e("hack", "args_type:" + args_type);
                        }
                    }
                    instream.close();
                }
            }
            catch (Exception e)
            {
                Log.d("hack", e.getMessage());
            }
        }


        /**
         * String split to array
         * array content mapping to class object
         * class object save to args_obj
         * args_obj last must be XC_MethodHook instance
         */

        int pos=0;

        if(args_type!=null&&args_type.length()!=0){

            String args_array[]=args_type.split(",");

            args_obj=new Object[args_array.length+1];

            /*
            * method args string -> object
            * */

            for (String tmp:args_array) {
                if(string_to_class(tmp)!=null){
                    args_obj[pos]=string_to_class(tmp);
                }else{
                    args_obj[pos]=lpparam.classLoader.loadClass(tmp);
                }
                pos++;
            }
        }else{
            args_obj=new Object[1];
        }

        /*
        * callback function
        * */
        args_obj[pos]=callback_fun;

        if (!lpparam.packageName.equals(package_name))
            return;

        XposedBridge.log("com.target hook successfully.....");

        findAndHookMethod(class_name, lpparam.classLoader, function_name,args_obj);
    }
}