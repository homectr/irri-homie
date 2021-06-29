#pragma once

#include <Homie.h>

class Valve;

using valve_cb_t = void(*)(Valve* valve);

class Valve {
    protected:
        // valve identifier
        unsigned char id;

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
        /**
         * Create valve
         * 
         * @param id - valve identifier - used as Homie parameter
         */
        Valve(unsigned char id);

        /**
         * Opens valve for specified time (in seconds) multiplied by intensity. 
         * Valve closes automatically afterwards
         * 
         * @param seconds - number of seconds to keep valve open (max. 7200)
         * @param intensity - percentage seconds multiplier: 0-200; 100=100%
         */
        virtual void open(unsigned int seconds, unsigned char intensity);

        /**
         * Opens valve for specified time (in seconds). 
         * Intensity is 100.
         * Valve closes automatically afterwards
         * 
         * @param seconds - number of seconds to keep valve open (max. 7200)
         */
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

        // set valve default runtime in seconds
        void setRunTime(unsigned int seconds){defRunTime = seconds;};

        // set valve default runtime in minutes
        void setRunTimeMin(unsigned int minutes){defRunTime = minutes*60;};

        // get default runtime for manual start (in seconds)
        unsigned int getRunTime(){return defRunTime;};

        // get default runtime for manual start (in minutes)
        unsigned int getRunTimeMin(){return defRunTime/60;};

        // get valve identifier
        const char* getIdStr(){return idStr;};

        // set Homie node representing for this valve
        void setHomie(HomieNode *node){homie = node;}
        HomieNode* getHomie(){return homie;}

};

/**
 * GPIO/pin managed valve
 */
class GPIOValve: public Valve {
    protected:
        // gpio managing valve
        unsigned char gpio;

        // if inverse, then gpio set to 1 will close valve
        unsigned char inverse = 0;

    public:
        /**
         * Create GIOValve object.
         * 
         * @param id - valve id, used as Homie node identifier
         * @param gpio - gpio number
         * @param inverse - gpio will be handled in inverse mode. Default = 0/no
         */
        GPIOValve(unsigned char id, unsigned char gpio, unsigned char inverse=0);

        virtual void open(unsigned int seconds, unsigned char intensity);

        virtual void close();

        // is valve in inverse mode?
        unsigned char isInverse(){return inverse;};

};