#pragma once




#define MIT_AUDLINK 1

#ifdef MIT_AUDLINK
#include <Arduino.h> 
#include <AudioStream.h> // github.com/PaulStoffregen/cores/blob/master/teensy4/AudioStream.h

class AudioInputTPLink : public AudioStream
{
public:
        AudioInputTPLink() : AudioStream(0, NULL) {  }

        
        virtual void update(void);

        void fireBlock(audio_block_t * block);



private:
        /*static audio_block_t *block_left;
        static uint16_t block_offset;
        static int32_t hpf_y1;
        static int32_t hpf_x1;

        static bool update_responsibility;
        static DMAChannel dma;
        static void isr(void);
        static void init(uint8_t pin);
        */

};
#endif

void as_linkaudio_setup();
void as_linkaudio_beat();


