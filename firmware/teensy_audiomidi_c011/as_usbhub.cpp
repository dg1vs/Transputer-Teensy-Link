
#include "as_usbhub.h"
#include "as_linkusb.h"
#include "as_ethernet.h"

/*#include "HIDDumper.h"*/

ASUSBLink usbLinkHandler;


void as_usbhost_OnKeyPress(uint8_t key,uint8_t modifier,uint8_t oemkey,uint8_t leds){
   usbLinkHandler.OnKeyPress( key, modifier, oemkey, leds);
}


void as_usbhost_mouse(uint8_t buttons, int x,int y,int wheel,int wheelh){
   usbLinkHandler.mouse( buttons,  x, y, wheel, wheelh);
}

void as_usbhost_jogdial(uint32_t buttons, int rotation){
   usbLinkHandler.jogdial( buttons,  rotation);
}

void as_usbhost_OnNoteOn(uint8_t channel, uint8_t note, uint8_t velocity){
   usbLinkHandler.OnNoteOn( channel,  note,  velocity);
}

void as_usbhost_OnNoteOff(uint8_t channel, uint8_t note, uint8_t velocity){
   usbLinkHandler.OnNoteOff( channel,  note,  velocity);
}

void as_usbhost_OnControlChange(uint8_t channel, uint8_t control, uint8_t value){
   usbLinkHandler.OnControlChange( channel,  control,  value);
}

void as_usbhost_OnPitchBendChange(uint8_t channel, int16_t value){
   usbLinkHandler.OnPitchBendChange( channel,  value);
}

void as_usbhost_OnProgramChange(uint8_t channel, uint8_t value){
   usbLinkHandler.OnProgramChange( channel,  value);
}   

void as_usbhost_OnSysExMessage(byte* a,unsigned int sizeofsysex){
   usbLinkHandler.OnSysExMessage( a,  sizeofsysex);
}   

void as_usbhost_OnSysExMessage_1(byte* a,unsigned int sizeofsysex,bool complete){
  /*int i;
  printf(">>>>>>>>>>>as_usbhost_OnSysExMessage_1 %d %d\r\n",sizeofsysex,complete);
  for(i=0;i < sizeofsysex;i++){
    printf("0x%02x ",a[i]);

  }
  printf("<<<<<<<<<<\r\n");
  */
  usbLinkHandler.OnSysExMessage( a,  sizeofsysex);
}



void as_usbmidiclient_setup(){
  /*für MIDI-Client */
  usbMIDI.setHandleNoteOff(as_usbhost_OnNoteOff);
  usbMIDI.setHandleNoteOn(as_usbhost_OnNoteOn);
  usbMIDI.setHandleControlChange(as_usbhost_OnControlChange);
  usbMIDI.setHandlePitchChange(as_usbhost_OnPitchBendChange);
  usbMIDI.setHandleProgramChange(as_usbhost_OnPitchBendChange);
  usbMIDI.setHandleSystemExclusive(as_usbhost_OnSysExMessage);
}  

void as_usbmidiclient_beat(){
  usbMIDI.read();
}





class ASKeyboardController : public KeyboardController {

public:
    ASKeyboardController(USBHost &host) : KeyboardController(host) { }
    ASKeyboardController(USBHost *host) : KeyboardController(host) { }
    void     attachGriffinEvent(void (*f)(uint32_t button, uint32_t rotation)) { griffinEvent = f; }
protected:
    virtual void hid_input_begin(uint32_t topusage, uint32_t type, int lgmin, int lgmax){

      topusage_as = topusage;
      topusage_type_as = type;

      KeyboardController::hid_input_begin(topusage,type,lgmin,lgmax);

    }
    virtual void hid_input_data(uint32_t usage, int32_t value){

      while(1){
        /*Griffin PowerMate filtern*/
         if(idVendor() != 0x77d) break;

         if(idProduct() != 0x410) break;
         /*topusage filtern*/
         if(topusage_as != 0xc0000) break;
        
         switch(usage){
           case 0x90001: griffin_button= value;   break;
           case 0x10033: griffin_rotation= value; break;
           default:

            break;
         }
         return;
      }
      KeyboardController::hid_input_data(usage,value);


    }   
    virtual void hid_input_end(){

      while(1){
        /*Griffin PowerMate filtern*/
         if(idVendor() != 0x77d) break;
         if(idProduct() != 0x410) break;
         /*topusage filtern*/
         if(topusage_as != 0xc0000) break;
         
         if(griffinEvent){
            griffinEvent(griffin_button,griffin_rotation);
         }else{
            printf("End Griffin PowerMate B: %x R: %d\r\n",griffin_button,griffin_rotation);
         }
         break;
      }      
      KeyboardController::hid_input_end();
      
    }     


protected:
    uint32_t topusage_as = 0;                 // What top report am I processing?
    uint32_t topusage_type_as = 0; 
    uint32_t griffin_button;
    int32_t griffin_rotation;
    void (*griffinEvent)(uint32_t button, int32_t rotation) = NULL;

    


       
};    
/*
KeyboardController: topusage= c0000 usage=10033, value=-1
KeyboardController: topusage= c0000 usage=90001, value=0

Raus hid_input_end c0000 vendor 77d product 410

*** HID Device KB1 77d:410 - connected ***
  manufacturer: Griffin Technology, Inc.
  product: Griffin PowerMate

*/
USBHost myusb;
USBHub hub1(myusb);
USBHub hub2(myusb);
USBHub hub3(myusb);

