#include <Homie.h>
#include <errno.h>
#include <NTPClient.h>

#include "settings.h"
#include "Valve.h"
#include "Program.h"
#include "System.h"
#include "config.h"

#include "handlersPrg.h"
#include "handlersValve.h"
#include "handlersSys.h"
#include "handlersHomie.h"
#include "utils.h"

#define NODEBUG_PRINT
#include "debug_print.h"

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

bool configLoaded = false;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600);

time_t getNTPtime(){
    
}

void setup() {
    for (uint8_t i = 0; i < NUMBER_OF_VALVES; i++) {
        pinMode(GPIOS[i], OUTPUT);
        digitalWrite(GPIOS[i], LOW);
    }

    Serial.begin(115200);
    Serial << endl << endl;

    initFS();

    // TODO: NTP client
    // TODO: Homie settings for UTC offset + time server

    // create valves
    for (int i=0; i<NUMBER_OF_VALVES; i++){
        valves[i] = new Valve(i);
        valves[i]->setOnOpenCB(onValveOpen);
        valves[i]->setOnCloseCB(onValveClose);
        DEBUG_PRINT("Creating valve %d addr=%X\n",i,valves[i]);
    }

    // create programs
    for (int i=0; i<NUMBER_OF_PROGRAMS; i++){
        programs[i] = new Program(i);
        programs[i]->setOnStartCB(onProgramStart);
        programs[i]->setOnStopCB(onProgramStop);
        for (int j=0; j<NUMBER_OF_VALVES; j++)
            programs[i]->addValve(valves[j]);
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

    Homie.onEvent(onHomieEvent);
    Homie.setup();

    timeClient.begin();

    for(int i=0;i<NUMBER_OF_PROGRAMS;i++)
        programs[i]->printConfig();

    setTime(8,30,0,27,5,2021);
    //setSyncProvider(getNTPtime);

}

#define CHECK_INTERVAL 60000
unsigned long lastCheck = millis()-1000000;
char buf[25];

void loop() {
    Homie.loop();

    if (configLoaded && (millis()-lastCheck) > (long)CHECK_INTERVAL) {
        dt2ISO(buf,25,now(),true,NULL);
        CONSOLE("Current time=%s\n",buf);

        DEBUG_PRINT("Checking valves %d:%d\n",hour(), minute());
        for (int i=0;i<NUMBER_OF_VALVES;i++) 
            if (valves[i]) valves[i]->loop();

        if (sys_disabledTill < now()){
            DEBUG_PRINT("Checking programs\n");
            for (int i=0;i<NUMBER_OF_PROGRAMS;i++) 
                if (programs[i]) programs[i]->loop();
        } else {
            CONSOLE("Programs disabled till %d\n", sys_disabledTill );
        }

        lastCheck=millis();
        DEBUG_PRINT("Checking end\n");
    }

    if (!configLoaded && Homie.isConfigured()){
        loadConfig();
        configLoaded = true;
        for(int i=0;i<NUMBER_OF_PROGRAMS;i++)
            programs[i]->printConfig();

    }

}