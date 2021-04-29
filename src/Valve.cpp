#include "Valve.h"

Valve::Valve(uint8_t pin){
    this->pin = pin;
    pinMode(pin,OUTPUT);

    this->status = 0;
}

void Valve::close(){
    digitalWrite(this->pin, 0);
    this->status = 0;
}

unsigned char Valve::isOpen(){
  return status==1;
}

void Valve::open(uint16_t seconds){
    if (seconds == 0) { 
        this->close();
        return;
    }
    digitalWrite(this->pin, 1);
    this->runtime = seconds;
    this->startedAt = millis();
    this->status = 1;
}

void Valve::loop(){
    if (status == 0) return;
    if (millis() - this->startedAt > this->runtime) this->close()
}