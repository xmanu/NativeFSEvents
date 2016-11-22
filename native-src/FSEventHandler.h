//
//  FSEventHandler.h
//  jnifsevents
//
//  Created by Jonatan Kronqvist on 15.12.2011.
//  Copyright 2011 n/a. All rights reserved.
//

#ifndef jnifsevents_FSEventHandler_h
#define jnifsevents_FSEventHandler_h

#include <CoreServices/CoreServices.h>

FSEventStreamRef monitor_paths(CFArrayRef paths, Boolean trackFiles, Boolean ignoreSelf, FSEventStreamCallback callback);
void unmonitor(FSEventStreamRef stream);

#endif
