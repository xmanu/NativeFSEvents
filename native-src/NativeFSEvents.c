//
//  NativeFSEvents.c
//  
//
//  Created by Jonatan Kronqvist on 16.12.2011.
//  Copyright 2011 n/a. All rights reserved.
//

#include <stdio.h>
#include "FSEventHandler.h"
#include "org_vaadin_jonatan_nativefsevents_NativeFSEvents.h"

//#define DEBUG

#ifdef DEBUG
#define LOG(...) fprintf(stderr, __VA_ARGS__)
#else
#define LOG(...)
#endif

CFStringRef to_cfstring(JNIEnv *env, jstring path);
char *to_cstring(CFStringRef string);
void fs_callback(ConstFSEventStreamRef streamRef,
                 void *clientCallBackInfo,
                 size_t numEvents,
                 void *eventPaths,
                 const FSEventStreamEventFlags eventFlags[],
                 const FSEventStreamEventId eventIds[]);
void install_monitor(CFArrayRef paths);


static JavaVM *jvm;
static jclass gClass = NULL;


void fs_callback(ConstFSEventStreamRef streamRef,
                 void *clientCallBackInfo,
                 size_t numEvents,
                 void *eventPaths,
                 const FSEventStreamEventFlags eventFlags[],
                 const FSEventStreamEventId eventIds[])
{
    int i;
    char **paths = eventPaths;
    
    JNIEnv *env = NULL;
    
    LOG("fs_callback()\n");

    (*jvm)->AttachCurrentThread(jvm, (void **)&env, NULL);
    jmethodID java_callback = (*env)->GetStaticMethodID(env, gClass, "eventCallback", "(Ljava/lang/String;Z)V");
    for (i=0; i<numEvents; i++) {
        jstring path = (*env)->NewStringUTF(env, paths[i]);
        jboolean own = (eventFlags[i] & kFSEventStreamEventFlagOwnEvent) ? true : false;
        (*env)->CallStaticVoidMethod(env, gClass, java_callback, path, own);
    }
    (*jvm)->DetachCurrentThread(jvm);
}

static CFMutableArrayRef monitored_paths;
static volatile CFRunLoopRef monitor_runloop;
static dispatch_queue_t install_queue;
static dispatch_queue_t monitor_queue;
static dispatch_semaphore_t monitor_semaphore;
static dispatch_semaphore_t monitored_paths_semaphore;
Boolean trackFiles;
Boolean ignoreSelf;

CFStringRef to_cfstring(JNIEnv *env, jstring path) {
    const char *cpath = (*env)->GetStringUTFChars(env, path, NULL);
    return CFStringCreateWithCString(kCFAllocatorDefault, cpath, kCFStringEncodingUTF8);
}

char *to_cstring(CFStringRef string) {
    CFIndex length = CFStringGetLength(string);
    char *buffer = malloc(length + 1);
    
    if (CFStringGetCString(string, buffer, length + 1, kCFStringEncodingUTF8)) {
        return buffer;
    } else {
        free(buffer);
        return NULL;
    }
}

static int counter = 0;

void install_monitor(CFArrayRef paths)
{
    const int mycounter = ++counter;
    LOG("install_monitor(%i)\n", mycounter);
    
    CFRetain(paths);
    
    dispatch_async(install_queue, ^{
        LOG("install_monitor(%i): install_queue\n", mycounter);
        
        if (dispatch_semaphore_wait(monitor_semaphore, DISPATCH_TIME_FOREVER) == 0) {
            LOG("install_monitor(%i): monitor_semaphore\n", mycounter);
            
            if (monitor_runloop != NULL) {
                LOG("install_monitor(%i): stopping existing runloop\n", mycounter);
                CFRunLoopStop(monitor_runloop);
                monitor_runloop = NULL;
            }
            
            dispatch_async(monitor_queue, ^{
                LOG("install_monitor(%i): monitor_queue\n", mycounter);
                
                if (mycounter != counter) {
                    LOG("install_monitor(%i): exiting as have been superceded\n", mycounter);
                    dispatch_semaphore_signal(monitor_semaphore);
                    CFRelease(paths);
                    return;
                }
                
                if (monitor_runloop != NULL) {
                    LOG("install_monitor(%i): not expecting a runloop to exist here!\n", mycounter);
                }
                
                FSEventStreamRef streamRef = monitor_paths(paths, trackFiles, ignoreSelf, &fs_callback);
                CFRelease(paths);
                
                monitor_runloop = CFRunLoopGetCurrent();
                LOG("install_monitor(%i): starting runloop\n", mycounter);
                
                dispatch_semaphore_signal(monitor_semaphore);
                
                CFRunLoopRun();
                
                LOG("install_monitor(%i): finished runloop\n", mycounter);
                unmonitor(streamRef);
            });
        } else {
            LOG("install_monitor(%i): failed to wait for semaphore\n", mycounter);
        }
    });
    
    
//    while (monitor_runloop == NULL) {
//        sleep(1);
//    }
}

