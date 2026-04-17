
#include <Arduino.h>
#include "as_transputerlink.h"
#include "mb_linkio.h"
#include <LibPrintf.h>

asTransputerLinkBase::asTransputerLinkBase(int pinReset,int pinAnalyse,int pinError){
      m_readCount  = 0;
      m_writeCount = 0;   
      m_openStamp  = 0;
      m_readCountSum=0;
      m_writeCountSum=0;        
      m_openFlag = false;
      
      pin_reset = pinReset;
      pin_analyse = pinAnalyse;      
      pin_error = pinError;  

    if(pin_error>=0){
      pinMode(pin_error,   INPUT_PULLUP);
    }
    if(pin_reset>=0){
      setReset_Pin(false);
      pinMode(pin_reset, OUTPUT);  
    }
    
    
    if(pin_analyse>=0){
      setAnalyse_Pin(false);
      pinMode(pin_analyse,   OUTPUT);  
    }      

}
asTransputerLinkBase::~asTransputerLinkBase(){}

void asTransputerLinkBase::setReset_Pin(bool ein){
  if(pin_reset>=0){
    digitalWrite(pin_reset,(ein) ? HIGH : LOW);
  #ifdef DEBUG_ON    
    printf("setReset_Pin %d \r\n",ein);
  #endif  
  printf("setReset_Pin %d \r\n",ein);
  }

}

void asTransputerLinkBase::setAnalyse_Pin(bool ein){
  if(pin_analyse>=0){
    digitalWrite(pin_analyse,(ein) ? HIGH  : LOW);
  #ifdef DEBUG_ON    
    printf("setAnalyse_Pin %d \r\n",ein);
  #endif  
  }

}
bool asTransputerLinkBase::getError_Pin(){
  if(pin_error >= 0){
    return (digitalRead(pin_error) ==HIGH) ? true : false; 
  }
  return false;
}    

void asTransputerLinkBase::ResetCounts() {
    m_readCount  = 0;
    m_writeCount = 0; 
    m_openStamp  = 0; 
}
int asTransputerLinkBase::OpenLink(    char *Name){
  printf("asTransputerLinkBase::OpenLink\r\n");
  *Name=0;
  sprintf(Name,"TeensyLinkDummy");
  return 1;   
}
int asTransputerLinkBase::CloseLink(    ){
  printf("asTransputerLinkBase::CloseLink\r\n");
  return 1;   
}
int asTransputerLinkBase::ReadLink(    char *Buffer, unsigned int Count, int Timeout ){

  printf("asTransputerLinkBase::ReadLink count %d\r\n",Count);
  return Count;  
}
int asTransputerLinkBase::WriteLink(   char *Buffer, unsigned int Count, int Timeout ){
  printf("asTransputerLinkBase::WriteLink count %d %d\r\n",Count,*Buffer);
  return Count;
}
int asTransputerLinkBase::ResetLink(    ){
#ifdef DEBUG_ON    
  printf("asTpLinkC011::ResetLink 300ms\r\n");
#endif  

  if(pin_reset>=0){
    setReset_Pin(false);
    delay(300);
    setReset_Pin(true);
    delay(300);
    setReset_Pin(false);
    delay(200);
    #ifdef DEBUG_ON    
      printf("asTpLinkC011::ResetLink durch\r\n");
    #endif  
  }  
  adapterReset();
 
  return 1;
}
int asTransputerLinkBase::AnalyseLink(  ){
#ifdef DEBUG_ON    
  printf("asTpLinkC011::AnalyseLink\r\n");
#endif  
  if ( !m_openFlag )    return ER_LINK_BAD;
  return 1;
}
int asTransputerLinkBase::TestError(    ){
  //Todo wenn PCB fertig
  //printf("asTpLinkC011::TestError\r\n");
  if ( !m_openFlag )    return ER_LINK_BAD;
    
  return 0;  
}

int asTransputerLinkBase::TestRead(     ){
  return 0;  
}
int asTransputerLinkBase::TestWrite(    ){
  return 0;  
}
