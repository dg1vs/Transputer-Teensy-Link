/*
   Teensy Linkadapter V3
   22.02.2026
   Teensy Linkadapter V2
   06.09.2025 Andre saischowa
   erweiterte Version
   -B300 kompatiblität
   - Audio   <-> Link
   - USB-Hub <-> Link

 
 */

#include "tplinkconf.h"

#ifdef MIT_OLEDDISP

  #include "as_tpdisplay.h"

#endif

#include "as_ethernet.h"

#include <LibPrintf.h>

#include "as_c011.h"
#include "as_teensylink.h"

#include "as_usbhub.h"
#include "as_linkusb.h"
#include "as_linkaudio.h"

/*void as_linkIO_init();*/



asTransputerLinkBase * tpLinkMain = NULL;

asTransputerLinkBase * tpLinkUSB = NULL;




void setup() {

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  delay(2000);
  /*
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  */
  Serial.println("*********************************");
  Serial.println("TransputerLink Adapter V3.01 (C011, TeensyLink, B300-Emu, Audio, USB-Host)");
  Serial.println("Andre Saischowa 2026");
  Serial.print( F("Compiled: "));
  Serial.print( F(__DATE__));
  Serial.print( F(", "));
  Serial.println( F(__TIME__));
  //Serial.print( F(", "));
  //Serial.println( F(__VERSION__)); 


#ifdef MIT_OLEDDISP

  disp_init();
  disp_writeLine("TransputerLink V3.01",0);
  disp_flush();

#endif

#ifdef MIT_LINK_C011

  tpLinkMain = new asTpLinkC011(TP_RESET, TP_ANALYSE, TP_ERROR);
  
  #ifdef MIT_USB_HOST
    #ifdef MIT_LINK_TEENSY
      tpLinkUSB = new asTpLinkTeensy(&LINK_TEENSY_SERIAL,-1,-1,-1);
    #endif
  #endif

#else

  tpLinkMain = new asTpLinkTeensy(&LINK_TEENSY_SERIAL,TP_RESET, TP_ANALYSE, TP_ERROR);

#endif

#ifdef MIT_USB_HOST
if( tpLinkUSB == NULL){
  tpLinkUSB = tpLinkMain;
}
#endif
  
#ifdef AS_ETHERNET
  as_eth_setup();
#endif  


#ifdef MIT_USB_HOST  
  as_usbhost_setup();

#endif
#ifdef MIT_USBMIDI_CLIENT
  as_usbmidiclient_setup();
#endif
#ifdef MIT_LINKAUDIO
  as_linkaudio_setup();
#endif
  Serial.println("*********************************");
}


void loop(){

  #ifdef MIT_USB_HOST  
    if(tpLinkUSB) {
      #ifdef MIT_USB_HOST  
        as_usbhost_beat();
      #endif
      #ifdef MIT_USBMIDI_CLIENT  
        as_usbmidiclient_beat();
      #endif
    }
  #endif  

  #ifdef AS_ETHERNET  
    as_eth_beat();
  #endif  

  #ifdef MIT_USB_HOST

    #ifdef MIT_LINKAUDIO
      if(tpLinkUSB) {
        as_linkaudio_beat();
      }
    #endif


    #ifdef AS_PAKET_READ
      if(tpLinkUSB) {
        handleLinkInData(tpLinkUSB);
      }
    #endif
  
  #endif
}

