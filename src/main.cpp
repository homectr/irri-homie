#include <Homie.h>
#include <errno.h>
#include <NTPClient.h>
#include <Timezone.h>

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

unsigned char GPIOS[NUMBER_OF_VALVES] = { 13, 12, 14, 16, 2, 4 };
const char* optsON = "open OPEN on ON 1";
const char* optsOFF = "closed CLOSED off OFF 0";
String opts = String(optsON) + " " + String(optsOFF);
unsigned char negativeOpts = strlen(optsON)+1; // start of negative options

HomieNode* valve_node[NUMBER_OF_VALVES];  // node for manipulating irrigation valves
Valve* valves[NUMBER_OF_VALVES];  // array of valve objects

HomieNode* prg_node[NUMBER_OF_PROGRAMS];  // node for manipulating irrigation programs
Program* programs[NUMBER_OF_PROGRAMS]; // array of program objects

HomieNode* sys_node;    // node for manipulating irrigation system

HomieSetting<const char*> tzName("timezone", "Time-zone e.g. Europe/Berlin");

unsigned char sys_intensity = 100;
time_t sys_disabledTill = 0;

bool configLoaded = false;
bool timeConfigured = false;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0);
Timezone* tz;
TimeChangeRule *tcr;

time_t ntp2LocalTime(){
    DEBUG_PRINT("%lu Syncing time ", millis());
    DEBUG_PRINT("%s\n",timeClient.getFormattedTime().c_str());
    time_t utc = timeClient.getEpochTime();
    return tz ? tz->toLocal(utc, &tcr) : utc;
}

void setup() {
    Serial.begin(115200);
    Serial << endl << endl;

    for (uint8_t i = 0; i < NUMBER_OF_VALVES; i++)
        pinMode(GPIOS[i], OUTPUT);

    initFS();

    // create valves
    for (int i=0; i<NUMBER_OF_VALVES; i++){
        DEBUG_PRINT("Creating valve %d\n",i+1);
        valves[i] = new Valve(i+1);
        valves[i]->close();
        valves[i]->setOnOpenCB(onValveOpen);
        valves[i]->setOnCloseCB(onValveClose);
    }

    // create programs
    for (int i=0; i<NUMBER_OF_PROGRAMS; i++){
        DEBUG_PRINT("Creating program %d\n",i+1);
        programs[i] = new Program(i+1);
        programs[i]->setOnStartCB(onProgramStart);
        programs[i]->setOnStopCB(onProgramStop);
        for (int j=0;j<NUMBER_OF_VALVES;j++) programs[i]->addValve(valves[j]);
    }

    Homie_setFirmware("Irrigation", "1.0.0");
    Homie.setGlobalInputHandler(updateHandler);
    Homie.setLedPin(15, 1);

    DEBUG_PRINT("Configuring valve properties\n");
    for(int i=0; i<NUMBER_OF_VALVES; i++){
        valve_node[i] = new HomieNode(valves[i]->getIdStr(), valves[i]->getIdStr(), "valve");
        valve_node[i]->advertise("status").setDatatype("boolean").settable();
        valve_node[i]->advertise("runtime").setDatatype("integer").setFormat("0:120").settable();
    }

    // create homie node for valves
    DEBUG_PRINT("Configuring program properties\n");
    for(int i=0; i<NUMBER_OF_PROGRAMS; i++){
        prg_node[i] = new HomieNode(programs[i]->getIdStr(), programs[i]->getIdStr(), "program");
        prg_node[i]->advertise("status").setDatatype("boolean").settable();
        prg_node[i]->advertise("name").setDatatype("string").settable();
        prg_node[i]->advertise("startHour").setDatatype("integer").setFormat("0:23").settable();
        prg_node[i]->advertise("startMin").setDatatype("integer").setFormat("0:59").settable();
        for(int j=0;j<7;j++){
            String did = "day"+String(j+1);
            prg_node[i]->advertise(did.c_str()).setDatatype("boolean").settable();
        }
        for(int j=0;j<NUMBER_OF_VALVES;j++){
            String vid = String("rt") + valves[j]->getIdStr();
            prg_node[i]->advertise(vid.c_str()).setDatatype("integer").setFormat("0:120").settable();
        }
    }
    
    DEBUG_PRINT("Configuring system properties\n");
    sys_node = new HomieNode("system", "Irrigation system", "device");
    sys_node->advertise("disabledTill").setDatatype("string").settable(handleSysDT);
    sys_node->advertise("intensity").setDatatype("integer").setUnit("%").settable(handleSysIntensity);

    Homie.onEvent(onHomieEvent);

    tzName.setDefaultValue("Europe/Berlin").setValidator([] (const char* tz) { return strcmp_P(tz,PSTR("Europe/Berlin")) == 0;});

    Homie.setup();

    DEBUG_PRINT("Printing program configuration\n");
    for(int i=0;i<NUMBER_OF_PROGRAMS;i++)
        programs[i]->printConfig();

    // configure time
    CONSOLE("Starting NTP client\n");
    timeClient.begin();
    setSyncProvider(ntp2LocalTime);

    tz = determineTimeZone(tzName.get());
    CONSOLE("Timezone tz=%s\n",tz?tzName.get():"unknown");

}

#define CHECK_INTERVAL 2000
#define ALIVE_INTERVAL 30000
unsigned long lastCheck = millis()-1000000;
unsigned long alive = millis() - 1000000;

void loop() {
    Homie.loop();
    timeClient.update();

    if (millis()-alive > (long)ALIVE_INTERVAL){
        CONSOLE("%s alive\n",nowStr(tcr->abbrev));
        alive=millis();

        if (!timeConfigured && WiFi.isConnected()){
            CONSOLE("%s configuring time: ",nowStr());
            if (timeClient.forceUpdate()){
                setTime(ntp2LocalTime());
                CONSOLE("%s\n",nowStr(tcr->abbrev));
                timeConfigured = true;
            } else {
                CONSOLE("failed\n");
            }
        }

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