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

    name = strdup("Program  ");
    *(name+8)=id+48;  // add {id} after "Program"

    this->valveCount = 0;

    // set default program run-time to 0 min
    for(int i=0;i<NUMBER_OF_VALVES;i++) this->runTimes[i] = 0;

    currentValve = 0;
    status = 0;
}

void Program::setName(const char* name){
    free(this->name);
    this->name = strdup(name);
}

void Program::start(){
    if (valveCount == 0) {
        CONSOLE_PGM(PSTR("Error: no valves defined for program '%s'\n"),getName());
        return;
    }
    DEBUG_PRINT("Starting program '%s'. Intensity %d\n",this->getName(), intensity);
    status = 1;
    currentValve = 0;
    if (onStart) onStart(id);

    // open the first valve having non-zero run time
    for (int v=0; v < valveCount;v++)
        if (runTimes[v]>0) {
            currentValve = v;
            valves[v]->open(runTimes[v]*60, intensity);
            break;
        }
}

void Program::stop(){
    status = 0;
    currentValve = 0;
    DEBUG_PRINT("Stopping program '%s'\n", this->getName());
    // close all valves on program stop
    for (char i = 0; i < valveCount;i++) 
        valves[i]->close();
    if (onStop) onStop(id);

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
        valves[currentValve]->open(runTimes[currentValve]*60, intensity);
    } else {
        // stop program if all valves have been cycled
        stop();
    }
}

unsigned char Program::setRunDay(unsigned char day, bool status){
    if (day>=7) return 0;
    runDays[day]=status;
    return 1;
}

unsigned char Program::setRunTime(unsigned char valve, unsigned char runtime){
    if (valve >= valveCount) return 0;
    runTimes[valve] = runtime;
    return 1;
}

void Program::addValve(Valve *valve){
    if (valveCount>=NUMBER_OF_VALVES) return;
    valves[valveCount] = valve;
    valveCount++;
}

void Program::printConfig(){
    DEBUG_PRINT("Program config '%s': rt=%s rd=%s sh=%d sm=%d\n",
        getName(),
        getStartHour(), 
        getStartMinute()
    );
}
