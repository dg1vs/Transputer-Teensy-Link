

#include "Arduino.h"

/*
Protokoll teensy <> transputer
-blockweise

1.Channelprotokoll

Controlpakete 
  
  4 byte        startkennung  "ctrl"
  2 byte        len           payload in bytes
  x...8192 byte payload

  Payload
  int kennung z.B.'midi'
  ...

*/

/*transputer spricht Little Endian*/
#define BYTESWAP32(z) ((uint32_t)((z&0xFF)<<24|((z>>8)&0xFF)<<16|((z>>16)&0xFF)<<8|((z>>24)&0xFF)))
#define BYTESWAP16(z) ((uint16_t)((z&0xFF)<<8) | ((z>>8)&0xFF));

enum {
  TPPACKET_HEADER_TYP6=0x504b5436 , /* PKT6 P,K,T = Synchronisierung,6=es folgen 2 Byte Payloadlen*/
  TPPACKET_HEADER_TYP8=0x504b5438, /*PKT8 P,K,T = Synchronisierung,6=es folgen 2 Byte Payloadlen,dann ein Byte Sourcenode,ein byte Destinationnode,maximal 255 Nodes*/
  TPPACKET_HEADER_TYPC=0x504b5443 /*PKTC P,K,T = Synchronisierung,6=es folgen 2 Byte Payloadlen,dann ein 2 Byte Sourcenode,2 byte Destinationnode, maximal 65535 Nodes*/
};




/*enum {
  TPPAYLOAD_UNBEKANNT=0,
  TPPAYLOAD_MIDI='MIDI',
  TPPAYLOAD_MOUSE='MOUS',  
  TPPAYLOAD_JOGDIAL='JOGD'
};
*/
enum {
  TPPAYLOAD_UNBEKANNT = 0, 
  TPPAYLOAD_MIDI      = 0x4d494449,  /* MIDI */ 
  TPPAYLOAD_MOUSE     = 0x4d4f5553,  /* MOUS */ 
  TPPAYLOAD_KEYB      = 0x4b455942,  /* KEYB */  
  TPPAYLOAD_JOGDIAL   = 0x4a4f4744,  /* JOGD */  
  TPPAYLOAD_JOYPAD    = 0x4a4f5950,  /* JOYP */  
  TPPAYLOAD_AUDIOPKT  = 0x41554450,  /* AUDP */ 
  TPPAYLOAD_ETHPKT    = 0x45544850  /* ETHP */ 
};







/*
typedef struct tp_pkt6_head{
  uint32_t kennung;
  uint16_t payloadlen;  
}tp_pkt6_head_t;

typedef struct tp_pkt8_head{
  uint32_t kennung;
  uint16_t payloadlen; 
  uint8_t sourcenode; 
  uint8_t destnode;   
}tp_pkt8_head_t;

typedef struct tp_pktc_head{
  uint32_t kennung;
  uint16_t payloadlen; 
  uint16_t sourcenode; 
  uint16_t destnode;   
}tp_pktc_head_t;
*/
enum {
 TPMIDI_UNBEKANNT     =0,
 TPMIDI_NOTEON        =1,
 TPMIDI_NOTEOFF       =2, 
 TPMIDI_CCHANGE       =3,
 TPMIDI_PITCHBEND     =4,
 TPMIDI_PROGRAMCHANGE =5,
 TPMIDI_SYSEXMESSAGE = 6  
};

typedef struct tp_midi{
  uint32_t kennung;
  uint8_t typ;
  uint8_t chan;  
  uint8_t data1;
  uint8_t data2; 
  /*immer aufpassen wegen Stuffbytes*/       
}tp_midi_t;



#define PLAYLOAD_MIDI_INIT(T,C,N,V)   TPPAYLOAD_MIDI,T,C,N,V


typedef struct tp_mouse{
  uint32_t kennung;
  int button;
  int x;  
  int y;
  int wheel; 
  int wheelh;   
  /*immer aufpassen wegen Stuffbytes*/       
}tp_mouse_t;
#define PLAYLOAD_MOUSE_INIT(B,X,Y,W,WH)   TPPAYLOAD_MOUSE,B,X,Y,W,WH

typedef struct tp_jogDial{
  uint32_t kennung;
  uint32_t button;
  int32_t rotation;  

  /*immer aufpassen wegen Stuffbytes*/       
}tp_jogDial_t;
#define PLAYLOAD_JOGDIAL_INIT(B,R)   TPPAYLOAD_JOGDIAL,B,R


enum TP_AUDIO_TYP{
    TP_AUDIO_TYP_L        = 0b00000000000000000000000000000100, /*nur daten linker kanal*/
    TP_AUDIO_TYP_R        = 0b00000000000000000000000000001000, /*nur daten rechter Kanal*/
    TP_AUDIO_TYP_LR       = 0b00000000000000000000000000001100, /*Daten für R und L*/   
    TP_AUDIO_TYP_LR_MASK  = 0b00000000000000000000000000001100  
};
typedef struct tp_audio{
  uint32_t kennung;
  uint32_t typ;
  uint32_t sequence;
  uint32_t samplestamp;  
  /*immer aufpassen wegen Stuffbytes*/       
}tp_audio_t;
#define PLAYLOAD_AUDIOPKT_INIT(T,S,STAMP)   TPPAYLOAD_AUDIOPKT,T,S,STAMP

#define ASUSBLINK_IMPLEMENTIERUNG

#ifdef ASUSBLINK_IMPLEMENTIERUNG

class asTransputerLinkBase;

class ASUSBLink {

  public:
    ASUSBLink(){usb_payloadpakets=0; }

    void OnKeyPress(uint8_t key,uint8_t modifier,uint8_t oemkey,uint8_t leds);


    void mouse(uint8_t buttons, int x,int y,int wheel,int wheelh);

    void jogdial(uint32_t buttons, int rotation);

    void OnNoteOn(uint8_t channel, uint8_t note, uint8_t velocity);

    void OnNoteOff(uint8_t channel, uint8_t note, uint8_t velocity);

    void OnControlChange(uint8_t channel, uint8_t control, uint8_t value);

    void OnPitchBendChange(uint8_t channel, int16_t value);

    void OnProgramChange(uint8_t channel, uint8_t value);

    void OnSysExMessage(uint8_t* a,unsigned int sizeofsysex);

    void audioPktSend(char * sampleBuf,int sampleLen);

protected:
    void sendPayLoad(char * payload,int payloadsize);    
    asTransputerLinkBase * getLink();
public:
    int usb_payloadpakets;
};

void handleLinkInData(asTransputerLinkBase * link);
#endif
