#include <time.h>
#include <Arduino.h>

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