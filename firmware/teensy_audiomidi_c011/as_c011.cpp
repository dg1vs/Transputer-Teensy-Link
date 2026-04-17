
#include <Arduino.h>
#include <LibPrintf.h> 
#include "as_c011.h"
#include "mb_linkio.h"

#define FAST_LINKBYTE_READ 1

#define OPENCLOSE_LOG_NO 1
// #ifdef TOT_OLD_CODE
// #define C011_IACK     2
// #define C011_IVALID   3
// #define C011_OACK     4
// #define C011_OVALID   5

// #define C011_ERROR    32
// #define C011_ANALYSE  31
// #define C011_RESET    33

// const int IN_PINS[8]     = { 1,  0, 24, 25, 14, 15,  6,  9};
// const int OUT_PINS[8]    = {17, 16, 22, 23, 20, 21, 26, 27};

// #define C011_INMASK_A 0b00000000000000000000000000001100
// #define C011_INMASK_B 0b00000000000000000011000000000000
// #define C011_INMASK_C 0b00000000000011000000000000000000
// #define C011_INMASK_D 0b00000000000000000000110000000000

// #define C011_OUTMASK 0b11001111110000000000000000000000
// #endif

/* Teensy 4.1 */


const int C011_IN_PINS[8]     =  { 1,  0, 24 , 25, 41,17 ,16  ,22  }; /* Daten zum Transputer*/
const int C011_OUT_PINS[8]    =  { 2, 3, 4, 33, 5, 38, 39, 26}; /* Daten vom Transputer*/



/* Daten teensy -> C011  
Bzeiechnung vom C011 aus gesehen
*/
#define C011_IN_0_MASK   0b00000000000000000000000000000100
#define C011_IN_1_MASK   0b00000000000000000000000000001000
#define C011_IN_2_MASK   0b00000000000000000001000000000000
#define C011_IN_3_MASK   0b00000000000000000010000000000000
#define C011_IN_4_MASK   0b00000000001000000000000000000000
#define C011_IN_5_MASK   0b00000000010000000000000000000000
#define C011_IN_6_MASK   0b00000000100000000000000000000000
#define C011_IN_7_MASK   0b00000001000000000000000000000000



#define C011_IN_01_MASK (C011_IN_0_MASK | C011_IN_1_MASK)
#define C011_IN_23_MASK (C011_IN_2_MASK | C011_IN_3_MASK)
#define C011_IN_47_MASK (C011_IN_4_MASK | C011_IN_5_MASK | C011_IN_6_MASK | C011_IN_7_MASK)
#define C011_IN_MASK (C011_IN_01_MASK | C011_IN_23_MASK | C011_IN_47_MASK)

/* Daten teensy <- C011  
Bzeiechnung vom C011 aus gesehen
*/
#define C011_OUT_0_MASK 0b00000000000000000000000000010000
#define C011_OUT_1_MASK 0b00000000000000000000000000100000
#define C011_OUT_2_MASK 0b00000000000000000000000001000000
#define C011_OUT_3_MASK 0b00000000000000000000000010000000
#define C011_OUT_4_MASK 0b00000000000000000000000100000000
#define C011_OUT_5_MASK 0b00010000000000000000000000000000
#define C011_OUT_6_MASK 0b00100000000000000000000000000000
#define C011_OUT_7_MASK 0b01000000000000000000000000000000

#define C011_OUT04_MASK (C011_OUT_0_MASK | C011_OUT_1_MASK| C011_OUT_2_MASK | C011_OUT_3_MASK | C011_OUT_4_MASK)
#define C011_OUT57_MASK (C011_OUT_5_MASK | C011_OUT_6_MASK| C011_OUT_7_MASK)


#define IACK_MASK   0b00000000000100000000000000000000 
#define OVALID_MASK 0b10000000000000000000000000000000 


asTpLinkC011::asTpLinkC011(int pinReset,int pinAnalyse,int pinError) : asTransputerLinkBase(pinReset, pinAnalyse, pinError){

    for (int i = 0; i < 8; i++) {
      pinMode(C011_IN_PINS[i], OUTPUT);
      pinMode(C011_OUT_PINS[i], INPUT_PULLUP);
    }
    pinMode(C011_IACK,   INPUT_PULLUP);
    pinMode(C011_IVALID, OUTPUT);
    
    pinMode(C011_OVALID, INPUT_PULLUP);
    pinMode(C011_OACK,   OUTPUT);
    

    digitalWriteFast(C011_IVALID,LOW);
    digitalWriteFast(C011_OACK,LOW);

    /*ResetLink();*/

}
asTpLinkC011::~asTpLinkC011(){}

