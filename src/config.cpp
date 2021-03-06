#include "config.h"

#include <pgmspace.h>

#ifdef USE_LITTLE_FS
#include <LittleFS.h>
#define BOARD_FS LittleFS
#else
#include <FS.h>
#define BOARD_FS SPIFFS
#endif

#include <TimeLib.h>

#include <ArduinoJson.h>
#include <Homie.h>

#include "settings.h"
#include "Valve.h"
#include "Program.h"
#include "utils.h"

#define NODEBUG_PRINT
#include "debug_print.h"

extern Valve *valves[NUMBER_OF_VALVES];
extern Program *programs[NUMBER_OF_PROGRAMS];

extern time_t sys_disabledTill;
extern char sys_disabledTillStr[25];

extern unsigned char sys_intensity;

//------------ FS
int initFS()
{
  CONSOLE_PGM(PSTR("[init:FS] Mounting FS\n"));
  if (!BOARD_FS.begin())
  {
    CONSOLE_PGM(PSTR("[init:FS] >  FS FAILED TO MOUNT!\n"));
    CONSOLE_PGM(PSTR("[init:FS] >  Make sure to upload config.json to FS first!\n"));
  }
  else
  {
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
 *      disabled-till: 0,
 *      valves:[10,10,0,0,0,0],
 *      programs:[
 *        { id: 1, run-times:"10,10,20,0,0,0", run-days:"10010101", start-hour: 6, start-min: 30 }
 *      ]
 *    }
 * }
 */
int loadConfig()
{
  const char *module = "[init:ldcfg]";
  DynamicJsonDocument jdoc(2048);

  const char *f = LOCAL_CONFIG_FILE;
  DEBUG_PRINT("%s reading local config file=%s\n", module, f);

  if (!BOARD_FS.exists(f))
  {
    DEBUG_PRINT("%s no configuration file\n", module);
    return 0;
  }

  File file = BOARD_FS.open(f, "r");
  auto err = deserializeJson(jdoc, file);
  file.close();

  if (err)
  {
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
  for (JsonVariant v : ja)
  {
    if (i >= NUMBER_OF_VALVES)
    {
      CONSOLE_PGM(PSTR("%s Error: too many valves in configuration file\n"), module);
      break;
    }
    CONSOLE_PGM(PSTR("%s   Valve %d "), module, i);
    unsigned int rt = v | 0;
    valves[i]->setRunTime(rt);
    CONSOLE_PGM(PSTR("runtime=%d\n"), rt);
    i++;
  }

  // read programs
  CONSOLE_PGM(PSTR("%s Reading programs\n"), module);
  ja = jroot[F("programs")].as<JsonArray>();
  i = 0;
  for (JsonVariant prg : ja)
  {
    if (i >= NUMBER_OF_PROGRAMS)
    {
      CONSOLE_PGM(PSTR("%s Error: too many programs in configuration file\n"), module);
      break;
    }

    CONSOLE_PGM(PSTR("%s   Program %d "), module, i);
    CONSOLE(" rt=");
    int ii = 0;
    for (JsonVariant vrt : prg[F("run-times")].as<JsonArray>())
    {
      unsigned int rt = vrt | 0;
      programs[i]->setRunTime(ii, rt);
      CONSOLE("%d,", programs[i]->getRunTime(ii));
      ii++;
    }

    ii = 0;
    CONSOLE(" rd=");
    for (JsonVariant vrd : prg[F("run-days")].as<JsonArray>())
    {
      unsigned char rd = vrd | 0;
      programs[i]->setRunDay(ii, rd);
      CONSOLE("%d:", programs[i]->getRunDay(ii));
      ii++;
    }

    programs[i]->setStart(
        prg[F("start-hour")] | (unsigned char)6,
        prg[F("start-min")] | (unsigned char)0);

    CONSOLE(" start=%02d:%02d\n", programs[i]->getStartHour(), programs[i]->getStartMinute());
    i++;
  }

  CONSOLE_PGM(PSTR("%s Reading global settings\n"), module);
  {
    // read intensity
    unsigned char i = jroot[F("intensity")] | (unsigned char)100;
    CONSOLE_PGM(PSTR("%s   Intensity=%d\n"), module, i);
    sys_intensity = i;
    for (int p = 0; p < NUMBER_OF_PROGRAMS; p++)
      programs[p]->setIntensity(i);
  }

  {
    // read disabled till
    time_t i = jroot[F("disabled-till")] | 0;
    sys_disabledTill = i;
    dt2ISO(sys_disabledTillStr, 25, sys_disabledTill, false, NULL);
    CONSOLE_PGM(PSTR("%s   Disabled till=%d\n"), module, i);
  }

  return 1;
}

int saveConfig()
{
  const char *module = "[init:svcfg]";
  DynamicJsonDocument jdoc(3000);

  const char *f = LOCAL_CONFIG_FILE;
  File file = BOARD_FS.open(f, "r");
  auto err = deserializeJson(jdoc, file);
  file.close();

  if (err)
  {
    CONSOLE_PGM(PSTR("%s invalid configuration file. error=%s\n"), module, err.c_str());
    // FIXME ignoring error and writing config anyway...
  }

  DEBUG_PRINT("%s Preparing local config\n", module);
  jdoc.remove("settings");
  JsonObject jroot = jdoc.createNestedObject("settings");

  if (valves[0])
  {
    JsonArray ja = jroot.createNestedArray("valves");
    for (int i = 0; i < NUMBER_OF_VALVES; i++)
      ja.add(valves[i]->getRunTime());
  }

  if (programs[0])
  {
    JsonArray ja = jroot.createNestedArray("programs");
    for (int i = 0; i < NUMBER_OF_PROGRAMS; i++)
    {
      if (programs[i]->isConfigured())
      {
        JsonObject jb = ja.createNestedObject();
        jb["start-hour"] = programs[i]->getStartHour();
        jb["start-min"] = programs[i]->getStartMinute();

        JsonArray ja = jb.createNestedArray("run-times");
        for (int j = 0; j < NUMBER_OF_VALVES; j++)
          ja.add(programs[i]->getRunTime(j));

        ja = jb.createNestedArray("run-days");
        for (int j = 0; j < 7; j++)
          ja.add(programs[i]->getRunDay(j));
      }
    }
  }

  jroot["intensity"] = sys_intensity;
  jroot["disabled-till"] = sys_disabledTill;

  DEBUG_PRINT("%s Saving local config file=%s\n", module, f);
  file = BOARD_FS.open(f, "w");
  size_t size = serializeJson(jdoc, file);
  DEBUG_PRINT("%s Saved %d bytes\n", module, size);
  file.close();

#ifndef NODEBUG_PRINT
  DEBUG_PRINT("Saved: start>>\n");
  serializeJsonPretty(jroot, Serial);
  DEBUG_PRINT("<<end\n");
#endif

  return 1;
}