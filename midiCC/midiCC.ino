#include "MIDIUSB.h"               // arduino midiUSB
#include <Modi.h>                  // Include Modi Library from GITHUB REPO: https://github.com/brendan-byrne/Modi

#define numberOfRows 2 
Modi Matrix(2, 3, 4, 1);           // Modi Matrix(A, B, C, toggleSmoothing)  
                                   // toggleSmoothing = 0 : Smoothing OFF
                                   // toggleSmoothing = 1 : Smoothing ON


int lastReading[numberOfRows][muxChannels];   // Check new readings against these values to determine whether or not to transmit MIDI value

byte midiVals[numberOfRows][muxChannels] = {  // This array represents CC and Note values for a Hybri PCB. Add more rows if you have more boards
  {20, 21, 22, 23, 24, 25, 26, 27},           // Pots    - CC Values
  {48, 49, 50, 51, 52, 53, 54, 55}            // Buttons - Note Values
};



// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).


void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}

// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}


void setup() {   
  Matrix.attach(A0, "pot");        
  Matrix.attach(A1, "button");  
  
  Matrix.smoothAmount = .5;           // If toggleSmoothing = 1 : Adjust amount of smoothing
                                      // .999 = More Smoothing  : .001 = Less Smoothing
  Serial.begin(115200);               // Begin Serial communication
}

void loop() {
  Matrix.refresh();                                                               // REQUIRED TO RECEIVE UPDATED READINGS
  for (int x = 0; x < Matrix.numMux; x++) {                                       // Loop through the rows
    for (int y = 0; y < muxChannels; y++) {                                       // Loop through the columns
      int reading = Matrix.getReading(x, y);                                         
      if (Matrix.analogType[x]) {                                                 // Matrix.analogType stores whether the row is an analog or digital reading. 0 = digital and 1 = analog
        if (reading > lastReading[x][y] + 7 || reading < lastReading[x][y] - 7) { // New reading is only accepted if it's different enough. Keeps readings very steady
          lastReading[x][y] = reading;                                            
          reading = map(reading, 0, 1023, 0, 127);                                // Map readings to MIDI friendly range
          controlChange(1, midiVals[x][y], reading); 
          MidiUSB.flush();
          //Serial.println(reading);
        }
      } 
      else if (reading != lastReading[x][y]) {                                    // If reading is different from last reading then proceed
        if (reading == HIGH) noteOn(1, midiVals[x][y], 65);
        if (reading == LOW) noteOff(1, midiVals[x][y], 0);
        MidiUSB.flush();
        //Serial.println(midiVals[x][y]);
        //Serial.println(reading);
        lastReading[x][y] = reading;
      }
    }
  }
}



