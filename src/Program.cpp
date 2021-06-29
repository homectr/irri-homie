#include <Arduino.h>
#include "Program.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#define NODEBUG_PRINT
#include "debug_print.h"

Program::Program(unsigned char id){
    this->id = id;

    char is[20];
    snprintf(is,20,"prg%d",id);
    idStr = strdup(is);

    valveCount = 0;

    // set default program run-time to 0 min
    for(int i=0; i < NUMBER_OF_VALVES; i++) {
        runTimes[i] = 0;
    }

    currentValve = 0;
    status = 0;
}

void Program::start(){
    if (valveCount == 0) {
        CONSOLE_PGM(PSTR("Error: no valves defined for program '%s'\n"),idStr);
        return;
    }
    DEBUG_PRINT("Starting program '%s'. Intensity %d\n",idStr, intensity);
    status = 1;
    currentValve = 0;
    if (onStart) onStart(this);

    // open the first valve having non-zero run time
    for (int v=0; v < valveCount;v++)
        if (runTimes[v]>0) {
            currentValve = v;
            valves[v]->open(runTimes[v], intensity);
            break;
        }
}

void Program::stop(){
    status = 0;
    currentValve = 0;
    DEBUG_PRINT("Stopping program '%s'\n", this->getName());
    // close all valves on program stop
    for (int i = 0; i < valveCount;i++) 
        valves[i]->close();
    if (onStop) onStop(this);

}

/**
 * returns 1 or 0 if program should start on provided datetime
 */
unsigned char Program::shouldStart(time_t datetime) {
    return runDays[weekday(datetime)-1] 
        && hour(datetime) == startHour 
        && minute(datetime) == startMin;
}

void Program::loop(){

    DEBUG_PRINT("[loop] Program %s\n",getName());

    // if program is not running 
    if (status == 0) {
        time_t dt = now();
        // and if program should start according to planned days & times
        DEBUG_PRINT("Checking program '%s'\n", getName());
        if (shouldStart(dt)) {
            DEBUG_PRINT("Start\n");    
            start();
        } else {
            DEBUG_PRINT("Program wont start - not in calendar %d\n",dt);
        }

        return;
    }

    // if program is running and valve is open, then do nothing
    if (valves[currentValve]->isOpen()) return;

    DEBUG_PRINT("Program '%s' current valve %d.\n", getName(), currentValve);

    // if program is running and currentValve is not open means valve reached its runtime
    // get and open next valve
    do {
        currentValve++;
        DEBUG_PRINT("cv=%d rt=%d\n",currentValve, runTimes[currentValve]);
    } while (currentValve < valveCount && runTimes[currentValve] == 0);
    
    DEBUG_PRINT("Program '%s' next valve %d.\n", getName(), currentValve);
    if (currentValve < valveCount) {
        valves[currentValve]->open(runTimes[currentValve], intensity);
    } else {
        // stop program if all valves have been cycled
        stop();
    }
}

unsigned char Program::setRunDay(unsigned char day, bool status){
    DEBUG_PRINT("[setRunDay] prg=%d day=%d rd=%d\n",id,day,status);
    if (day>=7) return 0;
    runDays[day]=status;
    return 1;
}

unsigned char Program::setRunTime(unsigned char valve, unsigned int runtime){
    DEBUG_PRINT("[setRunTime] prg=%d valve=%d rt=%d\n",id,valve,runtime);
    if (valve >= valveCount) return 0;
    runTimes[valve] = runtime*60;
    return 1;
}

void Program::addValve(Valve *valve){
    DEBUG_PRINT("[addValve] prg=%d  count=%d id=%d\n",id, valveCount,valve->getIdStr());
    if (valveCount>=NUMBER_OF_VALVES) return;
    valves[valveCount] = valve;
    valveCount++;
}

void Program::printConfig(){
    CONSOLE("Program %d name='%s': start=%02d:%02d ",
        id,
        idStr,
        getStartHour(), 
        getStartMinute()
    );
    CONSOLE(" rt=");
    for (int i=0; i < valveCount; i++) CONSOLE("%d,",getRunTime(i));
    CONSOLE(" rd=");
    for (int i=0; i<7; i++) CONSOLE("%d,", getRunDay(i));
    CONSOLE("\n");

}

unsigned int Program::getRunTime(unsigned char valve){
    DEBUG_PRINT("[getRT] prg=%d cnt=%d valve=%d\n",id, valveCount, valve);
    return valve<valveCount ? runTimes[valve]/60 : 0;
};

unsigned char Program::getRunDay(unsigned char day){
    DEBUG_PRINT("[getRD] prg=%d day=%d\n",id,day);
    return day<7 ? runDays[day] : 0;
};

unsigned char Program::isConfigured(){
    unsigned char c = 0;
    for (int i=0; i<7; i++) c |= runDays[i];
    if (c) {
        c = 0;
        for (int i=0; i<valveCount; i++) c |= runTimes[i] > 0;
    }
    return c;
}