
#pragma once

#include <Arduino.h>
#include "USBHost_t36.h"






void as_usbhost_setup();
void as_usbhost_beat();
 
/*für MIDI Client*/
void as_usbmidiclient_setup();
void as_usbmidiclient_beat();

 

#define AS_MIDI_FORWARD 1
#define AS_KEYB_FORWARD 1
#define AS_MOUSE_FORWARD 1

#ifdef UMSETZUNG
void as_usbhost_OnKeyPress(uint8_t key,uint8_t modifier,uint8_t oemkey,uint8_t leds);


void as_usbhost_mouse(uint8_t buttons, int x,int y,int wheel,int wheelh);

void as_usbhost_jogdial(uint32_t buttons, int rotation);

void as_usbhost_OnNoteOn(uint8_t channel, uint8_t note, uint8_t velocity);

void as_usbhost_OnNoteOff(uint8_t channel, uint8_t note, uint8_t velocity);

void as_usbhost_OnControlChange(uint8_t channel, uint8_t control, uint8_t value);

void as_usbhost_OnPitchBendChange(uint8_t channel, int16_t value);

void as_usbhost_OnProgramChange(uint8_t channel, uint8_t value);

void as_usbhost_OnSysExMessage(byte* a,unsigned int sizeofsysex);

void as_usbhost_OnSysExMessage(byte* a,unsigned int sizeofsysex);

void as_usbhost_OnSysExMessage_1(byte* a,unsigned int sizeofsysex,bool complete);

/*----------------------------------------------------------------------*/

void as_usbhost_SendNoteOff(uint8_t note, uint8_t velocity, uint8_t channel);
void as_usbhost_SendNoteOn(uint8_t note, uint8_t velocity, uint8_t channel);
void as_usbhost_SendPitchBend(int value, uint8_t channel);
void as_usbhost_SendProgramChange(uint8_t program, uint8_t channel);

void as_usbhost_SendControlChange(uint8_t channel, uint8_t control, uint8_t value);
/*inArrayContainsBoundaries = true inArray = F0 a b ... x y F7, ansonsten a b ... x y */
void as_usbhost_SendSysEx(unsigned int inLength,  const uint8_t* inArray, bool inArrayContainsBoundaries);

/* TODO
void as_usbhost_SendProgramChange(DataByte inProgramNumber,  Channel inChannel);

*/

#endif


