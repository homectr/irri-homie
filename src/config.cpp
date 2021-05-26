#include "config.h"

#include <pgmspace.h>

//#define NODEBUG_PRINT
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



extern Valve* valves[NUMBER_OF_VALVES];
extern Program* programs[NUMBER_OF_PROGRAMS];
extern HomieNode* valve_node;
extern HomieNode* prg_node;
extern HomieNode* sys_node;

extern time_t sys_disabledTill;
extern unsigned char sys_intensity;

//------------ FS
#define FS_READY_FILE           "/ready.txt"    // if exist SPIFFS is formatted & available

int initFS(){
    CONSOLE_PGM(PSTR("[init:FS] Mounting FS\n"));
    if (!BOARD_FS.begin()){
        CONSOLE_PGM(PSTR("[init:FS] >  FS FAILED TO MOUNT!\n"));
        CONSOLE_PGM(PSTR("[init:FS] >  going to format FS..."));
        if (BOARD_FS.format()){
            CONSOLE_PGM(PSTR("done.\n"));
            CONSOLE_PGM(PSTR("[init:FS] >  Mounting FS again...\n"));
            if (BOARD_FS.begin()){
                CONSOLE_PGM(PSTR("done.\n"));
            } else {
                CONSOLE_PGM(PSTR("FAILED!\n"));
                return 0;

            }
        } else {
            DEBUG_PRINT("FAILED!\n");
            return 0;
        }
    } else {
        DEBUG_PRINT("[init:FS] mounted\n");
        DEBUG_PRINT("[init:FS] ui bundle loaded %d\n",BOARD_FS.exists("/homie"));
        Dir dir = BOARD_FS.openDir("/");
        while (dir.next()) {
            DEBUG_PRINT("file=%s\n",dir.fileName().c_str());
        }
    }

    if (!BOARD_FS.exists(FS_READY_FILE)) {
        DEBUG_PRINT("[init:FS] >  FS will be formatted\n");
        if (BOARD_FS.format())
            DEBUG_PRINT(">  done.\n");
        
        File f = BOARD_FS.open(FS_READY_FILE, "w");
        if (!f) {
            DEBUG_PRINT("[init:FS] ERROR: FS write not possible.\n");
            return 0;
        } else {
            f.println("FS ready");
        }
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
    JsonObject jroot = jdoc[F("irrigation")];
    unsigned int i = 0;

    // read valves
    CONSOLE_PGM(PSTR("%s Reading valves\n"), module);
    ja = jroot[F("valves")].as<JsonArray>();
    for(JsonVariant v : ja) {
        CONSOLE_PGM(PSTR("%s   Valve %d "), module, i);
        unsigned int rt = v.as<unsigned int>();
        valves[i]->setRunTime(rt);
        valve_node->setProperty("manrt").setRange(i).send(String(rt));
        CONSOLE_PGM(PSTR("manrt=%d\n"), rt);
        i++;
        if (i>=NUMBER_OF_VALVES) {
            CONSOLE_PGM(PSTR("%s Error: too many valves in configuration file\n"), module);
            break;
        }
    }

    // read programs
    CONSOLE_PGM(PSTR("%s Reading programs\n"), module);
    ja = jroot[F("programs")].as<JsonArray>();
    i = 0;
    for(JsonVariant v : ja) {
        CONSOLE_PGM(PSTR("%s   Program %d "), module, i);

        jv = v[F("name")];
        if (jv) {
            const char* c = jv.as<const char*>();
            programs[i]->setName(c);
            prg_node->setProperty("name").setRange(i).send(String(c));
            CONSOLE_PGM(PSTR(" name=%s"), c);
        }

        jv = v[F("run-times")];
        if (jv) {
            const char* c = jv.as<const char*>();
            programs[i]->setRunTimes(c);
            prg_node->setProperty("runtimes").setRange(i).send(String(c));
            CONSOLE_PGM(PSTR(" rt=%s"), c);
        }

        jv = v[F("run-days")];
        if (jv) {
            const char* c = jv.as<const char*>();
            programs[i]->setRunDays(c);
            prg_node->setProperty("rundays").setRange(i).send(String(c));
            CONSOLE_PGM(PSTR(" rd=%d"), c);
        }

        jv = v[F("start-hour")];
        if (jv) {
            unsigned char h = jv.as<unsigned char>();
            CONSOLE_PGM(PSTR(" sh=%d"), h);
            jv = v[F("start-min")];
            if (jv) {
                unsigned char m = jv.as<unsigned char>();
                CONSOLE_PGM(PSTR(" sm=%d"), m);
                programs[i]->setStart(h,m);
                prg_node->setProperty("starthour").setRange(i).send(String(h));
                prg_node->setProperty("startmin").setRange(i).send(String(m));
            }
        }
        CONSOLE("\n");
        i++;
        if (i>=NUMBER_OF_PROGRAMS) {
            CONSOLE_PGM(PSTR("%s Error: too many programs in configuration file\n"), module);
            break;
        }
    }

    CONSOLE_PGM(PSTR("%s Reading system settings\n"), module);
    // read intensity
    jv = jroot[F("intensity")];
    if (jv){
        unsigned char i = jv.as<unsigned char>();
        CONSOLE_PGM(PSTR("%s   Intensity=%d\n"), i);
        sys_intensity = i;
        sys_node->setProperty("intensity").send(String(i));
    }

   // read disabled till
    jv = jdoc[F("disabled-till")];
    if (jv){
        time_t i = jv.as<time_t>();
        sys_disabledTill = i;
        char buff[15];
        snprintf(buff,14,"%04d-%02d-%02d",year(i),month(i),day(i));
        CONSOLE_PGM(PSTR("%s   Disabled till=%lu date=%s\n"), module, i, buff);
        sys_node->setProperty("dsbtill").send(String(buff));
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
    jdoc.remove("irrigation");
    JsonObject jroot = jdoc.createNestedObject("irrigation");

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
            char* buff = (char*)malloc(300);
            programs[i]->getRunTimes(buff,300);
            jb["run-times"] = buff;
            programs[i]->getRunDays(buff,300);
            jb["run-days"] = buff;
            jb["start-hour"] = programs[i]->getStartHour();
            jb["start-min"] = programs[i]->getStartMinute();
            free(buff);
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