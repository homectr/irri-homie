#pragma once

#include <Arduino.h>
#include <Homie.h>

bool handleSysIntensity(const HomieRange& range, const String& value);
bool handleSysDT(const HomieRange& range, const String& value);