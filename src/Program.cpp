#include <TimeLib.h>
#include "Program.h"


void Program::start(){
    this->status = 1;
    this->currentValve = 0;
    this->valves[0].open(this->runTimes[0]*(this->intensity/100));
}

void Program::stop(){
    this->status = 0;
    this->currentValve = 0;
    for (char i = 0; i<NUMBER_OF_VALVES;i++) valves[i].close();
}

unsigned char Program::calendarStart() {
    
    if (hour() == )
}

void Program::loop(){

    // if program is not running 
    if (status == 0) {
        if (this->calendarStart()) start();
        return;
    }

    // if program is running and valve is open, then do nothing
    if (valves[this->currentValve].isOpen()) return;

    // if program is running and no valve is open
    // get and open next valve
    this->currentValve++;
    if (this->currentValve < NUMBER_OF_VALVES) {
        valves[this->currentValve].open(this->runTimes[this->currentValve]*(this->intensity/100));
    } else {
        this->stop()
    }
}