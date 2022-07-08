#pragma once

#include <TimeLib.h>
#include <Homie.h>
#include "settings.h"
#include "Valve.h"

class Program;

using program_cb_t = void (*)(Program *program);

class Program
{

private:
  // program identifier
  unsigned char id;

  // program identifer for mqtt
  char *idStr = NULL;

  // program starting hour
  unsigned char startHour = 6;

  // program starting minute
  unsigned char startMin = 0;

  // how long (in seconds) should each valve be open
  unsigned int runTimes[NUMBER_OF_VALVES];

  // which days of a week should program run, starting with Sunday
  unsigned char runDays[7] = {0, 0, 0, 0, 0, 0, 0};

  // array of pointers to valves
  Valve *valves[NUMBER_OF_VALVES];

  // count of valves
  unsigned char valveCount = 0;

  // 1=program is running
  unsigned char status = 0;

  // currently used valve
  unsigned char currentValve = 0;

  // valve runtime multiplier, e.g. 120 = 120% valve runtime
  unsigned char intensity = 100;

  // callback to be called when program starts
  program_cb_t onStart = NULL;

  // callback to be called when program stops
  program_cb_t onStop = NULL;

  // Homie node
  HomieNode *homie = NULL;

public:
  /**
   * Create new irrigation program. Program are scheduling valves.
   *
   * @param id - program identified. Used as Homie node identifier.
   */
  Program(unsigned char id);

  // returns program identifier
  const char *getIdStr() { return idStr; };

  // Starts program.
  void start();

  // Stops program.
  void stop();

  // IMPORTANT: Program internal loop. Must be called in the main loop.
  void loop();

  // Sets program start time
  void setStart(unsigned char hour, unsigned char minute)
  {
    startHour = hour;
    startMin = minute;
  };

  // Returns programs start hour.
  unsigned char getStartHour() { return startHour; };

  // Returns program start minute.
  unsigned char getStartMinute() { return startMin; };

  /**
   * Sets runtime for specified valve.
   *
   * @param valve - valve number
   * @param runtime - valve runtime in seconds
   */
  unsigned char setRunTime(unsigned char valve, unsigned int runtime);

  // Sets runtime of specified valve (in minutes)
  unsigned char setRunTimeMin(unsigned char valve, unsigned int runtime) { return setRunTime(valve, runtime * 60); };

  // Returns runtime of specified valve (in seconds)
  unsigned int getRunTime(unsigned char valve);

  // Returns runtime of specified valve (in minutes)
  unsigned int getRunTimeMin(unsigned char valve) { return getRunTime(valve) / 60; };

  /**
   * Toggles specified day of week.
   *
   * @param day - day of week. Sunday = 1, Monday = 2, ...
   * @param status - true = program runs on that day of week
   */
  unsigned char setRunDay(unsigned char day, bool status);

  // Returns true if program runs on specified day of week
  unsigned char getRunDay(unsigned char day);

  // Sets program intensity
  void setIntensity(unsigned char i) { intensity = i; };

  // Sets callback for program on-start event
  void setOnStartCB(program_cb_t cb) { onStart = cb; };

  // Sets callback for program on-stop event
  void setOnStopCB(program_cb_t cb) { onStop = cb; };

  // Determine if program should start on specified datetime
  unsigned char shouldStart(time_t datetime);

  // Returns true if program is currently running
  unsigned char isRunning() { return status; };

  // Adds valve to program
  void addValve(Valve *valve);

  /**
   * Returns true if program is configured.
   * Configured means that program runs at least once per week
   * and at least one valve runtime is non-zero.
   */
  unsigned char isConfigured();

  void printConfig();

  // Set Homie node representing this program
  void setHomie(HomieNode *node) { homie = node; }

  // Return Homie node representing this program
  inline HomieNode *getHomie() { return homie; }
};
