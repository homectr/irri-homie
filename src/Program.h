#pragma once

#include <TimeLib.h>
#include "settings.h"
#include "Valve.h" 

using program_cb_t = void(*)(unsigned char programId);

class Program {

    private:
        // program identifier
        unsigned char id;

        // program identifer for mqtt
        char* idStr = NULL;

        // program name - default is "Program {id}"
        char* name = NULL;

        // program starting hour
        unsigned char startHour = 6;

        // program starting minute
        unsigned char startMin = 0;

        // how long (in seconds) should each valve be open
        unsigned int runTimes[NUMBER_OF_VALVES];

        // array containing valve names - for Homie
        char* valveNames[NUMBER_OF_VALVES];

        // which days of a week should program run, starting with Sunday
        unsigned char runDays[7] = {0,0,0,0,0,0,0};

        // array containing runday names - for Homie
        char* runDayNames[7] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL};

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

        const char* getIdStr(){return idStr;};

        void start();
        void stop();
        void loop();

        void setName(const char *name);
        const char* getName(){return name;};

        void setStart(unsigned char hour, unsigned char minute){startHour = hour; startMin = minute;};
        unsigned char getStartHour(){return startHour;};
        unsigned char getStartMinute(){return startMin;};

        unsigned char setRunTime(unsigned char valve, unsigned int runtime);
        unsigned int getRunTime(unsigned char valve);

        void setValveName(unsigned char valve, const char* name);
        const char* getValveName(unsigned char valve){return valve<NUMBER_OF_VALVES?valveNames[valve]:NULL;};

        unsigned char setRunDay(unsigned char day, bool status);
        unsigned char getRunDay(unsigned char day);

        void setRunDayName(unsigned char day, const char* name);
        const char* getRunDayName(unsigned char day);

        void setIntensity(unsigned char i){intensity = i;};
        void setOnStartCB(program_cb_t cb){onStart = cb;};
        void setOnStopCB(program_cb_t cb){onStop = cb;};
        
        unsigned char shouldStart(time_t datetime);
        unsigned char isRunning(){return status;};

        void addValve(Valve* valve);

        void printConfig();

        

};
