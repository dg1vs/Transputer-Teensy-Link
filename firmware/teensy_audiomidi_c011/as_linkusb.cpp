
#include "as_linkusb.h"
#include "as_transputerlink.h"

#include "as_linkaudio.h"
#include <AudioStream.h> 
#ifdef ASUSBLINK_IMPLEMENTIERUNG

extern asTransputerLinkBase * tpLinkUSB;
/*extern bool isB300 Connected();*/

extern ASUSBLink usbLinkHandler;

asTransputerLinkBase * ASUSBLink::getLink(){
   return tpLinkUSB;
}

extern void as_usbhost_SendControlChange(uint8_t channel, uint8_t control, uint8_t value);
extern void as_usbhost_SendSysEx(unsigned int inLength,  const uint8_t* inArray, bool inArrayContainsBoundaries = false);

extern void as_usbhost_SendNoteOff(uint8_t note, uint8_t velocity, uint8_t channel);
extern void as_usbhost_SendNoteOn(uint8_t note, uint8_t velocity, uint8_t channel);
extern void as_usbhost_SendPitchBend(int value, uint8_t channel);
extern void as_usbhost_SendProgramChange(uint8_t program, uint8_t channel);

const int16_t vsData[2][128] = {

  {  32688, 32672, 32544, 32368, 32096, 31744, 31328, 30800, 30224, 29584, 28864, 28080, 27216, 26288, 25296, 24272
, 23152, 22000, 20784, 19504, 18176, 16832, 15440, 14000, 12544, 11056, 9536, 8000, 6432, 4848, 3280, 1664
, 64, -1536, -3152, -4736, -6320, -7888, -9424, -10960, -12448, -13904, -15344, -16752, -18112, -19424, -20720, -21936
, -23088, -24208, -25232, -26224, -27152, -28016, -28800, -29520, -30176, -30752, -31280, -31712, -32064, -32336, -32528, -32656
, -32688, -32656, -32528, -32320, -32064, -31712, -31280, -30784, -30208, -29568, -28848, -28064, -27200, -26272, -25280, -24240
, -23120, -21984, -20768, -19504, -18208, -16848, -15472, -14032, -12560, -11072, -9536, -7984, -6432, -4848, -3264, -1664
, -64, 1536, 3152, 4720, 6304, 7872, 9440, 10976, 12464, 13936, 15376, 16768, 18128, 19424, 20704, 21920
, 23056, 24176, 25216, 26208, 27136, 28000, 28784, 29504, 30160, 30736, 31232, 31680, 32032, 32304, 32496, 32640
}
, {  0, 8272, 11392, 15648, 18944, 19248, 20672, 19184, 16576, 18960, 19456, 17520, 19104, 20064, 20912, 23232
, 21280, 19520, 20848, 17792, 13952, 13536, 12736, 15008, 19104, 20176, 24544, 30592, 31296, 31920, 32752, 31104
, 31440, 29968, 25536, 25968, 27072, 25360, 26656, 28032, 28688, 31248, 29152, 24496, 23552, 20096, 14864, 12896
, 10560, 10384, 12816, 11120, 10512, 13360, 12288, 10288, 9024, 5152, 3472, 1920, -3472, -4560, -2224, -2128
, 0, 2128, 2224, 4560, 3472, -1920, -3472, -5152, -9024, -10288, -12288, -13360, -10512, -11120, -12816, -10384
, -10560, -12896, -14864, -20096, -23552, -24496, -29152, -31248, -28688, -28032, -26656, -25360, -27072, -25968, -25536, -29968
, -31440, -31104, -32752, -31920, -31296, -30592, -24544, -20176, -19104, -15008, -12736, -13536, -13952, -17792, -20848, -19520
, -21280, -23232, -20912, -20064, -19104, -17520, -19456, -18960, -16576, -19184, -20672, -19248, -18944, -15648, -11392, -8272
}
};

int ctrlInbuf[(8192)]; /*Pakete sind maximal 8192  groß*/

