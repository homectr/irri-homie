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
extern const unsigned char GPIOS[NUMBER_OF_VALVES];
extern HomieNode* valve_node[NUMBER_OF_VALVES];

void onValveOpen(unsigned char valveId){
    DEBUG_PRINT("[onValveOpen] id=%d\n",valveId);
    if (valveId >= NUMBER_OF_VALVES) return;  // if it's not a valid range
    // switch valve on
    digitalWrite(GPIOS[valveId], 1);

    // update Homie property
    if (Homie.isConnected()) valve_node[valveId]->setProperty("status").send("1");

    Homie.getLogger() << nowStr() << " Valve " << valveId << " set to OPEN" << endl;
}

void onValveClose(unsigned char valveId){
    DEBUG_PRINT("[onValveClose] id=%d\n",valveId);
    if (valveId >= NUMBER_OF_VALVES) return;  // if it's not a valid range
    // switch valve off
    digitalWrite(GPIOS[valveId], 0);

    // update Homie property
    if (Homie.isConnected()) valve_node[valveId]->setProperty("status").send("0");
    Homie.getLogger() << nowStr() << " Valve " << valveId << " set to CLOSED" << endl;
}

bool handleValveStatus(unsigned char valveIdx, const String& value) {
    if (valveIdx >= NUMBER_OF_VALVES) return false;  // if it's not a valid range

    int8_t i = opts.indexOf(value);
    if (0 > i) return false;  // if the value is not valid
  
    bool on = (i < negativeOpts); // if one of the positive options

    if (on) valves[valveIdx]->open(); // if valve opened manually, then use default run time
    else valves[valveIdx]->close();

    digitalWrite(GPIOS[valveIdx], on ? HIGH : LOW);

    return true;
}

bool handleValveRT(unsigned char valveId, const String& value){
    if (valveId > NUMBER_OF_VALVES) return false;  // if it's not a valid range

    int t = value.toInt();
    valves[valveId]->setRunTime(t*60);

    if (Homie.isConnected()) valve_node[valveId]->setProperty("runtime").send(String(t));
    Homie.getLogger() << nowStr() << "Valve runtime" << valveId << " set to " << t << endl;

    return true;
}