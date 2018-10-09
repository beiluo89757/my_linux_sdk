#ifndef __Common_h__
#define __Common_h__


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <curl/curl.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>       
#include <pthread.h>

#ifdef __APPLE__
#include <dispatch/dispatch.h>
#else
#include <semaphore.h>
#endif


// ==== OSStatus ====
typedef int         OSStatus;



#endif // __Common_h__