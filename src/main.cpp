#include <Homie.h>
#include <errno.h>
#include <LittleFS.h>

#include "settings.h"
#include "Valve.h"
#include "Program.h"
#include "System.h"
#include "config.h"

#include "handlersPrg.h"
#include "handlersValve.h"
#include "handlersSys.h"

unsigned char GPIOS[NUMBER_OF_VALVES] = { 16, 5, 4, 14, 12, 13 };
const char* optsON = "open OPEN on ON 1";
const char* optsOFF = "closed CLOSED off OFF 0";
String opts = String(optsON) + " " + String(optsOFF);
unsigned char negativeOpts = strlen(optsON)+1; // start of negative options

HomieNode* valve_node;  // node for manipulating irrigation valves
Valve* valves[NUMBER_OF_VALVES];  // array of valve objects

HomieNode* prg_node;  // node for manipulating irrigation programs
Program* programs[NUMBER_OF_PROGRAMS]; // array of program objects

HomieNode* sys_node;    // node for manipulating irrigation system

unsigned char sys_intensity = 100;
time_t sys_disabledTill = 0;

void setup() {
    for (uint8_t i = 0; i < NUMBER_OF_VALVES; i++) {
        pinMode(GPIOS[i], OUTPUT);
        digitalWrite(GPIOS[i], LOW);
    }

    Serial.begin(115200);
    Serial << endl << endl;

    initFS();

    // create valves
    for (int i=0; i<NUMBER_OF_VALVES; i++){
        valves[i] = new Valve(i);
        valves[i]->setOnOpenCB(onValveOpen);
        valves[i]->setOnCloseCB(onValveClose);
        // TODO set valve properties from configuration
    }

    // create programs
    for (int i=0; i<NUMBER_OF_PROGRAMS; i++){
        programs[i] = new Program(i, valves, NUMBER_OF_VALVES);
        programs[i]->setOnStartCB(onProgramStart);
        programs[i]->setOnStopCB(onProgramStop);
        // TODO set program properties from configuration
        // TODO name set handler programs[i]->setName("Program handleProgramStop);    
    }

    Homie_setFirmware("Irrigation", "1.0.0");

    // create homie node for valves
    valve_node = new HomieNode("valves", "Irrigation valves", "valve", true, 1, NUMBER_OF_VALVES);
    valve_node->advertise("status").setName("Status").setDatatype("enum").setFormat("OPEN,CLOSED").settable(handleValveStatus);
    valve_node->advertise("manrt").setName("Manual Run Time").setDatatype("integer").setFormat("0:60").settable(handleValveRT);

    prg_node = new HomieNode("programs", "Irrigation programs", "program", true, 1, NUMBER_OF_PROGRAMS);
    prg_node->advertise("status").setName("Status").setDatatype("enum").setFormat("ON,OFF").settable(handleProgramStatus);
    prg_node->advertise("name").setName("Name").setDatatype("string").settable(handleProgramName);
    prg_node->advertise("starthour").setName("Start Hour").setDatatype("integer").setFormat("0:23").settable(handleProgramStartHour);
    prg_node->advertise("startmin").setName("Start Minute").setDatatype("integer").setFormat("0:59").settable(handleProgramStartMin);
    prg_node->advertise("rundays").setName("Program Run Days").setDatatype("string").settable(handleProgramRunDays);
    prg_node->advertise("runtimes").setName("Valve Run Times").setDatatype("string").settable(handleProgramRunTimes);
    
    sys_node = new HomieNode("system", "Irrigation system", "irrigation");
    sys_node->advertise("dsbtill").setName("Disabled till").setDatatype("string").settable(handleSysDT);
    sys_node->advertise("intensity").setName("Irrigatin intensity").setDatatype("integer").setFormat("0:200").setUnit("%").settable(handleSysIntensity);

    Homie.setup();

    loadConfig();
}

void loop() {
    Homie.loop();

}