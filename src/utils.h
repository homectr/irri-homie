#include <time.h>
#include <Arduino.h>
#include <Timezone.h>
#include "Program.h"
#include "Valve.h"

/**
 * Stores date-time formatted as ISO string to provided buffer
 * 
 * @param buffer destimation buffer
 * @param size size of destination buffer - min. 11 bytes
 * @param datetime date-time
 * @param includeTime if true, then time part will be included
 * @param tz time-zone suffix, only used with time part
 * 
 * @return 1 if no error ocurred, 0 if error ocurred
 */
int dt2ISO(char* buffer, size_t size, time_t datetime, bool includeTime, const char* tz);

// return current ISO date-time string
const char* nowStr();
const char* nowStr(const char* tzAbbrev);

// return true/false string
const char* boolStr(bool value);

// return time-zone offset
Timezone* determineTimeZone(const char* tzName);

// find program by id
Program* findProgramById(const char* id);

// find valve by id
Valve* findValveById(const char* id);