ASKeyboardController keyboard1(myusb);
ASKeyboardController keyboard2(myusb);
USBHIDParser hid1(myusb);
USBHIDParser hid2(myusb);
USBHIDParser hid3(myusb);
USBHIDParser hid4(myusb);
USBHIDParser hid5(myusb);


MouseController mouse1(myusb);
JoystickController joystick1(myusb);
//BluetoothController bluet(myusb, true, "0000");   // Version does pairing to device
BluetoothController bluet(myusb);   // version assumes it already was paired
int user_axis[64];
uint32_t buttons_prev = 0;
RawHIDController rawhid1(myusb);
RawHIDController rawhid2(myusb, 0xffc90004);


/*HID(1 : 10001): 02 03 72 06 50 03 00 00 
END:
  Begin topusage:d0000 type:2 min:0 max:1
    asidx[29] usage=D0042, value=1 
    asidx[30] usage=D0032, value=1 
    Begin topusage:d0000 type:2 min:0 max:2047
    asidx[31] usage=10030, value=1650 (X)
    Begin topusage:d0000 type:2 min:0 max:2047
    asidx[32] usage=10031, value=848 (Y)
  END:
  */

USBDriver *drivers[] = {&hub1, &hub2,&joystick1, &bluet, &hid1, &hid2, &hid3, &hid4, &hid5};
#define CNT_DEVICES (sizeof(drivers)/sizeof(drivers[0]))


const char * driver_names[CNT_DEVICES] = {"Hub1","Hub2", "JOY1D", "Bluet", "HID1" , "HID2", "HID3", "HID4", "HID5"};
bool driver_active[CNT_DEVICES] = {false, false, false, false};

// Lets also look at HID Input devices
USBHIDInput *hiddrivers[] = {&mouse1, &keyboard1, &keyboard2, &joystick1, &rawhid1, &rawhid2};
#define CNT_HIDDEVICES (sizeof(hiddrivers)/sizeof(hiddrivers[0]))


const char * hid_driver_names[CNT_DEVICES] = {"Mouse1", "KB1", "KB2", "Joystick1", "RawHid1", "RawHid2"};
bool hid_driver_active[CNT_DEVICES] = {false, false};
bool show_changed_only = false;

uint8_t joystick_left_trigger_value = 0;
uint8_t joystick_right_trigger_value = 0;
uint64_t joystick_full_notify_mask = (uint64_t) - 1;



MIDIDevice usbhost_midi1(myusb);
MIDIDevice usbhost_midi2(myusb);


void as_usbhost_SendNoteOff(uint8_t note, uint8_t velocity, uint8_t channel){
  printf("as_usbhost_SendNoteOff note %d vel %d chan %d\r\n",note,  velocity,  channel);
  usbhost_midi1.sendNoteOff( note,  velocity,  channel);
}
void as_usbhost_SendNoteOn(uint8_t note, uint8_t velocity, uint8_t channel){
  printf("as_usbhost_SendNoteOn note %d vel %d chan %d\r\n",note,  velocity,  channel);
  usbhost_midi1.sendNoteOn( note,  velocity,  channel);
}
void as_usbhost_SendPitchBend(int value, uint8_t channel){
  usbhost_midi1.sendPitchBend( value,  channel);
}

void as_usbhost_SendProgramChange(uint8_t program, uint8_t channel){
  usbhost_midi1.sendProgramChange(program,channel);
}

void as_usbhost_SendControlChange(uint8_t channel, uint8_t control, uint8_t value){
  usbhost_midi1.sendControlChange( control,  value,  channel);

  /*printf("sende midi %d %d %d\r\n",control,  value,  channel);*/

}
void as_usbhost_SendSysEx(unsigned int inLength,  const uint8_t * inArray, bool inArrayContainsBoundaries){
    usbhost_midi1.sendSysEx( inLength,   inArray,  inArrayContainsBoundaries);

}


/* TODO
void as_usbhost_SendProgramChange(Channel inChannel, DataByte inProgramNumber){
  usbhost_midi1.sendProgramChange( control,  value,  channel);


}

*/

void OnRawPress(uint8_t keycode)
{
	Serial.print("raw key press: ");
	Serial.println((int)keycode);
}

void OnRawRelease(uint8_t keycode)
{
	Serial.print("raw key release: ");
	Serial.println((int)keycode);
}

void OnNoteOn(byte channel, byte note, byte velocity)
{
	Serial.print("Note On, ch=");
	Serial.print(channel);
	Serial.print(", note=");
	Serial.print(note);
	Serial.print(", velocity=");
	Serial.print(velocity);
	Serial.println();
}

void OnNoteOff(byte channel, byte note, byte velocity)
{
	Serial.print("Note Off, ch=");
	Serial.print(channel);
	Serial.print(", note=");
	Serial.print(note);
	//Serial.print(", velocity=");
	//Serial.print(velocity);
	Serial.println();
}

