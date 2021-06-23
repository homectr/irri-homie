#pragma once

#include <Homie.h>

class Valve;

using valve_cb_t = void(*)(Valve* valve);

class Valve {
    protected:
        // valve identifier
        unsigned char id;

        // if inverse, then status==1 => closed
        unsigned char inverse = 0;

        // valve identifier for mqtt
        char *idStr = NULL;

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

        HomieNode* homie;

    public:
        Valve(unsigned char id, unsigned char inverse=0);

        /**
         * Opens valve for specified time (in seconds). 
         * Valve closes automatically afterwards
         * 
         * @param seconds - number of seconds to keep valve open (max. 7200)
         */
        virtual void open(unsigned int seconds, unsigned char intensity);

        inline void open(unsigned int seconds){open(seconds, (unsigned char)100);};

        // open valve for time specified in default runtime
        inline void open(){open(defRunTime, (unsigned char)100);};

        // close valve
        virtual void close();

        // is valve open?
        unsigned char isOpen(){return status;};

        // valve handler - has to be called
        virtual void loop();

        // set valve onOpen callback
        void setOnOpenCB(valve_cb_t cb){onOpen = cb;};

        // set valve onClose callback
        void setOnCloseCB(valve_cb_t cb){onClose = cb;};

        // set valve default runtime
        void setRunTime(unsigned int seconds){defRunTime = seconds;};

        // get default runtime for manual start
        unsigned int getRunTime(){return defRunTime;};

        const char* getIdStr(){return idStr;};

        unsigned char isInverse(){return inverse;};

        void setHomie(HomieNode *node){homie = node;}
        HomieNode* getHomie(){return homie;}

};

class GPIOValve: public Valve {
    protected:
        unsigned char gpio;

    public:
        GPIOValve(unsigned char id, unsigned char gpio, unsigned char inverse=0);
        virtual void open(unsigned int seconds, unsigned char intensity);
        virtual void close();
};