void handleLinkInData(asTransputerLinkBase * link){

  if(/*!isB300 Connected() &&*/ link && !link->isOpen() && link->hasInByte()){

    char * lName = "SteuerLnkIn";
    /*hier  können asynchron Daten empfangen werden*/
     if(link->OpenLink(lName)==1){
        while(1){
          int i;
          char recByte[2];
          int len;
          /*Header checken, erst einmal nur PKT6*/
          int ret=link->ReadLink(recByte,1,1);
          if(ret!=1 || recByte[0]!='P') break;
          ret=link->ReadLink(recByte,1,1);
          if(ret!=1 || recByte[0]!='K') break;
          ret=link->ReadLink(recByte,1,1);
          if(ret!=1 || recByte[0]!='T') break;          
          ret=link->ReadLink(recByte,1,1);
          if(ret!=1 || recByte[0]!='6') break;
          /* Header OK, Länge einlesen*/
          ret=link->ReadLink(recByte,2,1);
          if(ret!=2) break;
          len = recByte[0] | (recByte[1] << 8);
          /*printf("ret = %d ,%d %d len %d\r\n",ret,recByte[0],recByte[1],len);*/
          ret=link->ReadLink((char *)ctrlInbuf,len,10);
          /*printf("gelesen %d %08x\r\n",ret,ctrlInbuf[0]);*/

           switch(ctrlInbuf[0]){
              case TPPAYLOAD_MIDI: {
                    tp_midi_t * m= (tp_midi_t*)ctrlInbuf;
                    /* printf("Midi kommt -- ");
                    printf(" kennung %04x ",m->kennung);
                    printf(" typ %d ",m->typ);
                    printf(" chan %d ",m->chan);
                    printf(" data1 %d ",m->data1);
                    printf(" data2 %d\r\n",m->data2);
                    */


                    switch(m->typ){
                      case TPMIDI_NOTEON:         as_usbhost_SendNoteOn(m->data1, m->data2, m->chan);        break;
                      case TPMIDI_NOTEOFF:        as_usbhost_SendNoteOff(m->data1, m->data2, m->chan);       break;                        
                      case TPMIDI_CCHANGE:        as_usbhost_SendControlChange(m->chan, m->data1, m->data2); break;
                      case TPMIDI_PITCHBEND:      as_usbhost_SendPitchBend(m->data1, m->chan);               break;
                      case TPMIDI_PROGRAMCHANGE:  as_usbhost_SendProgramChange(m->data1, m->chan);           break;
                      case TPMIDI_SYSEXMESSAGE:{
                        /*etwas tricky, gültig ist nur kennung und typ, chan,data1,data2 werden ignoriert, dann kommen die sysexdaten
                        anzahl ist payload -sizeof(paloadgesamtlen)*/
                              char * iBuf = (char*)ctrlInbuf;
                              int sysexlen= len-sizeof(tp_midi_t);
                              iBuf += sizeof(tp_midi_t);
                              
                              /*printf("PKT6 sysex kommt len %d\r\n",len);
                              for(i=0;i < sysexlen;i++) printf("  %d ",iBuf[i]);
                              printf("\r\n");
                              for(i=0;i < sysexlen;i++) printf("0x%02x ",iBuf[i]);
                              printf("\r\n");*/

                              as_usbhost_SendSysEx(sysexlen,iBuf,(iBuf[0]== 0xF0 && iBuf[sysexlen-1]== 0xF7));
                        } 
                        break; 
                      default:
                        printf("Midi kommt -- Typ unbekannt \r\n");
                        printf(" kennung %04x ",m->kennung);
                        printf(" typ %d ",m->typ);
                        printf(" chan %d ",m->chan);
                        printf(" data1 %d ",m->data1);
                        printf(" data2 %d\r\n",m->data2);
                        break;
                    }
                    
                    if(m->typ == TPMIDI_CCHANGE){
                      
                      
                    }else if(m->typ == TPMIDI_SYSEXMESSAGE){


                    }
                    /*https://github.com/gljubojevic/akai-mpk-mini-editor */
                    /*TPMIDI_SYSEXMESSAGE*/


                  } 
                  break;
              case TPPAYLOAD_AUDIOPKT: {
                  tp_audio_t * m= (tp_audio_t*)ctrlInbuf;
                    printf("audiopaket kommt typ %d\r\n",m->typ);
                    
                    /*
                    audio_block_t *block;
                  	
                    block = allocate();
                    if(block){
                      memcpy(block->data,m->sampleBlock,AUDIO_BLOCK_SAMPLES);
                    }
                    */
                  }  
                  break; 
              default: {
                  char * iBuf = (char*)ctrlInbuf;
                  printf("PKT6 Unbekannt len %d\r\n",len);
                  for(i=0;i < len;i++) printf("  %d ",iBuf[i]);
                  printf("\r\n");
                  for(i=0;i < len;i++) printf("0x%02x ",iBuf[i]);
                  printf("\r\n");
                }
                break;
           }

          break;

        }
        link->CloseLink();


     }
  }
}

