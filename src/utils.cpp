#include "utils.h"

#define NODEBUG_PRINT
#include "debug_print.h"

int dt2ISO(char* buffer, size_t size, time_t dt, bool includeTime, const char* tz){
    if (size < 11) return 0;
    snprintf(buffer,size,"%04d-%02d-%02d",year(dt),month(dt),day(dt));
    DEBUG_PRINT("ISODATE1=%s\n",buffer);
    if (includeTime)
        snprintf(buffer+10,size-10,"T%02d:%02d:%02d%s", hour(dt), minute(dt), second(dt), tz ? tz:"Z");
    DEBUG_PRINT("ISODATE2=%s\n",buffer);
    return 1;
}

static char buf[25];
static long lastCheck = millis()-10000;

const char* nowStr(){
    if (millis()-lastCheck > 1000) dt2ISO(buf,25,now(),true,NULL);
    return buf;
}