void OnControlChange(byte channel, byte control, byte value)
{
	Serial.print("Control Change, ch=");
	Serial.print(channel);
	Serial.print(", control=");
	Serial.print(control);
	Serial.print(", value=");
	Serial.print(value);
	Serial.println();
}


void griffinEventFunc(uint32_t griffin_button,int32_t griffin_rotation){
  printf("GriffinEvenet %x %d\r\n",griffin_button,griffin_rotation);
}

void OnPress(int key)
{
#ifdef AS_KEYB_FORWARD
  if (keyboard1) {
    as_usbhost_OnKeyPress( key,keyboard1.getModifiers(),keyboard1.getOemKey(),keyboard1.LEDS());


  } else {
    as_usbhost_OnKeyPress( key,keyboard2.getModifiers(),keyboard2.getOemKey(),keyboard2.LEDS());
   
  }

#else  
  Serial.print("key '");
  switch (key) {
    case KEYD_UP       : Serial.print("UP"); break;
    case KEYD_DOWN    : Serial.print("DN"); break;
    case KEYD_LEFT     : Serial.print("LEFT"); break;
    case KEYD_RIGHT   : Serial.print("RIGHT"); break;
    case KEYD_INSERT   : Serial.print("Ins"); break;
    case KEYD_DELETE   : Serial.print("Del"); break;
    case KEYD_PAGE_UP  : Serial.print("PUP"); break;
    case KEYD_PAGE_DOWN: Serial.print("PDN"); break;
    case KEYD_HOME     : Serial.print("HOME"); break;
    case KEYD_END      : Serial.print("END"); break;
    case KEYD_F1       : Serial.print("F1"); break;
    case KEYD_F2       : Serial.print("F2"); break;
    case KEYD_F3       : Serial.print("F3"); break;
    case KEYD_F4       : Serial.print("F4"); break;
    case KEYD_F5       : Serial.print("F5"); break;
    case KEYD_F6       : Serial.print("F6"); break;
    case KEYD_F7       : Serial.print("F7"); break;
    case KEYD_F8       : Serial.print("F8"); break;
    case KEYD_F9       : Serial.print("F9"); break;
    case KEYD_F10      : Serial.print("F10"); break;
    case KEYD_F11      : Serial.print("F11"); break;
    case KEYD_F12      : Serial.print("F12"); break;
    default: Serial.print((char)key); break;
  }
  Serial.print("'  ");
  Serial.print(key);
  Serial.print(" MOD: ");
  if (keyboard1) {
    Serial.print(keyboard1.getModifiers(), HEX);
    Serial.print(" OEM: ");
    Serial.print(keyboard1.getOemKey(), HEX);
    Serial.print(" LEDS: ");
    Serial.println(keyboard1.LEDS(), HEX);
  } else {
    Serial.print(keyboard2.getModifiers(), HEX);
    Serial.print(" OEM: ");
    Serial.print(keyboard2.getOemKey(), HEX);
    Serial.print(" LEDS: ");
    Serial.println(keyboard2.LEDS(), HEX);
  }
#endif  

  //Serial.print("key ");
  //Serial.print((char)keyboard1.getKey());
  //Serial.print("  ");
  //Serial.print((char)keyboard2.getKey());
  //Serial.println();
}
void OnHIDExtrasPress(uint32_t top, uint16_t key)
{
  Serial.print("HID (");
  Serial.print(top, HEX);
  Serial.print(") key press:");
  Serial.print(key, HEX);
  if (top == 0xc0000) {
    switch (key) {
      case  0x20 : Serial.print(" - +10"); break;
      case  0x21 : Serial.print(" - +100"); break;
      case  0x22 : Serial.print(" - AM/PM"); break;
      case  0x30 : Serial.print(" - Power"); break;
      case  0x31 : Serial.print(" - Reset"); break;
      case  0x32 : Serial.print(" - Sleep"); break;
      case  0x33 : Serial.print(" - Sleep After"); break;
      case  0x34 : Serial.print(" - Sleep Mode"); break;
      case  0x35 : Serial.print(" - Illumination"); break;
      case  0x36 : Serial.print(" - Function Buttons"); break;
      case  0x40 : Serial.print(" - Menu"); break;
      case  0x41 : Serial.print(" - Menu  Pick"); break;
      case  0x42 : Serial.print(" - Menu Up"); break;
      case  0x43 : Serial.print(" - Menu Down"); break;
      case  0x44 : Serial.print(" - Menu Left"); break;
      case  0x45 : Serial.print(" - Menu Right"); break;
      case  0x46 : Serial.print(" - Menu Escape"); break;
      case  0x47 : Serial.print(" - Menu Value Increase"); break;
      case  0x48 : Serial.print(" - Menu Value Decrease"); break;
      case  0x60 : Serial.print(" - Data On Screen"); break;
      case  0x61 : Serial.print(" - Closed Caption"); break;
      case  0x62 : Serial.print(" - Closed Caption Select"); break;
      case  0x63 : Serial.print(" - VCR/TV"); break;
      case  0x64 : Serial.print(" - Broadcast Mode"); break;
      case  0x65 : Serial.print(" - Snapshot"); break;
      case  0x66 : Serial.print(" - Still"); break;
      case  0x80 : Serial.print(" - Selection"); break;
      case  0x81 : Serial.print(" - Assign Selection"); break;
      case  0x82 : Serial.print(" - Mode Step"); break;
      case  0x83 : Serial.print(" - Recall Last"); break;
      case  0x84 : Serial.print(" - Enter Channel"); break;
      case  0x85 : Serial.print(" - Order Movie"); break;
      case  0x86 : Serial.print(" - Channel"); break;
      case  0x87 : Serial.print(" - Media Selection"); break;
      case  0x88 : Serial.print(" - Media Select Computer"); break;
      case  0x89 : Serial.print(" - Media Select TV"); break;
      case  0x8A : Serial.print(" - Media Select WWW"); break;
      case  0x8B : Serial.print(" - Media Select DVD"); break;
      case  0x8C : Serial.print(" - Media Select Telephone"); break;
      case  0x8D : Serial.print(" - Media Select Program Guide"); break;
      case  0x8E : Serial.print(" - Media Select Video Phone"); break;
      case  0x8F : Serial.print(" - Media Select Games"); break;
      case  0x90 : Serial.print(" - Media Select Messages"); break;
      case  0x91 : Serial.print(" - Media Select CD"); break;
      case  0x92 : Serial.print(" - Media Select VCR"); break;
      case  0x93 : Serial.print(" - Media Select Tuner"); break;
      case  0x94 : Serial.print(" - Quit"); break;
      case  0x95 : Serial.print(" - Help"); break;
      case  0x96 : Serial.print(" - Media Select Tape"); break;
      case  0x97 : Serial.print(" - Media Select Cable"); break;
      case  0x98 : Serial.print(" - Media Select Satellite"); break;
      case  0x99 : Serial.print(" - Media Select Security"); break;
      case  0x9A : Serial.print(" - Media Select Home"); break;
      case  0x9B : Serial.print(" - Media Select Call"); break;
      case  0x9C : Serial.print(" - Channel Increment"); break;
      case  0x9D : Serial.print(" - Channel Decrement"); break;
      case  0x9E : Serial.print(" - Media Select SAP"); break;
      case  0xA0 : Serial.print(" - VCR Plus"); break;
      case  0xA1 : Serial.print(" - Once"); break;
      case  0xA2 : Serial.print(" - Daily"); break;
      case  0xA3 : Serial.print(" - Weekly"); break;
      case  0xA4 : Serial.print(" - Monthly"); break;
      case  0xB0 : Serial.print(" - Play"); break;
      case  0xB1 : Serial.print(" - Pause"); break;
      case  0xB2 : Serial.print(" - Record"); break;
      case  0xB3 : Serial.print(" - Fast Forward"); break;
      case  0xB4 : Serial.print(" - Rewind"); break;
      case  0xB5 : Serial.print(" - Scan Next Track"); break;
      case  0xB6 : Serial.print(" - Scan Previous Track"); break;
      case  0xB7 : Serial.print(" - Stop"); break;
      case  0xB8 : Serial.print(" - Eject"); break;
      case  0xB9 : Serial.print(" - Random Play"); break;
      case  0xBA : Serial.print(" - Select DisC"); break;
      case  0xBB : Serial.print(" - Enter Disc"); break;
      case  0xBC : Serial.print(" - Repeat"); break;
      case  0xBD : Serial.print(" - Tracking"); break;
      case  0xBE : Serial.print(" - Track Normal"); break;
      case  0xBF : Serial.print(" - Slow Tracking"); break;
      case  0xC0 : Serial.print(" - Frame Forward"); break;
      case  0xC1 : Serial.print(" - Frame Back"); break;
      case  0xC2 : Serial.print(" - Mark"); break;
      case  0xC3 : Serial.print(" - Clear Mark"); break;
      case  0xC4 : Serial.print(" - Repeat From Mark"); break;
      case  0xC5 : Serial.print(" - Return To Mark"); break;
      case  0xC6 : Serial.print(" - Search Mark Forward"); break;
      case  0xC7 : Serial.print(" - Search Mark Backwards"); break;
      case  0xC8 : Serial.print(" - Counter Reset"); break;
      case  0xC9 : Serial.print(" - Show Counter"); break;
      case  0xCA : Serial.print(" - Tracking Increment"); break;
      case  0xCB : Serial.print(" - Tracking Decrement"); break;
      case  0xCD : Serial.print(" - Pause/Continue"); break;
      case  0xE0 : Serial.print(" - Volume"); break;
      case  0xE1 : Serial.print(" - Balance"); break;
      case  0xE2 : Serial.print(" - Mute"); break;
      case  0xE3 : Serial.print(" - Bass"); break;
      case  0xE4 : Serial.print(" - Treble"); break;
      case  0xE5 : Serial.print(" - Bass Boost"); break;
      case  0xE6 : Serial.print(" - Surround Mode"); break;
      case  0xE7 : Serial.print(" - Loudness"); break;
      case  0xE8 : Serial.print(" - MPX"); break;
      case  0xE9 : Serial.print(" - Volume Up"); break;
      case  0xEA : Serial.print(" - Volume Down"); break;
      case  0xF0 : Serial.print(" - Speed Select"); break;
      case  0xF1 : Serial.print(" - Playback Speed"); break;
      case  0xF2 : Serial.print(" - Standard Play"); break;
      case  0xF3 : Serial.print(" - Long Play"); break;
      case  0xF4 : Serial.print(" - Extended Play"); break;
      case  0xF5 : Serial.print(" - Slow"); break;
      case  0x100: Serial.print(" - Fan Enable"); break;
      case  0x101: Serial.print(" - Fan Speed"); break;
      case  0x102: Serial.print(" - Light"); break;
      case  0x103: Serial.print(" - Light Illumination Level"); break;
      case  0x104: Serial.print(" - Climate Control Enable"); break;
      case  0x105: Serial.print(" - Room Temperature"); break;
      case  0x106: Serial.print(" - Security Enable"); break;
      case  0x107: Serial.print(" - Fire Alarm"); break;
      case  0x108: Serial.print(" - Police Alarm"); break;
      case  0x150: Serial.print(" - Balance Right"); break;
      case  0x151: Serial.print(" - Balance Left"); break;
      case  0x152: Serial.print(" - Bass Increment"); break;
      case  0x153: Serial.print(" - Bass Decrement"); break;
      case  0x154: Serial.print(" - Treble Increment"); break;
      case  0x155: Serial.print(" - Treble Decrement"); break;
      case  0x160: Serial.print(" - Speaker System"); break;
      case  0x161: Serial.print(" - Channel Left"); break;
      case  0x162: Serial.print(" - Channel Right"); break;
      case  0x163: Serial.print(" - Channel Center"); break;
      case  0x164: Serial.print(" - Channel Front"); break;
      case  0x165: Serial.print(" - Channel Center Front"); break;
      case  0x166: Serial.print(" - Channel Side"); break;
      case  0x167: Serial.print(" - Channel Surround"); break;
      case  0x168: Serial.print(" - Channel Low Frequency Enhancement"); break;
      case  0x169: Serial.print(" - Channel Top"); break;
      case  0x16A: Serial.print(" - Channel Unknown"); break;
      case  0x170: Serial.print(" - Sub-channel"); break;
      case  0x171: Serial.print(" - Sub-channel Increment"); break;
      case  0x172: Serial.print(" - Sub-channel Decrement"); break;
      case  0x173: Serial.print(" - Alternate Audio Increment"); break;
      case  0x174: Serial.print(" - Alternate Audio Decrement"); break;
      case  0x180: Serial.print(" - Application Launch Buttons"); break;
      case  0x181: Serial.print(" - AL Launch Button Configuration Tool"); break;
      case  0x182: Serial.print(" - AL Programmable Button Configuration"); break;
      case  0x183: Serial.print(" - AL Consumer Control Configuration"); break;
      case  0x184: Serial.print(" - AL Word Processor"); break;
      case  0x185: Serial.print(" - AL Text Editor"); break;
      case  0x186: Serial.print(" - AL Spreadsheet"); break;
      case  0x187: Serial.print(" - AL Graphics Editor"); break;
      case  0x188: Serial.print(" - AL Presentation App"); break;
      case  0x189: Serial.print(" - AL Database App"); break;
      case  0x18A: Serial.print(" - AL Email Reader"); break;
      case  0x18B: Serial.print(" - AL Newsreader"); break;
      case  0x18C: Serial.print(" - AL Voicemail"); break;
      case  0x18D: Serial.print(" - AL Contacts/Address Book"); break;
      case  0x18E: Serial.print(" - AL Calendar/Schedule"); break;
      case  0x18F: Serial.print(" - AL Task/Project Manager"); break;
      case  0x190: Serial.print(" - AL Log/Journal/Timecard"); break;
      case  0x191: Serial.print(" - AL Checkbook/Finance"); break;
      case  0x192: Serial.print(" - AL Calculator"); break;
      case  0x193: Serial.print(" - AL A/V Capture/Playback"); break;
      case  0x194: Serial.print(" - AL Local Machine Browser"); break;
      case  0x195: Serial.print(" - AL LAN/WAN Browser"); break;
      case  0x196: Serial.print(" - AL Internet Browser"); break;
      case  0x197: Serial.print(" - AL Remote Networking/ISP Connect"); break;
      case  0x198: Serial.print(" - AL Network Conference"); break;
      case  0x199: Serial.print(" - AL Network Chat"); break;
      case  0x19A: Serial.print(" - AL Telephony/Dialer"); break;
      case  0x19B: Serial.print(" - AL Logon"); break;
      case  0x19C: Serial.print(" - AL Logoff"); break;
      case  0x19D: Serial.print(" - AL Logon/Logoff"); break;
      case  0x19E: Serial.print(" - AL Terminal Lock/Screensaver"); break;
      case  0x19F: Serial.print(" - AL Control Panel"); break;
      case  0x1A0: Serial.print(" - AL Command Line Processor/Run"); break;
      case  0x1A1: Serial.print(" - AL Process/Task Manager"); break;
      case  0x1A2: Serial.print(" - AL Select Tast/Application"); break;
      case  0x1A3: Serial.print(" - AL Next Task/Application"); break;
      case  0x1A4: Serial.print(" - AL Previous Task/Application"); break;
      case  0x1A5: Serial.print(" - AL Preemptive Halt Task/Application"); break;
      case  0x200: Serial.print(" - Generic GUI Application Controls"); break;
      case  0x201: Serial.print(" - AC New"); break;
      case  0x202: Serial.print(" - AC Open"); break;
      case  0x203: Serial.print(" - AC Close"); break;
      case  0x204: Serial.print(" - AC Exit"); break;
      case  0x205: Serial.print(" - AC Maximize"); break;
      case  0x206: Serial.print(" - AC Minimize"); break;
      case  0x207: Serial.print(" - AC Save"); break;
      case  0x208: Serial.print(" - AC Print"); break;
      case  0x209: Serial.print(" - AC Properties"); break;
      case  0x21A: Serial.print(" - AC Undo"); break;
      case  0x21B: Serial.print(" - AC Copy"); break;
      case  0x21C: Serial.print(" - AC Cut"); break;
      case  0x21D: Serial.print(" - AC Paste"); break;
      case  0x21E: Serial.print(" - AC Select All"); break;
      case  0x21F: Serial.print(" - AC Find"); break;
      case  0x220: Serial.print(" - AC Find and Replace"); break;
      case  0x221: Serial.print(" - AC Search"); break;
      case  0x222: Serial.print(" - AC Go To"); break;
      case  0x223: Serial.print(" - AC Home"); break;
      case  0x224: Serial.print(" - AC Back"); break;
      case  0x225: Serial.print(" - AC Forward"); break;
      case  0x226: Serial.print(" - AC Stop"); break;
      case  0x227: Serial.print(" - AC Refresh"); break;
      case  0x228: Serial.print(" - AC Previous Link"); break;
      case  0x229: Serial.print(" - AC Next Link"); break;
      case  0x22A: Serial.print(" - AC Bookmarks"); break;
      case  0x22B: Serial.print(" - AC History"); break;
      case  0x22C: Serial.print(" - AC Subscriptions"); break;
      case  0x22D: Serial.print(" - AC Zoom In"); break;
      case  0x22E: Serial.print(" - AC Zoom Out"); break;
      case  0x22F: Serial.print(" - AC Zoom"); break;
      case  0x230: Serial.print(" - AC Full Screen View"); break;
      case  0x231: Serial.print(" - AC Normal View"); break;
      case  0x232: Serial.print(" - AC View Toggle"); break;
      case  0x233: Serial.print(" - AC Scroll Up"); break;
      case  0x234: Serial.print(" - AC Scroll Down"); break;
      case  0x235: Serial.print(" - AC Scroll"); break;
      case  0x236: Serial.print(" - AC Pan Left"); break;
      case  0x237: Serial.print(" - AC Pan Right"); break;
      case  0x238: Serial.print(" - AC Pan"); break;
      case  0x239: Serial.print(" - AC New Window"); break;
      case  0x23A: Serial.print(" - AC Tile Horizontally"); break;
      case  0x23B: Serial.print(" - AC Tile Vertically"); break;
      case  0x23C: Serial.print(" - AC Format"); break;

    }
  }
  Serial.println();
}

