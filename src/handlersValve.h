#pragma once

#include <Arduino.h>
#include <Homie.h>

void onValveOpen(unsigned char valveId, unsigned char inverse);
void onValveClose(unsigned char valveId, unsigned char inverse);

bool handleValveStatus(unsigned char valveIdx, const String& value);
bool handleValveRT(const HomieRange& range, const String& value);