void ASUSBLink::sendPayLoad(char * payload,int payloadsize){
    char * lName = "SteuerLnkOut";
   if(/*!isB300 Connected() && */getLink() && !getLink()->isOpen()){
    if(getLink()->OpenLink(lName)==1){
        /*transputer spricht Little Endian*/
        uint32_t kennung = BYTESWAP32(TPPACKET_HEADER_TYP6); /*0x6c727463;*/  /*ctrl in Little endian*/
        /*char pl= payloadsize/4;  */
        uint8_t pl[2];
        int l;
        pl[0] = (uint8_t)payloadsize & 0xFF;
        pl[1] = (uint8_t)(payloadsize >>8) & 0xFF;
        #ifdef AS_VIEW_LINKPAKET
        int i;
                

          printf("sendPayLoad Size: %d %x   %02x:%02x\r\n ",payloadsize,payloadsize,pl[1],pl[0]);   
        #endif

        getLink()->adapterReset(); 
        
        l = getLink()->WriteLink((char *)&kennung,4,10);
        if(l==4){
            #ifdef AS_VIEW_LINKPAKET
              if(l!=4 ) printf("StartkennungFehler %d von 4\r\n",l);  
            #endif

            l = getLink()->WriteLink(pl,sizeof(pl),10);
            
            #ifdef AS_VIEW_LINKPAKET
              if(l!= sizeof(pl))printf("Lenfehler %d von 1 \r\n",l);  
            #endif
            
            #ifdef AS_VIEW_LINKPAKET
              for(i=0;i <payloadsize;i++){
                printf(" %02x",payload[i]);
              }
              printf("\r\n");
            #endif

            l = getLink()->WriteLink(payload,payloadsize,10);

            
              if(l!=payloadsize){
                printf("Fehler %d %d\r\n",l,payloadsize);
              }
            #ifdef AS_VIEW_LINKPAKET
              printf("sendPayLoad durch \r\n ");   
            #endif
            usb_payloadpakets++;
            /* printf("Pay %d von pl %d\r\n",l,pl);  */  
        }else{

        }

          
        getLink()->CloseLink();
    }    
   }


}

void ASUSBLink::OnNoteOn(uint8_t channel, uint8_t note, uint8_t velocity){

   tp_midi_t payload = { PLAYLOAD_MIDI_INIT(TPMIDI_NOTEON,channel,note,velocity) };
   sendPayLoad((char *) &payload,sizeof(payload));
   //printf("NoteOn chan: %d note: %d velocity: %d\r\n",channel,note,velocity);




}


void ASUSBLink::OnNoteOff(uint8_t channel, uint8_t note, uint8_t velocity){

   tp_midi_t payload = { PLAYLOAD_MIDI_INIT(TPMIDI_NOTEOFF,channel,note,velocity) };
   sendPayLoad((char *) &payload,sizeof(payload));  

   //printf("NoteOff chan: %d note: %d velocity: %d\r\n",channel,note,velocity);
}
int16_t aBuf[128];


void ASUSBLink::OnControlChange(uint8_t channel, uint8_t control, uint8_t value){
   if(control==20){
     if(value>0){
        memcpy((char*)aBuf,(char*)vsData[1],256);
        audioPktSend((char*)aBuf,sizeof(aBuf));
     }
   } else{
      tp_midi_t payload = { PLAYLOAD_MIDI_INIT(TPMIDI_CCHANGE,channel,control,value) };
      sendPayLoad((char *) &payload,sizeof(payload));   
   }

   /*printf("ControlChange chan: %d control: %d value: %d\r\n",channel,control,value);*/
}
void ASUSBLink::OnPitchBendChange(uint8_t channel, int16_t value){

   tp_midi_t payload = { PLAYLOAD_MIDI_INIT(TPMIDI_PITCHBEND ,channel,(uint8_t)(value & 0xFF),(uint8_t)(value >> 8 & 0xFF)) };
   /*sendPayLoad((char *) &payload,sizeof(payload)); */  
   /*int16_t v = (payload.data1 + (payload.data2 << 8));*/
   /*printf("as_usbhost_OnPitchBendChange chan: %d  value: %d  %x %x %d\r\n",channel,value,payload.data1,payload.data2,v);*/
   sendPayLoad((char *) &payload,sizeof(payload));

   /*printf("ControlChange chan: %d control: %d value: %d\r\n",channel,control,value);*/
}