void OnHIDExtrasRelease(uint32_t top, uint16_t key)
{
  Serial.print("HID (");
  Serial.print(top, HEX);
  Serial.print(") key release:");
  Serial.println(key, HEX);
}

bool OnReceiveHidData(uint32_t usage, const uint8_t *data, uint32_t len) {
  // Called for maybe both HIDS for rawhid basic test.  One is for the Teensy
  // to output to Serial. while still having Raw Hid...
  if (usage == 0xffc90004) {
    // Lets trim off trailing null characters.
    while ((len > 0) && (data[len - 1] == 0)) {
      len--;
    }
    if (len) {
      Serial.print("RawHid Serial: ");
      Serial.write(data, len);
    }
  } else {
    Serial.print("RawHID data: ");
    Serial.println(usage, HEX);
    while (len) {
      uint8_t cb = (len > 16) ? 16 : len;
      const uint8_t *p = data;
      uint8_t i;
      for (i = 0; i < cb; i++) {
        Serial.printf("%02x ", *p++);
      }
      Serial.print(": ");
      for (i = 0; i < cb; i++) {
        Serial.write(((*data >= ' ') && (*data <= '~')) ? *data : '.');
        data++;
      }
      len -= cb;
      Serial.println();
    }
  }

  return true;
}

void as_usbhost_setup()
{
  
  Serial.println("\n\nUSB Host Testing");
  Serial.println(sizeof(USBHub), DEC);
  myusb.begin();
  keyboard1.attachPress(OnPress);
  keyboard2.attachPress(OnPress);


  keyboard1.attachExtrasPress(OnHIDExtrasPress);
  keyboard1.attachExtrasRelease(OnHIDExtrasRelease);

  keyboard2.attachExtrasPress(OnHIDExtrasPress);
  keyboard2.attachExtrasRelease(OnHIDExtrasRelease);

  keyboard1.attachGriffinEvent(as_usbhost_jogdial);
  keyboard2.attachGriffinEvent(as_usbhost_jogdial);

  // The below forceBootProtocol will force which ever
  // next keyboard that attaches to this device to be in boot protocol
  // Only try this if you run into keyboard with issues.  If this is a combined
  // device like wireless mouse and keyboard this can cause mouse problems.
  //keyboard1.forceBootProtocol();
  rawhid1.attachReceive(OnReceiveHidData);
  rawhid2.attachReceive(OnReceiveHidData);



#ifdef AS_MIDI_FORWARD

	usbhost_midi1.setHandleNoteOff(as_usbhost_OnNoteOff);
	usbhost_midi1.setHandleNoteOn(as_usbhost_OnNoteOn);
	usbhost_midi1.setHandleControlChange(as_usbhost_OnControlChange);
  usbhost_midi1.setHandlePitchChange(as_usbhost_OnPitchBendChange);
  usbhost_midi1.setHandleProgramChange(as_usbhost_OnProgramChange);

  usbhost_midi1.setHandleSystemExclusive(as_usbhost_OnSysExMessage);
  usbhost_midi1.setHandleSysEx(as_usbhost_OnSysExMessage_1);
  
	usbhost_midi2.setHandleNoteOff(as_usbhost_OnNoteOff);
	usbhost_midi2.setHandleNoteOn(as_usbhost_OnNoteOn);
	usbhost_midi2.setHandleControlChange(as_usbhost_OnControlChange);
  usbhost_midi2.setHandlePitchChange(as_usbhost_OnPitchBendChange);
  usbhost_midi2.setHandleProgramChange(as_usbhost_OnProgramChange);



  
/*
  PitchBendCallback = void (*)(Channel channel, int)
ProgramChangeCallback = void (*)(Channel channel, byte)
*/


#else
	usbhost_midi1.setHandleNoteOff(OnNoteOff);
	usbhost_midi1.setHandleNoteOn(OnNoteOn);
	usbhost_midi1.setHandleControlChange(OnControlChange);
#endif

}

