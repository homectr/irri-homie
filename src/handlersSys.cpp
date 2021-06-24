#include "handlersSys.h"
#include "settings.h"
#include "config.h"
#include "utils.h"
#include <TimeLib.h>
#include <Timezone.h>

#define NODEBUG_PRINT
#include "debug_print.h"

extern HomieNode* sys_node;
extern unsigned char sys_intensity;
extern time_t sys_disabledTill;
extern char sys_disabledTillStr[25];
extern Timezone *tz;

bool handleSysIntensity(const HomieRange& range, const String& value){
    int i = value.toInt();
    DEBUG_PRINT("Set Intensity: %s %d\n",value.c_str(),i);
    if (i<0 || i>200) return false;  // out of allowed range

    sys_intensity = i;
    if (Homie.isConnected()) sys_node->setProperty("intensity").send(String(i));

    Homie.getLogger() << "Intensity set to " << i << endl;

    return saveConfig();;
}

bool handleSysDT(const HomieRange& range, const String& value){
    DEBUG_PRINT("Set DisabledTill: %s\n",value.c_str());

    TimeElements tm;
    DEBUG_PRINT("sss=%s-%s-%s\n",value.substring(0,4).c_str(),value.substring(5,7).c_str(),value.substring(8,10).c_str());
    tm.Year = CalendarYrToTm(value.substring(0,4).toInt());
    tm.Month = value.substring(5,7).toInt();
    tm.Day = value.substring(8,10).toInt();
    tm.Hour = value.substring(11,13).toInt();
    tm.Minute = value.substring(14,16).toInt();
    tm.Second = 0;
    DEBUG_PRINT("utc=%04d-%02d-%02d\n",tmYearToCalendar(tm.Year), tm.Month, tm.Day);

    time_t utc = makeTime(tm);
    DEBUG_PRINT("utc=%lu\n",utc);

    TimeChangeRule *tr;

    time_t local = tz->toLocal(utc,&tr);
    DEBUG_PRINT("local=%lu tz=%s %d\n",local, tr->abbrev, tr->offset);
    breakTime(local,tm);
    DEBUG_PRINT("local=%04d-%02d-%02d\n",tmYearToCalendar(tm.Year), tm.Month, tm.Day);

    if (Homie.isConnected()) sys_node->setProperty("disabledTill").send(value);

    sys_disabledTill = local;
    dt2ISO(sys_disabledTillStr,25,sys_disabledTill,false,NULL);

    Homie.getLogger() << "Scheduler disabled till " << sys_disabledTill << " d=" << tmYearToCalendar(tm.Year) << '-' << tm.Month << '-' << tm.Day << endl;

    return saveConfig();
}