//
//  FSEventHandler.c
//  jnifsevents
//
//  Created by Jonatan Kronqvist on 15.12.2011.
//  Copyright 2011 n/a. All rights reserved.
//

#include <stdio.h>
#include "FSEventHandler.h"

FSEventStreamRef monitor_paths(CFArrayRef paths, FSEventStreamCallback callback)
{
    void *callbackInfo = NULL; // could put stream-specific data here.
    FSEventStreamRef stream;
    CFAbsoluteTime latency = 0.2; /* Latency in seconds */
    
    /* Create the stream, passing in a callback */
    stream = FSEventStreamCreate(NULL,
                                 callback,
                                 callbackInfo,
                                 paths,
                                 kFSEventStreamEventIdSinceNow, /* Or a previous event ID */
                                 latency,
                                 kFSEventStreamCreateFlagNoDefer | kFSEventStreamCreateFlagIgnoreSelf /* Flags explained in reference */
                                 );
    
    FSEventStreamScheduleWithRunLoop(stream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    FSEventStreamStart(stream);
    return stream;
}

void unmonitor(FSEventStreamRef stream)
{
    FSEventStreamStop(stream);
    FSEventStreamUnscheduleFromRunLoop(stream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    FSEventStreamInvalidate(stream);
    FSEventStreamRelease(stream);
}