void printToHtml(EthernetClient * client){
  client->println("<h4>USB Devices</h4>");
  client->println("<ul>");
  int count=0;
  for (uint8_t i = 0; i < CNT_DEVICES; i++) {

      if (driver_active[i]) {
        char buf[128];
        uint8_t *psz = drivers[i]->product();
        *buf=0;
        sprintf(buf,"<li>%s %s (%x:%x)<br>", driver_names[i], ((psz) ? psz: "unknown" ), drivers[i]->idVendor(), drivers[i]->idProduct());
        client->print(buf);

        client->print("</li>");
        count++;
      }
  
  }
  if(count < 1) {
    client->print("<li>not found</li>");
  }
  client->println("</ul>");
}



void as_usbhost_beat()
{
  myusb.Task();
  usbhost_midi1.read();
  usbhost_midi2.read();

  if (Serial.available()) {
    int ch = Serial.read(); // get the first char.
    while (Serial.read() != -1) ;
    if ((ch == 'b') || (ch == 'B')) {
      Serial.println("Only notify on Basic Axis changes");
      joystick1.axisChangeNotifyMask(0x3ff);
    } else if ((ch == 'f') || (ch == 'F')) {
      Serial.println("Only notify on Full Axis changes");
      joystick1.axisChangeNotifyMask(joystick_full_notify_mask);

    } else {
      if (show_changed_only) {
        show_changed_only = false;
        Serial.println("\n*** Show All fields mode ***");
      } else {
        show_changed_only = true;
        Serial.println("\n*** Show only changed fields mode ***");
      }
    }
  }

  for (uint8_t i = 0; i < CNT_DEVICES; i++) {
    if (*drivers[i] != driver_active[i]) {
      if (driver_active[i]) {
        Serial.printf("*** Device %s - disconnected ***\n", driver_names[i]);
        driver_active[i] = false;
      } else {
        Serial.printf("*** Device %s %x:%x - connected ***\n", driver_names[i], drivers[i]->idVendor(), drivers[i]->idProduct());
        driver_active[i] = true;

        const uint8_t *psz = drivers[i]->manufacturer();
        if (psz && *psz) Serial.printf("  manufacturer: %s\n", psz);
        psz = drivers[i]->product();
        if (psz && *psz) Serial.printf("  product: %s\n", psz);
        psz = drivers[i]->serialNumber();
        if (psz && *psz) Serial.printf("  Serial: %s\n", psz);
      }
    }
  }

  for (uint8_t i = 0; i < CNT_HIDDEVICES; i++) {
    if (*hiddrivers[i] != hid_driver_active[i]) {
      if (hid_driver_active[i]) {
        Serial.printf("*** HID Device %s - disconnected ***\n", hid_driver_names[i]);
        hid_driver_active[i] = false;
      } else {
        Serial.printf("*** HID Device %s %x:%x - connected ***\n", hid_driver_names[i], hiddrivers[i]->idVendor(), hiddrivers[i]->idProduct());
        hid_driver_active[i] = true;

        const uint8_t *psz = hiddrivers[i]->manufacturer();
        if (psz && *psz) Serial.printf("  manufacturer: %s\n", psz);
        psz = hiddrivers[i]->product();
        if (psz && *psz) Serial.printf("  product: %s\n", psz);
        psz = hiddrivers[i]->serialNumber();
        if (psz && *psz) Serial.printf("  Serial: %s\n", psz);
        // Note: with some keyboards there is an issue that they don't output in boot protocol mode
        // and may not work.  The above code can try to force the keyboard into boot mode, but there
        // are issues with doing this blindly with combo devices like wireless keyboard/mouse, which
        // may cause the mouse to not work.  Note: the above id is in the builtin list of
        // vendor IDs that are already forced
        #if 0  // In list in older, newer code should support the N Key rollover directly
        if (hiddrivers[i] == &keyboard1) {
          if (keyboard1.idVendor() == 0x04D9) {
            Serial.println("Gigabyte vendor: force boot protocol");
            // Gigabyte keyboard
            keyboard1.forceBootProtocol();
          }
        }
        #endif
      }
    }
  }



  if (mouse1.available()) {

    #ifdef AS_MOUSE_FORWARD
      as_usbhost_mouse(mouse1.getButtons(), mouse1.getMouseX(),mouse1.getMouseY(),mouse1.getWheel(),mouse1.getWheelH());
    #else
      Serial.print("Mouse: buttons = ");
      Serial.print(mouse1.getButtons());
      Serial.print(",  mouseX = ");
      Serial.print(mouse1.getMouseX());
      Serial.print(",  mouseY = ");
      Serial.print(mouse1.getMouseY());
      Serial.print(",  wheel = ");
      Serial.print(mouse1.getWheel());
      Serial.print(",  wheelH = ");
      Serial.print(mouse1.getWheelH());
      Serial.println();
    #endif  
    mouse1.mouseDataClear();
  }
  if (joystick1.available()) {
    uint64_t axis_mask = joystick1.axisMask();
    uint64_t axis_changed_mask = joystick1.axisChangedMask();
    Serial.print("Joystick: buttons = ");
    uint32_t buttons = joystick1.getButtons();
    Serial.print(buttons, HEX);
    //Serial.printf(" AMasks: %x %x:%x", axis_mask, (uint32_t)(user_axis_mask >> 32), (uint32_t)(user_axis_mask & 0xffffffff));
    //Serial.printf(" M: %lx %lx", axis_mask, joystick1.axisChangedMask());
    if (show_changed_only) {
      for (uint8_t i = 0; axis_changed_mask != 0; i++, axis_changed_mask >>= 1) {
        if (axis_changed_mask & 1) {
          Serial.printf(" %d:%d", i, joystick1.getAxis(i));
        }
      }

    } else {
      for (uint8_t i = 0; axis_mask != 0; i++, axis_mask >>= 1) {
        if (axis_mask & 1) {
          Serial.printf(" %d:%d", i, joystick1.getAxis(i));
        }
      }
    }
    uint8_t ltv;
    uint8_t rtv;
    switch (joystick1.joystickType()) {
      default:
        break;
      case JoystickController::PS4:
        ltv = joystick1.getAxis(3);
        rtv = joystick1.getAxis(4);
        if ((ltv != joystick_left_trigger_value) || (rtv != joystick_right_trigger_value)) {
          joystick_left_trigger_value = ltv;
          joystick_right_trigger_value = rtv;
          joystick1.setRumble(ltv, rtv);
        }
        break;

      case JoystickController::PS3:
        ltv = joystick1.getAxis(18);
        rtv = joystick1.getAxis(19);
        if ((ltv != joystick_left_trigger_value) || (rtv != joystick_right_trigger_value)) {
          joystick_left_trigger_value = ltv;
          joystick_right_trigger_value = rtv;
          joystick1.setRumble(ltv, rtv, 50);
        }
        break;

      case JoystickController::XBOXONE:
      case JoystickController::XBOX360:
        ltv = joystick1.getAxis(4);
        rtv = joystick1.getAxis(5);
        if ((ltv != joystick_left_trigger_value) || (rtv != joystick_right_trigger_value)) {
          joystick_left_trigger_value = ltv;
          joystick_right_trigger_value = rtv;
          joystick1.setRumble(ltv, rtv);
          Serial.printf(" Set Rumble %d %d", ltv, rtv);
        }
        break;
    }
    if (buttons != buttons_prev) {
      if (joystick1.joystickType() == JoystickController::PS3) {
        joystick1.setLEDs((buttons >> 12) & 0xf); //  try to get to TRI/CIR/X/SQuare
      } else {
        uint8_t lr = (buttons & 1) ? 0xff : 0;
        uint8_t lg = (buttons & 2) ? 0xff : 0;
        uint8_t lb = (buttons & 4) ? 0xff : 0;
        joystick1.setLEDs(lr, lg, lb);
      }
      buttons_prev = buttons;
    }

    Serial.println();
    joystick1.joystickDataClear();
  }

  // See if we have some RAW data
  if (rawhid1) {
    int ch;
    uint8_t buffer[64];
    uint8_t count_chars = 0;
    memset(buffer, 0, sizeof(buffer));
    if (Serial.available()) {
      while (((ch = Serial.read()) != -1) && (count_chars < sizeof(buffer))) {
        buffer[count_chars++] = ch;
      }
      rawhid1.sendPacket(buffer);
    }
  }
}

