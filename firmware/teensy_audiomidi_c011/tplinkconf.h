#pragma once

//#define  MIT_LINK_C011
#define  MIT_LINK_TEENSY 

#define LINK_TEENSY_SERIAL  Serial7 /* Rx 28, Tx 29 */

/* Pins für Steuersignale*/
#define TP_ERROR    32
#define TP_ANALYSE  31
#define TP_RESET    30

/*für B300, WWW-Server*/
#define AS_ETHERNET 1
#define AS_B300_MAIN_PORT 4047

#define MIT_USB_HOST 1

#define MIT_USBMIDI_CLIENT 1

#define MIT_LINKAUDIO_NO 1

#define AS_VIEW_LINKPAKET_NO 1
#define AS_BYPASSTEST_NO 1

//erst mal nicht, problem mit endlosschleife auf teensylink !!!
#define AS_PAKET_READ_NO 1


#define  MIT_OLEDDISP 1

#ifdef MIT_OLEDDISP
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

#endif
