#include "Program.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

Program::Program(unsigned char id, Valve** v, unsigned char valveCount){
    this->id = id;

    name = strdup("Program  ");
    *(name+8)=id+48;  // add {id} after "Program"

    *valves = *v;

    this->valveCount = valveCount > NUMBER_OF_VALVES ? NUMBER_OF_VALVES : valveCount;

    for(unsigned char i=0;i<this->valveCount;i++) this->runTimes[i] = 0;
}

void Program::setName(const char* name){
    free(this->name);
    this->name = strdup(name);
}

void Program::start(){
    if (valveCount == 0) return;
    status = 1;
    currentValve = 0;
    if (onStart) onStart(id);
    valves[0]->open(runTimes[0]*(intensity/100));
}

void Program::stop(){
    status = 0;
    currentValve = 0;

    // close all valves on program stop
    for (char i = 0; i<NUMBER_OF_VALVES;i++) valves[i]->close();

    if (onStop) onStop(id);
}

unsigned char Program::calendarStart() {
    return now() > disabledTill 
        && runDays[weekday()-1] 
        && hour() == startHour 
        && minute() == startMin;
}

void Program::loop(){

    // if program is not running 
    if (status == 0) {
        if (calendarStart()) start();
        return;
    }

    // if program is running and valve is open, then do nothing
    if (valves[currentValve]->isOpen()) return;

    // if program is running and currentValve is not open means valve reached its runtime
    // get and open next valve
    currentValve++;
    if (currentValve < NUMBER_OF_VALVES) {
        valves[currentValve]->open(runTimes[currentValve]*(intensity/100));
    } else {
        // stop program if all valves have been cycled
        stop();
    }
}

unsigned char Program::setRunDays(const char* runDays){
    unsigned i=0;
    while (i<7 && *(runDays+i)) {
        this->runDays[i] = *(runDays+i)=='1'?1:0;
        i++;
    }
    return 1;
}

unsigned char Program::setRunTimes(const char* runTimes){

    char* rt = strdup(runTimes);
    char* rs=rt;
    char* re=rt;
    for (unsigned char i=0;i<NUMBER_OF_VALVES;i++){
        rs = re;
        while (*re && *re != ',') re++; // find next colon or end of string
        if (re != rs) {
            char* nn;
            long t = strtol(rs, &nn, 10);
            if (errno == ERANGE) {
                free(rt);
                return 0;
            }
        }
    }
    free(rt);
    return 1;
}

int Program::getRunDays(char* buffer, size_t size){
    if (size < 8) return 0;
    for (int i=0;i<7;i++){
        buffer[i]=this->runDays[i]?'1':'0';
    }
    buffer[7]=0;
}

int Program::getRunTimes(char* buffer, size_t size){
    size_t pos = 0;
    for(int i=0;i<NUMBER_OF_VALVES;i++){
        pos += snprintf(buffer+pos, size-pos, "%d, ", runTimes[i]);
    }
}