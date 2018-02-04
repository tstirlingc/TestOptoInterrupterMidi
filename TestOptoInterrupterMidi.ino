#include <MIDI.h>
#include "Bounce2.h"

#define MidiPin_TX 0
#define MidiPin_RX 1
#define BEGIN_NOTE 48
#define MIDI_CHANNEL 1
int program = 22; // accordion
#define PROGRAM_CHANGE_UPDATE 10000 // 10 seconds

struct MySettings : public midi::DefaultSettings
{
  static const long BaudRate = 115200;
};

MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial, MIDI, MySettings);

//MIDI_CREATE_DEFAULT_INSTANCE();
//SoftwareSerial midiSerial(MidiPin_RX, MidiPin_TX);
//MIDI_CREATE_INSTANCE(SoftwareSerial, midiSerial, MIDI);
#define NUM_BUTTONS 4
int note[NUM_BUTTONS];
int buttonStatus[NUM_BUTTONS];
#define PROGRAM_SWITCH_UP 14 // Chnages MIDI program, starting at 22=accordion
#define PROGRAM_SWITCH_DOWN 15 
Bounce debouncerUP = Bounce();
Bounce debouncerDOWN = Bounce();

// itr9608
// OptoInterrupter
//   output is attached to pin 6,7 with 12k pull-down resistor
// LED output is attached to pin 2,3,4,5
// Power to OptoInterrupters is on pin 10,11 through 2n3094 transistors
#define OptoPowerBank1 10

int OptoRead[NUM_BUTTONS] = {6, 7, 8, 9}; // Opto interrupter read pins on Arduino
int LED[NUM_BUTTONS] = {2, 3, 4, 5}; // pins for LEDs on Arduino

void setup() {
    pinMode(MidiPin_TX, OUTPUT);
    digitalWrite(MidiPin_TX, LOW);
    pinMode(MidiPin_RX, INPUT); 
    for (int i=0 ; i<NUM_BUTTONS ; ++i) {
      note[i] = BEGIN_NOTE + i;
    }
    pinMode(OptoPowerBank1, OUTPUT);
    digitalWrite(OptoPowerBank1, LOW);

    for (int i = 0 ; i < NUM_BUTTONS ; ++i) {
      pinMode(OptoRead[i], INPUT_PULLUP);
      pinMode(LED[i], OUTPUT);
      digitalWrite(LED[i], LOW);
      buttonStatus[i] = 0;
    }
    pinMode(PROGRAM_SWITCH_UP, INPUT_PULLUP);
    pinMode(PROGRAM_SWITCH_DOWN, INPUT_PULLUP);
    debouncerUP.attach(PROGRAM_SWITCH_UP);
    debouncerUP.interval(20);
    debouncerDOWN.attach(PROGRAM_SWITCH_DOWN);
    debouncerDOWN.interval(20);
    
    MIDI.begin(MIDI_CHANNEL);
    MIDI.sendProgramChange(static_cast<byte>(program), MIDI_CHANNEL);
}

void readOptoAndDisplay(int optoRead, int LEDid)
{
  if (digitalRead(optoRead) == LOW) {
    digitalWrite(LED[LEDid], HIGH);
    if (buttonStatus[LEDid] == 0) {
      MIDI.sendNoteOn(note[LEDid], 127, MIDI_CHANNEL);
      buttonStatus[LEDid] = 1;
    }
  } else {
    digitalWrite(LED[LEDid], LOW);
    if (buttonStatus[LEDid] == 1) {
      MIDI.sendNoteOff(note[LEDid], 0, MIDI_CHANNEL);
      buttonStatus[LEDid] = 0;
    }
  }
}

void loop() {
  digitalWrite(OptoPowerBank1, HIGH);
  delayMicroseconds(300);
  for (int i=0 ; i<NUM_BUTTONS ; ++i) {
    readOptoAndDisplay(OptoRead[i], i);
  }
  digitalWrite(OptoPowerBank1, LOW);
  debouncerUP.update();
  if (debouncerUP.read() == LOW) { 
    program++;
    if (program > 128) { program = 0; }
    MIDI.sendProgramChange(static_cast<byte>(program), MIDI_CHANNEL);
  }
  debouncerDOWN.update();
  if (debouncerDOWN.read() == LOW) {
    program--;
    if (program < 0) { program = 128; }
    MIDI.sendProgramChange(static_cast<byte>(program), MIDI_CHANNEL);
  }
}
