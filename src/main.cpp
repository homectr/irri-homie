#include <Homie.h>
#include <errno.h>
#include <NTPClient.h>

#include "settings.h"
#include "Valve.h"
#include "Program.h"
#include "System.h"
#include "config.h"

#include "handlers.h"
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

HomieNode* valve_node[NUMBER_OF_VALVES];  // node for manipulating irrigation valves
Valve* valves[NUMBER_OF_VALVES];  // array of valve objects

HomieNode* prg_node[NUMBER_OF_PROGRAMS];  // node for manipulating irrigation programs
Program* programs[NUMBER_OF_PROGRAMS]; // array of program objects

HomieNode* sys_node;    // node for manipulating irrigation system

unsigned char sys_intensity = 100;
time_t sys_disabledTill = 0;

bool configLoaded = false;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600);

time_t getNTPtime(){
    return 0;
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
        DEBUG_PRINT("Creating valve %d\n",i);
        valves[i] = new Valve(i);
        valves[i]->setOnOpenCB(onValveOpen);
        valves[i]->setOnCloseCB(onValveClose);
    }

    // create programs
    for (int i=0; i<NUMBER_OF_PROGRAMS; i++){
        DEBUG_PRINT("Creating program %d\n",i);
        programs[i] = new Program(i);
        programs[i]->setOnStartCB(onProgramStart);
        programs[i]->setOnStopCB(onProgramStop);
        for (int j=0;j<NUMBER_OF_VALVES;j++) programs[i]->addValve(valves[j]);
    }

    Homie_setFirmware("Irrigation", "1.0.0");
    Homie.setGlobalInputHandler(updateHandler);

    DEBUG_PRINT("Configuring valve properties\n");
    for(int i=0; i<NUMBER_OF_VALVES; i++){
        valve_node[i] = new HomieNode(valves[i]->getIdStr(), valves[i]->getName(), "switch");
        valve_node[i]->advertise("status").setName("Status").setDatatype("boolean").settable();
        valve_node[i]->advertise("runtime").setName("Manual Run Time").setDatatype("integer").setFormat("0:120").settable();
    }

    // create homie node for valves
    DEBUG_PRINT("Configuring program properties\n");
    for(int i=0; i<NUMBER_OF_PROGRAMS; i++){
        prg_node[i] = new HomieNode(programs[i]->getIdStr(), programs[i]->getName(), "switch");
        prg_node[i]->advertise("status").setName("Status").setDatatype("boolean").settable();
        prg_node[i]->advertise("name").setName("Name").setDatatype("string").settable();
        prg_node[i]->advertise("starthour").setName("Start Hour").setDatatype("integer").setFormat("0:23").settable();
        prg_node[i]->advertise("startmin").setName("Start Minute").setDatatype("integer").setFormat("0:59").settable();
        for(int j=0;j<7;j++){
            String did = "day"+String(j);
            prg_node[i]->advertise(did.c_str()).setName(programs[i]->getRunDayName(j)).setDatatype("boolean").settable();
        }
        for(int j=0;j<NUMBER_OF_VALVES;j++){
            String vid = valves[j]->getIdStr() + String("rt");
            prg_node[i]->advertise(vid.c_str()).setName(programs[i]->getValveName(j)).setDatatype("integer").setFormat("0:120").settable();
        }
    }
    
    DEBUG_PRINT("Configuring system properties\n");
    sys_node = new HomieNode("system", "Irrigation system", "irrigation");
    sys_node->advertise("dsbtill").setName("Disabled till").setDatatype("string").settable(handleSysDT);
    sys_node->advertise("intensity").setName("Irrigation intensity").setDatatype("integer").setFormat("0:200").setUnit("%").settable(handleSysIntensity);

    Homie.onEvent(onHomieEvent);
    Homie.setup();

    DEBUG_PRINT("Starting NTP client\n");
    timeClient.begin();

    DEBUG_PRINT("Printing program configuration\n");
    for(int i=0;i<NUMBER_OF_PROGRAMS;i++)
        programs[i]->printConfig();

    setTime(8,30,0,27,5,2021); // FIXME
    //setSyncProvider(getNTPtime);

}

#define CHECK_INTERVAL 2000
#define ALIVE_INTERVAL 30000
unsigned long lastCheck = millis()-1000000;
unsigned long alive = millis() - 1000000;

void loop() {
    Homie.loop();

    if (millis()-alive > (long)ALIVE_INTERVAL){
        CONSOLE("%s alive\n",nowStr());
        alive=millis();
    }

    if (configLoaded && (millis()-lastCheck) > (long)CHECK_INTERVAL) {

        DEBUG_PRINT("Checking valves %d:%d\n",hour(), minute());
        for (int i=0;i<NUMBER_OF_VALVES;i++) 
            if (valves[i]) valves[i]->loop();

        if (sys_disabledTill < now()){
            DEBUG_PRINT("Checking programs\n");
            for (int i=0;i<NUMBER_OF_PROGRAMS;i++) 
                if (programs[i]) programs[i]->loop();
        } else {
            CONSOLE("Programs disabled till %ld\n", sys_disabledTill );
        }
        DEBUG_PRINT("Checking end\n");

        lastCheck=millis();
    }

    if (!configLoaded && Homie.isConfigured()){
        loadConfig();
        configLoaded = true;
        for(int i=0;i<NUMBER_OF_PROGRAMS;i++)
            programs[i]->printConfig();
    }

}