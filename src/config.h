#pragma once

#define LOCAL_CONFIG_FILE   "/homie/settings.json"

int initFS();
int loadConfig();
int saveConfig();


