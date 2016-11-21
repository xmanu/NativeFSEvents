package demoapp;

import org.vaadin.jonatan.nativefsevents.*;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.util.Random;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.Executor;
import java.util.concurrent.ScheduledThreadPoolExecutor;

/**
 * Created by manuel on 17.11.16.
 */
public class DemoApp {

    public static void main(String[] args) throws IOException, InterruptedException {
        System.out.println(Paths.get("").toAbsolutePath().toString());
        NativeFSEvents nativeEvents = new NativeFSEvents("/", new NativeFSEvents.NativeFSEventListener() {
            @Override
            public void pathModified(String path) {
                System.out.println(path);
            }
        });
        nativeEvents.startMonitoring();
        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        try {
            Files.write(Paths.get("test.txt"), "Test".getBytes());
        } catch (IOException e) {
            e.printStackTrace();
        }
        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        try {
            Files.write(Paths.get("test.txt"), "Test2".getBytes());
        } catch (IOException e) {
            e.printStackTrace();
        }

        ProcessBuilder pb = new ProcessBuilder();
        Process p = pb.command("touch", "test2.txt").start();
        p.waitFor();


        //Executor ex = new ScheduledThreadPoolExecutor(2);

        //for (int i = 0; i < 4; ++i) // create and start threads
        //    ex.execute(new Thread(new Worker(i)));

        try {
            Thread.sleep(10000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        System.out.println("Done");
        nativeEvents.stopMonitoring();
    }
}

class Worker implements Runnable {
    final int num;

    Worker(int num) {
        this.num = num;
    }

    public void run() {
        doWork();
    }

    void doWork() {
        try {
            Random r = new Random();
            String testString = ("Test" + r.nextInt());
            System.out.println(testString);
            Files.write(Paths.get("test"+num+".txt"), testString.getBytes());
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}