FASTRUN  uint8_t asTpLinkC011::read_C011(){

  uint8_t r=0;
  
#ifdef FAST_LINKBYTE_READ  

  r =  (GPIO9_DR & C011_OUT04_MASK) >> 4;
  r |= (GPIO6_DR & C011_OUT57_MASK) >> 23;
#else
  uint8_t i=0;
  uint8_t mask=0x1;
  for(i=0;i < 8;i++){
    if(digitalRead(C011_OUT_PINS[i])) {
       r |= mask;
    }
    mask <<=1;
  }
#endif

  return r;
}

FASTRUN  void asTpLinkC011::write_C011(uint8_t inV){


   uint32_t w;
   uint32_t v = inV;

   w  = (inV << 17) & C011_IN_47_MASK;
   w |= (inV << 10) & C011_IN_23_MASK;
   w |= (inV << 2) & C011_IN_01_MASK;   
  
   GPIO6_DR = (GPIO6_DR & ~C011_IN_MASK ) | (w & C011_IN_MASK);
}


int asTpLinkC011::OpenLink(    char *Name){

#ifdef OPENCLOSE_LOG   
  printf("asTpLinkC011::OpenLink\r\n");
#endif
  // *Name=0;
  // sprintf(lName,"TeensyLinkDummy");

  //ResetLink(    );
  ResetCounts();
  m_openFlag = true;
  m_openStamp = micros();
  return 1;   
}
int asTpLinkC011::CloseLink(    ){
  if(isOpen()) {
#ifdef OPENCLOSE_LOG       
    unsigned long long  closeStamp = micros();
    printf("asTpLinkC011::CloseLink Write %d Read: %d  Bytes RunTime %llu us\r\n",writeCount(),readCount() ,(closeStamp - m_openStamp));
#endif    
  }
  m_openFlag = false;
  return 1;   
}


