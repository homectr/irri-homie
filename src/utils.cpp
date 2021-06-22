#include "utils.h"
#include <Timezone.h>

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

const char* nowStr(){ nowStr(NULL);};

const char* nowStr(const char* tzAbbrev){
    if (millis()-lastCheck > 1000) dt2ISO(buf,25,now(),true,tzAbbrev);
    return buf;
}

const char* TrueStr = "true";
const char* FalseStr = "false";

const char* boolStr(bool v=1){return v?TrueStr:FalseStr;};

Timezone* determineTimeZone(const char* tzName){
    Timezone *tz = NULL;
    if (strcmp_P(tzName,PSTR("US/Eastern")) == 0) {
        TimeChangeRule dst = {"EDT", Second, Sun, Mar, 2, -240};  //UTC - 4 hours
        TimeChangeRule std = {"EST", First, Sun, Nov, 2, -300};   //UTC - 5 hours
        tz = new Timezone(dst, std);
    }
    if (strcmp_P(tzName,PSTR("Europe/Berlin")) == 0) {
        TimeChangeRule dst = {"CEST", Last, Sun, Mar, 1, +120};  //UTC + 2 hours
        TimeChangeRule std = {"CET", Last, Sun, Oct, 2, +60};   //UTC + 1 hours
        tz = new Timezone(dst, std);
    }
    if (strcmp_P(tzName,PSTR("Europe/London")) == 0) {
        TimeChangeRule dst = {"BST", Last, Sun, Mar, 1, +60};  //UTC + 1
        TimeChangeRule std = {"GMT", Last, Sun, Oct, 2, +0};   //UTC
        tz = new Timezone(dst, std);
    }    
    return tz;
}