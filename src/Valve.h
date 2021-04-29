#pragma once

using valve_cb_t = void(*)(unsigned char valveId);

class Valve {
    private:
        // valve identifier
        unsigned char id;

        // valve status
        unsigned char status = 0;

        // valve opened at
        unsigned long openedAt = 0;

        // valve runtime in seconds
        unsigned int runtime = 0;

        // valve default runtime
        unsigned int defRunTime = 0;

        valve_cb_t onOpen = NULL;
        valve_cb_t onClose = NULL;

    public:
        Valve(unsigned char id);

        // open valve for specified time (in seconds)
        // valve closes automatically afterwards
        void open(unsigned int seconds);

        // open valve for time specified in default runtime
        void open(){open(defRunTime);};

        // close valve
        void close();

        // is valve open?
        unsigned char isOpen(){return status == 1;};

        // valve handler - has to be called
        void loop();

        // set valve onOpen callback
        void setOnOpenCB(valve_cb_t cb){onClose = cb;};

        // set valve onClose callback
        void setOnCloseCB(valve_cb_t cb){onOpen = cb;};

        // set valve default runtime
        void setRunTime(unsigned int seconds){defRunTime = seconds;};

        // get default runtime for manual start
        unsigned int getRunTime(){return defRunTime;};

};