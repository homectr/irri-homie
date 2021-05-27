#pragma once

#include <TimeLib.h>
#include "settings.h"
#include "Valve.h" 

using program_cb_t = void(*)(unsigned char programId);

class Program {

    private:
        // program identifier
        unsigned char id;

        // program name - default is "Program {id}"
        char* name;

        // program starting hour
        unsigned char startHour = 6;

        // program starting minute
        unsigned char startMin = 0;

        // how long (in seconds) should each valve be open
        unsigned int runTimes[NUMBER_OF_VALVES];

        // which days of a week should program run, starting with Sunday
        unsigned char runDays[7] = {0,0,0,0,0,0,0};

        // array of pointers to valves
        Valve* valves[NUMBER_OF_VALVES];

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

    public:
        Program(unsigned char id);
        void start();
        void stop();
        void loop();
        void setStart(unsigned char hour, unsigned char minute){startHour = hour; startMin = minute;};
        unsigned char setRunTime(unsigned char valve, unsigned int runtime);
        unsigned char setRunDay(unsigned char day, bool status);
        void setIntensity(unsigned char i){intensity = i;};
        void setOnStartCB(program_cb_t cb){onStart = cb;};
        void setOnStopCB(program_cb_t cb){onStop = cb;};
        void setName(const char *name);
        const char* getName(){return name;};
        unsigned int getRunTime(unsigned char valve){return valve<valveCount ? runTimes[valve] : 0;};
        unsigned char getRunDay(unsigned char day){return day<7 ? runDays[day] : 0;};
        unsigned char getStartHour(){return startHour;};
        unsigned char getStartMinute(){return startMin;};
        unsigned char shouldStart(time_t datetime);
        unsigned char isRunning(){return status;};
        void addValve(Valve* valve);
        void printConfig();

};
