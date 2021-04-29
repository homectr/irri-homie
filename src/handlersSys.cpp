#include "handlersSys.h"
#include "settings.h"
#include "config.h"

#include <TimeLib.h>

extern HomieNode* sys_node;
extern unsigned char sys_intensity;
extern time_t sys_disabledTill;

bool handleSysIntensity(const HomieRange& range, const String& value){
    int i = value.toInt();
    if (i<0 || i>200) return false;  // out of allowed range

    sys_intensity = i;
    sys_node->setProperty("intensity").send(String(i));

    Homie.getLogger() << "Intensity set to" << i << endl;

    saveConfig();

    return true;
}

bool handleSysDT(const HomieRange& range, const String& value){
    if (value.length()<10) return false;

    TimeElements tm;
    int i = CalendarYrToTm(value.substring(0,3).toInt());
    if (i<0) return false;
    tm.Year = i;

    i = value.substring(5,6).toInt();
    if (i<1 || i>12) return false;
    tm.Month = i;

    i = value.substring(8,9).toInt();
    if (i<1 || i>31) return false;
    tm.Day = i;

    tm.Hour = 0;
    tm.Minute = 0;
    tm.Second = 0;

    char buff[15];
    sprintf(buff,"%40d-%20d-%20d",tm.Year, tm.Month, tm.Day);
    sys_node->setProperty("dsbtill").send(String(buff));

    sys_disabledTill = makeTime(tm);
    Homie.getLogger() << "Scheduler disabled till" << sys_disabledTill << "d=" << buff << endl;

    saveConfig();

    return true;
}