JNIEXPORT void JNICALL Java_org_vaadin_jonatan_nativefsevents_NativeFSEvents_monitor(JNIEnv *env, jclass class, jstring path)
{
    CFStringRef pathString = to_cfstring(env, path);
    
    LOG("monitor(%s)\n", to_cstring(pathString));
    
    if (dispatch_semaphore_wait(monitored_paths_semaphore, DISPATCH_TIME_FOREVER) == 0) {
        CFArrayAppendValue(monitored_paths, pathString);
        CFArrayRef paths = CFArrayCreateCopy(NULL, monitored_paths);
        
        dispatch_semaphore_signal(monitored_paths_semaphore);
        
        install_monitor(paths);
        
        CFRelease(paths);
    } else {
        LOG("monitor(%s): failed to wait for semaphore\n", to_cstring(pathString));
    }
    
    CFRelease(pathString);
}

JNIEXPORT void JNICALL Java_org_vaadin_jonatan_nativefsevents_NativeFSEvents_unmonitor(JNIEnv *env, jclass class, jstring path)
{
    LOG("unmonitor()\n");
    if (dispatch_semaphore_wait(monitor_semaphore, DISPATCH_TIME_FOREVER) == 0) {
        LOG("umonitor(): checking runloop\n");
        
        if (monitor_runloop != NULL) {
            LOG("unmonitor(): stopping runloop\n");
            CFRunLoopStop(monitor_runloop);
            monitor_runloop = NULL;
        }
        dispatch_semaphore_signal(monitor_semaphore);
        
        if (dispatch_semaphore_wait(monitored_paths_semaphore, DISPATCH_TIME_FOREVER) == 0) {
            LOG("unmonitor(): removing path\n");
            
            /* Remove path from monitored_paths */
            CFStringRef cfpath = to_cfstring(env, path);
            for (int i=0; i<CFArrayGetCount(monitored_paths); i++) {
                CFStringRef str = CFArrayGetValueAtIndex(monitored_paths, i);
                if (CFStringCompare(str, cfpath, 0) == kCFCompareEqualTo) {
                    CFArrayRemoveValueAtIndex(monitored_paths, i);
                    break;
                }
            }
            CFRelease(cfpath);
            
            CFArrayRef paths = CFArrayCreateCopy(NULL, monitored_paths);
            
            dispatch_semaphore_signal(monitored_paths_semaphore);
            
            if (CFArrayGetCount(paths) > 0) {
                LOG("unmonitor(): installing with remaining %li paths\n", CFArrayGetCount(paths));
                
                // Reinstall the monitor without the unmonitored path.
                install_monitor(paths);
            } else {
                LOG("unmonitor(): no paths left\n");
            }
            
            CFRelease(paths);
        } else {
            LOG("unmonitor(): failed to wait for monitored_paths semaphore\n");
        }
    } else {
        LOG("unmonitor(): failed to wait for monitor semaphore\n");
    }
}

JNIEXPORT void JNICALL Java_org_vaadin_jonatan_nativefsevents_NativeFSEvents_monitorFiles(JNIEnv *env, jclass class, jboolean monitorFiles)
{
    trackFiles = monitorFiles;
    LOG("monitorFiles(%d)\n", trackFiles);
}

JNIEXPORT void JNICALL Java_org_vaadin_jonatan_nativefsevents_NativeFSEvents_ignoreSelf(JNIEnv *env, jclass class, jboolean ignSelf)
{
    ignoreSelf = ignSelf;
    LOG("ignoreSelf(%d)\n", ignoreSelf);
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
    LOG("JNI_OnLoad()\n");
    monitored_paths = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
    install_queue = dispatch_queue_create("jnifsevents-install", DISPATCH_QUEUE_SERIAL);
    monitor_queue = dispatch_queue_create("jnifsevents-monitor", DISPATCH_QUEUE_SERIAL);
    monitor_semaphore = dispatch_semaphore_create(1L);
    monitored_paths_semaphore = dispatch_semaphore_create(1L);
    trackFiles = false;
    ignoreSelf = true;
    
    jvm = vm;
    JNIEnv *env;
    (*jvm)->GetEnv(jvm, (void**)&env, JNI_VERSION_1_6);
    jclass class = (*env)->FindClass(env, "org/vaadin/jonatan/nativefsevents/NativeFSEvents");
    gClass = (*env)->NewGlobalRef(env, class);
    (*env)->DeleteLocalRef(env, class);
    return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved) {
    LOG("JNI_OnUnload()\n");
    CFRelease(monitored_paths);
    dispatch_release(install_queue);
    dispatch_release(monitor_queue);
    dispatch_release(monitor_semaphore);
}
