#pragma once

#include <Arduino.h>
#include <Homie.h>
#include <Valve.h>

void onValveOpen(Valve *valve);
void onValveClose(Valve *valve);

bool handleValveStatus(unsigned char valveIdx, const String &value);
bool handleValveRT(const HomieRange &range, const String &value);