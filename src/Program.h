#ifndef __PROGRAM_H__
#define __PROGRAM_H__

#include <time.h>
#include "config.h"
#include "Valve.h" 

class Program {

    private:
        unsigned char id;
        unsigned char startHour = 7;
        unsigned char startMin = 0;
        unsigned char runTimes[NUMBER_OF_VALVES] = {10,10,10,10,10,10};
        unsigned char runDays[7] = {1,1,1,1,1,1,1};
        
        Valve valves[];

        unsigned char status = 0; 
        unsigned char currentValve = 0;
        unsigned char intensity = 0;

    private:
        unsigned char calendarStart();

    public:
        Program(unsigned char id, Valve valves[]);
        void start();
        void stop();
        void loop();
        void setStart(unsigned char hour, unsigned char minute);
        void setRunTimes(const char* runtimes);
        void setRunDays(const char* runDays);
        void setIntensity(unsigned char intensity);
};

#endif