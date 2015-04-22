package com.pzhao.opensltest;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;

public class WriteThread extends Thread{
    public MainActivity mActivity;
    public String URI_PCM = "/storage/emulated/0/DCIM/hou.wav";  
    public BufferedInputStream bis;
    public boolean isRuning=true;
    public boolean startFlag=false;
public WriteThread(MainActivity activity){
    mActivity=activity;
    try {
        bis=new BufferedInputStream(new FileInputStream(new File(URI_PCM)));
    } catch (FileNotFoundException e) {
        // TODO Auto-generated catch block
        e.printStackTrace();
    }
}

public void run(){
    while(isRuning){
        if(startFlag&&MainActivity.checkWrite()){
            try {
                bis.read(mActivity.buffer.array());
               MainActivity.write();
            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }
    try {
        bis.close();
        MainActivity.shutdown();
    } catch (IOException e) {
        // TODO Auto-generated catch block
        e.printStackTrace();
    }
}


}
