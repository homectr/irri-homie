#ifndef __VALVE_H__
#define __VALVE_H__

class Valve {
    private:
        unsigned char pin;
        unsigned char status;

        size_t startedAt;
        uint16_t runtime;

    public:
        Valve(unsigned char pin);
        void open(unsigned int seconds);
        void close();
        unsigned char isOpen();
        void loop();
}

#endif