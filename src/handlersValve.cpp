#include <Homie.h>
#include <Arduino.h>
#include "settings.h"
#include "Valve.h"
#include "utils.h"

#define NODEBUG_PRINT
#include "debug_print.h"

extern Valve* valves[NUMBER_OF_VALVES];
extern const String opts;
extern const unsigned char negativeOpts;

void onValveOpen(Valve* valve){
    if (!valve) return;  // if it's not a valid range
    DEBUG_PRINT("[onValveOpen] id=%d\n",valve->getIdStr);

    // update Homie property
    if (Homie.isConnected()) valve->getHomie()->setProperty("status").send(boolStr(true));

    Homie.getLogger() << nowStr() << " Valve " << valve->getIdStr() << " set to OPEN" << endl;
}

void onValveClose(Valve* valve){
    if (!valve) return;  // if it's not a valid range
    DEBUG_PRINT("[onValveClose] id=%d\n",valve->getIdStr());

    // update Homie property
    if (Homie.isConnected()) valve->getHomie()->setProperty("status").send(boolStr(false));
    Homie.getLogger() << nowStr() << " Valve " << valve->getIdStr() << " set to CLOSED" << endl;
}

bool handleValveStatus(const String& valveId, const String& value) {
    Valve* valve = findValveById(valveId.c_str());
    if (!valve) return false;  // if it's not a valid valve

    int8_t i = opts.indexOf(value);
    if (0 > i) return false;  // if the value is not valid
  
    bool on = (i < negativeOpts); // if one of the positive options

    if (on) valve->open(); // if valve opened manually, then use default run time
    else valve->close();

    return true;
}

bool handleValveRT(const String& valveId, const String& value){
    Valve* valve = findValveById(valveId.c_str());
    if (!valve) return false;  // if it's not a valid range

    int t = value.toInt();
    valve->setRunTime(t*60);

    if (Homie.isConnected()) valve->getHomie()->setProperty("runtime").send(String(t));
    Homie.getLogger() << nowStr() << "Valve runtime" << valveId << " set to " << t << endl;

    return true;
}