#include "config.h"

#include <pgmspace.h>

#define NODEBUG_PRINT
#include "debug_print.h"

#ifdef USE_LITTLE_FS
#include <LittleFS.h>
#define BOARD_FS    LittleFS
#else
#include <FS.h>
#define BOARD_FS    SPIFFS
#endif

#include <TimeLib.h>

#include <ArduinoJson.h>
#include <Homie.h>

#include "settings.h"
#include "Valve.h"
#include "Program.h"
#include "utils.h"

extern Valve* valves[NUMBER_OF_VALVES];
extern Program* programs[NUMBER_OF_PROGRAMS];
extern HomieNode* valve_node;
extern HomieNode* prg_node;
extern HomieNode* sys_node;

extern time_t sys_disabledTill;
extern unsigned char sys_intensity;

//------------ FS
int initFS(){
    CONSOLE_PGM(PSTR("[init:FS] Mounting FS\n"));
    if (!BOARD_FS.begin()){
        CONSOLE_PGM(PSTR("[init:FS] >  FS FAILED TO MOUNT!\n"));
        CONSOLE_PGM(PSTR("[init:FS] >  Make sure to upload config.json to FS first!\n"));
    } else {
        DEBUG_PRINT("[init:FS] FS available\n");
    }

    return 1;

}


/**
 * Load configuration stored in EEPROM (BOARD_FS).
 * Configuration is stored in text formatted json file.
 * {
 *    irrigation: {
 *      intensity: 100,
 *      disabled-till: "YYYY-MM-DD"
 *      valves:[10,10,0,0,0,0],
 *      programs:[
 *        { id: 1, run-times:"10,10,20,0,0,0", run-days:"10010101", start-hour: 6, start-min: 30 }
 *      ]
 *    }
 * }
 */
int loadConfig() {
    const char *module = "[init:ldcfg]";
    DynamicJsonDocument jdoc(2048);

    const char* f = LOCAL_CONFIG_FILE;
    DEBUG_PRINT("%s reading local config file=%s\n", module, f);
    
    if (!BOARD_FS.exists(f)) {
        DEBUG_PRINT("%s no configuration file\n", module);
        return 0;
    }
    
    File file = BOARD_FS.open(f,"r");
    auto err = deserializeJson(jdoc,file);
    file.close();

    if (err){
        CONSOLE_PGM(PSTR("%s invalid configuration file. error=%s\n"), module, err.c_str());
        return 0;
    }

    JsonVariant jv;
    JsonArray ja;
    JsonObject jroot = jdoc[F("settings")];
    unsigned int i = 0;

    // read valves
    CONSOLE_PGM(PSTR("%s Reading valves\n"), module);
    ja = jroot[F("valves")].as<JsonArray>();
    for(JsonVariant v : ja) {
        if (i>=NUMBER_OF_VALVES) {
            CONSOLE_PGM(PSTR("%s Error: too many valves in configuration file\n"), module);
            break;
        }
        CONSOLE_PGM(PSTR("%s   Valve %d "), module, i);
        unsigned int rt = v.as<unsigned int>();
        valves[i]->setRunTime(rt);
        CONSOLE_PGM(PSTR("manrt=%d\n"), rt);
        i++;
    }

    // read programs
    CONSOLE_PGM(PSTR("%s Reading programs\n"), module);
    ja = jroot[F("programs")].as<JsonArray>();
    i = 0;
    for(JsonVariant prg : ja) {
        if (i>=NUMBER_OF_PROGRAMS) {
            CONSOLE_PGM(PSTR("%s Error: too many programs in configuration file\n"), module);
            break;
        }

        CONSOLE_PGM(PSTR("%s   Program %d "), module, i);

        programs[i]->setName(prg[F("name")] | String("Program "+String(i)).c_str());
        programs[i]->setRunTimes(prg[F("run-times")] | "");
        programs[i]->setRunDays(prg[F("run-days")] | "0000000");
        programs[i]->setStart(
            prg[F("start-hour")] | (unsigned char)6,
            prg[F("start-min")] | (unsigned char)0
        );

        CONSOLE("\n");
        i++;
    }

    CONSOLE_PGM(PSTR("%s Reading global settings\n"), module);
    {
        // read intensity
        unsigned char i = jroot[F("intensity")] | (unsigned char)100;
        CONSOLE_PGM(PSTR("%s   Intensity=%d\n"), module, i);
        sys_intensity = i;
        for(int p=0;p<NUMBER_OF_PROGRAMS;p++) programs[p]->setIntensity(i);
    }
 
    {
        // read disabled till
        time_t i = jroot[F("disabled-till")] | 0;
        sys_disabledTill = i;
        CONSOLE_PGM(PSTR("%s   Disabled till=%d\n"), module, i);
    }
    
    return 1;
}

int saveConfig(){
    const char *module = "[init:svcfg]";
    DynamicJsonDocument jdoc(3000);

    const char* f = LOCAL_CONFIG_FILE;
    File file = BOARD_FS.open(f,"r");
    auto err = deserializeJson(jdoc,file);
    file.close();

    if (err){
        CONSOLE_PGM(PSTR("%s invalid configuration file. error=%s\n"), module, err.c_str());
        return 0;
    }    
    
    DEBUG_PRINT("%s Preparing local config\n", module);
    jdoc.remove("settings");
    JsonObject jroot = jdoc.createNestedObject("settings");

    if (valves[0]) {
        JsonArray ja = jroot.createNestedArray("valves");
        for(int i=0;i<NUMBER_OF_VALVES;i++){
            JsonObject jb = ja.createNestedObject();
            jb["id"] = i;
            jb["runtime"] = valves[i]->getRunTime();
        }
    }

    if (programs[0]) {
        JsonArray ja = jroot.createNestedArray("programs");
        for(int i=0;i<NUMBER_OF_PROGRAMS;i++){
            JsonObject jb = ja.createNestedObject();
            jb["id"] = i;
            jb["name"] = programs[i]->getName();
            jb["run-times"] = programs[i]->getRunTimes();
            jb["run-days"] = programs[i]->getRunDays();
            jb["start-hour"] = programs[i]->getStartHour();
            jb["start-min"] = programs[i]->getStartMinute();
        }
    }
    
    jroot["intensity"] = sys_intensity;
    jroot["disabled-till"] = sys_disabledTill;  

    DEBUG_PRINT("%s Saving local config file=%s\n", module, f);
    file = BOARD_FS.open(f,"w");
    size_t size = serializeJson(jdoc,file);
    DEBUG_PRINT("%s Saved %d bytes\n", module, size);
    file.close();

    return 1;

}