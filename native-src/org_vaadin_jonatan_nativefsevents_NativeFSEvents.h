/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class org_vaadin_jonatan_nativefsevents_NativeFSEvents */

#ifndef _Included_org_vaadin_jonatan_nativefsevents_NativeFSEvents
#define _Included_org_vaadin_jonatan_nativefsevents_NativeFSEvents
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     org_vaadin_jonatan_nativefsevents_NativeFSEvents
 * Method:    monitor
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_org_vaadin_jonatan_nativefsevents_NativeFSEvents_monitor
  (JNIEnv *, jclass, jstring);

/*
 * Class:     org_vaadin_jonatan_nativefsevents_NativeFSEvents
 * Method:    unmonitor
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_org_vaadin_jonatan_nativefsevents_NativeFSEvents_unmonitor
  (JNIEnv *, jclass, jstring);

/*
 * Class:     org_vaadin_jonatan_nativefsevents_NativeFSEvents
 * Method:    monitorFiles
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL Java_org_vaadin_jonatan_nativefsevents_NativeFSEvents_monitorFiles
  (JNIEnv *, jclass, jboolean);

/*
 * Class:     org_vaadin_jonatan_nativefsevents_NativeFSEvents
 * Method:    ignoreSelf
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL Java_org_vaadin_jonatan_nativefsevents_NativeFSEvents_ignoreSelf
  (JNIEnv *, jclass, jboolean);

#ifdef __cplusplus
}
#endif
#endif
