package com.jack.accessibility;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.provider.Settings;
import android.view.View;
import android.app.Activity;
import android.content.Intent;

public class MainActivity extends Activity {
    
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		this.findViewById(R.id.activeButton).setOnClickListener(new View.OnClickListener() {
			
			@Override
			public void onClick(View arg0) {
				Intent killIntent = new Intent(Settings.ACTION_ACCESSIBILITY_SETTINGS);
				startActivity(killIntent);
			}
		});
		
		this.findViewById(R.id.installButton).setOnClickListener(new View.OnClickListener() {
			
			@Override
			public void onClick(View arg0) {
				MyAccessibilityService.INVOKE_TYPE = MyAccessibilityService.TYPE_INSTALL_APP;
				String fileName = Environment.getExternalStorageDirectory() + "/test.apk"; 
				File installFile = new File(fileName);
				if(installFile.exists()){
					installFile.delete();
				}
				try {
					installFile.createNewFile();
					FileOutputStream out = new FileOutputStream(installFile);
					byte[] buffer = new byte[512];
					InputStream in = MainActivity.this.getAssets().open("test.apk");
					int count;
					while((count= in.read(buffer))!=-1){
						out.write(buffer, 0, count);
					}
					in.close();
					out.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
				Intent intent = new Intent(Intent.ACTION_VIEW); 
				intent.setDataAndType(Uri.fromFile(new File(fileName)), "application/vnd.android.package-archive"); 
				startActivity(intent);
				
			}
		});
		this.findViewById(R.id.uninstallButton).setOnClickListener(new View.OnClickListener() {
			
			@Override
			public void onClick(View arg0) {
				MyAccessibilityService.INVOKE_TYPE = MyAccessibilityService.TYPE_UNINSTALL_APP;
				Uri packageURI = Uri.parse("package:com.example.test"); 
				Intent uninstallIntent = new Intent(Intent.ACTION_DELETE, packageURI); 
				startActivity(uninstallIntent); 
			}
		});
		this.findViewById(R.id.killAppButton).setOnClickListener(new View.OnClickListener() {
			
			@Override
			public void onClick(View arg0) {
				MyAccessibilityService.INVOKE_TYPE = MyAccessibilityService.TYPE_KILL_APP;
				Intent killIntent = new Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
				Uri packageURI = Uri.parse("package:com.example.test"); 
				killIntent.setData(packageURI);
				startActivity(killIntent);
			}
		});
	}
}