/* FASTRUN */ int asTpLinkC011::ReadLink(    char *Buffer, unsigned int Count, int Timeout ){
    
  unsigned int inCount=0;
  
  bool timeOutFlag=false;
#ifdef DEBUG_ON   
  printf("asTpLinkC011::ReadLink count %d Tout %d ",Count,Timeout);
#endif
 //Timeout = 100;
  Timeout = (Timeout > 0) ? Timeout * 100 : 10000*100;

  //irgendwas ist schief gegangen
  //if(digitalRead(C011_OACK)) return 0;

  const unsigned long in_time= millis()+Timeout;

  if ( !m_openFlag )    return ER_LINK_BAD;
  if ( Count < 1 )      return ER_LINK_CANT;
  if ( Count > 0x7FFF ) return ER_LINK_CANT;
  

    //falls ein vorheriger Transfer noch nicht abgeschlossen ist

  while(digitalReadFast(C011_OACK)){
     if(millis() > in_time){
        timeOutFlag = true;
        #ifdef DEBUG_ON
               printf("raus C\r\n");
       #endif
               
        break;
     }
  }
 #ifdef DEBUG_ON
printf(" --->  [");  
#endif

  while((inCount < Count) && !timeOutFlag){

    //if(getOVALID_C011()/* && !getOACK_C011()*/) {
    if(digitalRead(C011_OVALID)){

       Buffer[inCount] = read_C011();
#ifdef DEBUG_ON    
       if(inCount < 9) {
         printf(" %02x",Buffer[inCount]);
       }
#endif
      // digitalWriteFast(C011_OACK,HIGH);
       
      //  while(digitalReadFast(C011_OACK)){

      //     if(!digitalReadFast(C011_OVALID)){  

      //       digitalWriteFast(C011_OACK,LOW);
      //       //digitalWrite(C011_OACK,LOW);
      //       inCount++;
      //     }else{
      //       //printf("#%ld #r\r\n",(in_time-millis()));  
      //       if(millis() > in_time){
      //          timeOutFlag = true;
      //          //inCount = -1;

      //          #ifdef DEBUG_ON
      //          printf("raus A\r\n");
      //          #endif
      //          break;
      //       }
            
      //     }
      //  } 
#ifdef FAST_LINKBYTE_READ      
       digitalWriteFast(C011_OACK,HIGH);

       while(1){


          if(!digitalReadFast(C011_OVALID)){  
            digitalWriteFast(C011_OACK,LOW);
#else   
       digitalWrite(C011_OACK,HIGH);
       while(1){         
          if(!digitalRead(C011_OVALID)){  
            digitalWrite(C011_OACK,LOW);  
#endif            
            inCount++;
            break;
          }else{

            if(millis() > in_time){
               timeOutFlag = true;

               #ifdef DEBUG_ON
               printf("raus A\r\n");
               #endif
               break;
            }
            
          }
       }         
    }else{
        if(millis() > in_time){
               timeOutFlag = true;
               //inCount = -1;

               #ifdef DEBUG_ON
               printf("raus B\r\n");
               #endif
               break;
            }
    }
  }
#ifdef DEBUG_ON
  if(inCount > 8) {
     printf("...");
  }   
  printf("] inCount:%d von %d \r\n\r\n",inCount,Count);
#endif  


  if(inCount>0)  {
    m_readCount += inCount;  
    m_readCountSum += inCount;  
  }  
  return inCount;  
}



/* FASTRUN */ int asTpLinkC011::WriteLink(   char *Buffer, unsigned int Count, int Timeout ){
 
#ifdef DEBUG_ON   
  printf("asTpLinkC011::WriteLink count %d Tout: %d [",Count,Timeout);
#endif  
  //unsigned long out_time= millis();
  //keinen unendliche Timeouts, maximal 1000sek
  Timeout = (Timeout > 0) ? Timeout * 100 : 10000*100;
  unsigned int outCount=0;
  bool timeOutFlag=false;

  const unsigned long out_time= millis()+Timeout;

  
  if ( !m_openFlag )    return ER_LINK_BAD;
  if ( Count < 1 )      return ER_LINK_CANT;
  if ( Count > 0x7FFF ) return ER_LINK_CANT;
    

  //falls ein vorheriger Transfer noch nicht abgeschlossen ist
  while(digitalRead(C011_IVALID)){
     if(millis() > out_time){
        timeOutFlag = true;
        break;
     }
  }
  /*printf("A ivalid %d\r\n",digitalRead(C011_IVALID));*/
  while((outCount < Count) && !timeOutFlag ){

       //delayNanoseconds(250);
       write_C011(Buffer[outCount]);
#ifdef DEBUG_ON   
       if(outCount < 16) {
         printf(" %02x",Buffer[outCount]);
       }
#endif
/*printf("B\r\n");*/
       digitalWriteFast(C011_IVALID,HIGH);
       /*digitalWrite(C011_IVALID,HIGH);*/
/*
 * geht nicht, warum ???
       while(true) {

          if(digitalRead(C011_IACK)){
            digitalWriteFast(C011_IVALID,LOW);
            outCount++;
            break;
            
          }else{
          
            if(millis() > out_time){
               timeOutFlag = true;
               break;
            }
          }
          
       }
*/

      while(digitalRead(C011_IVALID)) {
        
          if(digitalReadFast(C011_IACK)){
          /*if(digitalRead(C011_IACK)){*/
            /* printf("C011_IACK ok\r\n");*/
            digitalWriteFast(C011_IVALID,LOW);
            /*digitalWrite(C011_IVALID,LOW);*/
            outCount++;


          }else{
            /* printf("kein lesen von C011_IACK\r\n");*/
            if(millis() > out_time){
               timeOutFlag = true;
               //outCount = -1;
               break;
            }
          
          }
          
       }

  }

#ifdef DEBUG_ON 
  if(outCount>8) {
    printf("...");
  }
  printf("] -> outCount:%d von %d\r\n\r\n",outCount,Count);
#endif  
  if(outCount > 0)  {
    m_writeCount +=outCount; 
    m_writeCountSum +=outCount;
  }  
  // delayNanoseconds(50000);
  return outCount;
}



/*
gegen hängenden Adapter, sehr tricky
*/
void asTpLinkC011::adapterReset(){
   if(digitalRead(C011_OACK)) {
    digitalWrite(C011_OACK,LOW);
   }
   if(digitalRead(C011_IVALID)){
    digitalWrite(C011_IVALID,LOW);
   }
}


int asTpLinkC011::TestRead(     ){
  if ( !m_openFlag )    return ER_LINK_BAD;
  if(digitalRead(C011_OVALID)) return 1;
  return 0;  
}
int asTpLinkC011::TestWrite(    ){
  if ( !m_openFlag )    return ER_LINK_BAD;
  return 1;  
}

bool asTpLinkC011::hasInByte(){
   return digitalRead(C011_OVALID);
}

