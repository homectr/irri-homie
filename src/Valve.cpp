#include <Arduino.h>
#include "Valve.h"

Valve::Valve(unsigned char id){
    this->id = id;
}

void Valve::close(){
    status = 0;
    if (onClose) onClose(id);
}

void Valve::open(unsigned int minutes){
    if (minutes == 0) { 
        close();
        return;
    }
    runtime = (minutes > 120 ? 120 : minutes) * 60000;
    openedAt = millis();
    status = 1;
    if (onOpen) onOpen(id);
}

void Valve::loop(){
    if (status == 0) return;
    if (millis() - this->openedAt > this->runtime) this->close();
}