void ASUSBLink::OnProgramChange(uint8_t channel, uint8_t value){
    tp_midi_t payload = { PLAYLOAD_MIDI_INIT(TPMIDI_PROGRAMCHANGE ,channel,value,0) };
   sendPayLoad((char *) &payload,sizeof(payload));   
}

void ASUSBLink::OnSysExMessage(uint8_t* sysexdata,unsigned int sysexlen){
      /*  printf("Sysex mit %d bytes\r\n",sizeofsysex);*/
    int gesamtlen = (sysexlen+sizeof(tp_midi_t));
    char * payload = malloc(gesamtlen);
    /*printf("Sysex mit %d bytes\r\n",sysexlen);*/
    if(payload){
      tp_midi_t * head = (tp_midi_t * )payload;
      head->kennung = TPPAYLOAD_MIDI;      
      head->typ = TPMIDI_SYSEXMESSAGE;
      head->chan = 0;
      head->data1 = 0;
      head->data2 = 0;
      /*
      tp_midi_t head = { PLAYLOAD_MIDI_INIT(TPMIDI_SYSEXMESSAGE,0,0,0) };

      memcpy(payload,&head,sizeof(tp_midi_t));
      */
      
      memcpy(payload+sizeof(tp_midi_t),sysexdata,sysexlen);

      sendPayLoad(payload,gesamtlen);

      free(payload); 
   }
}


void ASUSBLink::OnKeyPress(uint8_t key,uint8_t modifier,uint8_t oemkey,uint8_t leds){
  printf("KeyPress key: %d mod: %x oemkey: %x led: %x\r\n",key, modifier, oemkey, leds);
}

void ASUSBLink::mouse(uint8_t buttons, int x,int y,int wheel,int wheelH){
   tp_mouse_t payload = { PLAYLOAD_MOUSE_INIT(buttons,  x, y, wheel, wheelH) };
   sendPayLoad((char *) &payload,sizeof(payload));

   /*printf("mouse butt: %x x: %d y: %d w: %d wh: %d\r\n",buttons,x,y,wheel,wheelH);*/
}

void ASUSBLink::jogdial(uint32_t buttons, int rotation){
    tp_jogDial_t payload = { PLAYLOAD_JOGDIAL_INIT(buttons,rotation) };
   sendPayLoad((char *) &payload,sizeof(payload));
}

void ASUSBLink::audioPktSend(char * sampleBuf,int sampleLen){
   /*tp_audio_t payload = { PLAYLOAD_AUDIOPKT_INIT(815) };
   printf("AudioPaket feuert %x %d\r\n",payload.kennung,payload.typ);
   sendPayLoad((char *) &payload,sizeof(payload)); */
   
    int gesamtlen = (sampleLen+sizeof(tp_audio_t));
    char * payload = malloc(gesamtlen);
    uint16_t * sampleBuf16 = (uint16_t *)sampleBuf;
    /*printf("Sysex mit %d bytes\r\n",sysexlen);*/
    if(payload){
      tp_audio_t * head = (tp_audio_t * )payload;
      head->kennung = TPPAYLOAD_AUDIOPKT;  
      head->typ = 0;    
      head->typ |= TP_AUDIO_TYP_L;
      head->sequence    = 42;
      head->samplestamp = 96;

      /*
      tp_midi_t head = { PLAYLOAD_MIDI_INIT(TPMIDI_SYSEXMESSAGE,0,0,0) };

      memcpy(payload,&head,sizeof(tp_midi_t));
      */
      #ifdef NIX_CODE
      for(int i=0;i < 128;i++){
         sampleBuf16[i] =/*BYTESWAP16*/(vsData[0][i]); 
         /*if(i % 16 == 0) printf("\r\n");
          printf("%04x %04x # ",vsData[0][i],sampleBuf16[i]);*/
        
      }
      #endif
     

      #ifdef NIX_CODE
      printf("\r\n");
      for(int i=0;i < 256;i++){

         if(i % 16 == 0) printf("\r\n");
        printf("%02x ",sampleBuf[i]);
        
      }

      /*memcpy(sampleBuf,(char*)vsData[0],256);*/
      printf("\r\n");
      #endif
      memcpy(payload+sizeof(tp_audio_t),sampleBuf,sampleLen);

      sendPayLoad(payload,gesamtlen);

      free(payload); 
   }

}

#endif