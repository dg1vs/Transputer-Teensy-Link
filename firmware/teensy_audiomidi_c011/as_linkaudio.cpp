


#include "as_linkaudio.h"
#include <Audio.h>

/*#include <Arduino.h>

#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
*/

#define MIT_SGTL5000 1


#ifdef MIT_AUDLINK
void AudioInputTPLink::update(){


}
void AudioInputTPLink::fireBlock(audio_block_t * block){
   if(block){
    	transmit(block);
	    release(block);
   }
}

#endif



#ifdef MIT_AUDLINK
AudioInputTPLink         tplink;          //xy=149,142
AudioOutputI2S           i2s1;           //xy=386,137
AudioConnection          patchCord1(tplink, 0, i2s1, 0);
AudioConnection          patchCord2(tplink, 0, i2s1, 1);

AudioControlSGTL5000 sgtl5000;

#elif MIT_SGTL5000
// GUItool: begin automatically generated code
AudioSynthWaveformSine   sine1;          //xy=149,142
AudioOutputI2S           i2s1;           //xy=386,137
AudioConnection          patchCord1(sine1, 0, i2s1, 0);
AudioConnection          patchCord2(sine1, 0, i2s1, 1);
// GUItool: end automatically generated code

AudioControlSGTL5000 sgtl5000;

#else


// GUItool: begin automatically generated code
AudioInputI2S            i2s1;           //xy=181,791
AudioSynthWaveformSine   sine1;          //xy=194,148
AudioOutputI2S           i2s2;           //xy=431,887
AudioOutputUSB           usb1;           //xy=469,148
AudioConnection          patchCord1(i2s1, 1, i2s2, 0);
AudioConnection          patchCord2(sine1, 0, usb1, 0);
AudioConnection          patchCord3(sine1, 0, usb1, 1);
#endif




void as_linkaudio_setup(){
#ifdef  MIT_AUDLINK
  AudioMemory(20);

  sgtl5000.enable();
  sgtl5000.volume(0.32);  


  Serial.println("MIT_AUDLINK ok");
#elif MIT_SGTL5000
  
  AudioMemory(20);

  sgtl5000.enable();
  sgtl5000.volume(0.32);  

  sine1.amplitude(0.75);
  sine1.frequency(440);
  Serial.println("MIT_SGTL5000 ok");


#else

  /*
  AudioMemory(20);
  sine1.amplitude(0.75);
  sine1.frequency(440);
  */
#endif  
}
void as_linkaudio_beat(){
   

}
