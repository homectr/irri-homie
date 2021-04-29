#include <Homie.h>
#include <Arduino.h>
#include "settings.h"
#include "Valve.h"

extern Valve* valves[NUMBER_OF_VALVES];
extern const String opts;
extern const unsigned char negativeOpts;
extern const unsigned char GPIOS[NUMBER_OF_VALVES];
extern HomieNode* valve_node;

void onValveOpen(unsigned char valveId){
    // switch valve on
    digitalWrite(GPIOS[valveId], 1);

    // update Homie property
    valve_node->setProperty("status").setRange(valveId).send("OPEN");
    Homie.getLogger() << "Valve " << valveId << " is OPEN" << endl;
}

void onValveClose(unsigned char valveId){
    // switch valve off
    digitalWrite(GPIOS[valveId], 0);

    // update Homie property
    valve_node->setProperty("status").setRange(valveId).send("CLOSED");
    Homie.getLogger() << "Valve " << valveId << " is CLOSED" << endl;
}

bool handleValveStatus(const HomieRange& range, const String& value) {
    if (!range.isRange) return false;  // if it's not a range
    if (range.index < 1 || range.index > NUMBER_OF_VALVES) return false;  // if it's not a valid range

    int8_t i = opts.indexOf(value);
    if (0 > i) return false;  // if the value is not valid
  
    bool on = (i < negativeOpts); // if one of the positive options

    if (on) valves[range.index-1]->open(); // if valve opened manually, then use default run time
    else valves[range.index-1]->close();

    digitalWrite(GPIOS[range.index - 1], on ? HIGH : LOW);

    return true;
}

bool handleValveRT(const HomieRange& range, const String& value){
    if (!range.isRange) return false;  // if it's not a range
    if (range.index < 1 || range.index > NUMBER_OF_VALVES) return false;  // if it's not a valid range

    unsigned char valveId = range.index-1;
    int t = value.toInt();
    valves[valveId]->setRunTime(t);

    valve_node->setProperty("manrt").setRange(valveId).send(String(t));
    Homie.getLogger() << "Valve runtime" << valveId << " set to " << t << endl;

    return true;
}