#include <Homie.h>

const unsigned int NUMBER_OF_SWITCHES = 6;
const unsigned int NUMBER_OF_PROGRAMS = 2;
const unsigned char GPIOS[NUMBER_OF_SWITCHES] = { 16, 5, 4, 14, 12, 13 };
const String opts = String("on ON 1 off OFF 0");

struct IriProgram {
    char* id;
    
};

HomieNode node("irrix6", "IrriHomie x6", "irrigation", true, 1, NUMBER_OF_SWITCHES);

bool switchHandler(const HomieRange& range, const String& value) {
    if (!range.isRange) return false;  // if it's not a range

    if (range.index < 1 || range.index > NUMBER_OF_SWITCHES) return false;  // if it's not a valid range

    int8_t i = opts.indexOf(value);
    if (0 > i) return false;  // if the value is not valid
  
    bool on = (i<8); // off starts after 7th position

    digitalWrite(GPIOS[range.index - 1], on ? HIGH : LOW);
    node.setProperty("switch").setRange(range).send(value);  // Update the state of the led
    Homie.getLogger() << "Switch " << range.index << " is " << value << endl;

    return true;
}

void setup() {
  for (uint8_t i = 0; i < NUMBER_OF_SWITCHES; i++) {
    pinMode(GPIOS[i], OUTPUT);
    digitalWrite(GPIOS[i], LOW);
  }

  Serial.begin(115200);
  Serial << endl << endl;

  Homie_setFirmware("Switchx5", "1.0.0");

  node.advertise("switch").setName("Switch").setDatatype("boolean").settable(switchHandler);

  Homie.setup();
}

void loop() {
  Homie.loop();

}