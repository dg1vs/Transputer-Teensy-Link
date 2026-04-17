
#include <Arduino.h>
#include <LibPrintf.h> 
#include "as_teensylink.h"
#include "mb_linkio.h"

#define FAST_LINKBYTE_READ 1

#define OPENCLOSE_LOG 1

#define AS_FASTRUN FASTRUN


/* -------------- von ktransputer_iserver xlink*/

// Check if user enabled 9-bit UART in Teensy Core Library.
#ifndef SERIAL_9N1_RXINV_TXINV
#error "----------------------------------------"
#error "You need to enable 9-bit UART in Teensy Core Library:"
#error "1) Goto c:\Users\USER\AppData\Local\Arduino15\packages\teensy\hardware\avr\1.59.0\cores\teensy4>"
#error "2) Edit HardwareSerial.h"
#error "3) Uncomment #define SERIAL_9BIT_SUPPORT"
#error "----------------------------------------"
#endif

/*   von xlink.h übernommen */
void link0_init(HardwareSerialIMXRT * p,int speed) {
  switch(speed)
  {
    case LINK0_SPEED_5MBPS:
                              p->begin(5000000, SERIAL_9N1_RXINV_TXINV );     // Link0 connected to SERIAL5, max 6M OK
                              p->flush();
                              p->clear();  

                              Serial.println("Emulation  =  5.0 Mb/s");
                              break;
    case LINK0_SPEED_10MBPS:
    case 0b00: /* 00 = 10Mbps*/
                              Serial.println("ERROR: 10Mbps not supported. Please use 20Mbps.\nHalting.");
                              while(1);
                              
                              Serial.println("Emulation  = 10.0 Mb/s");
                              break;
    case LINK0_SPEED_20MBPS:
                              p->begin(20000000, SERIAL_9N1_RXINV_TXINV );
                              CCM_CSCDR1=105450240;
                              /*LPUART7_WATER = (0 << 24) | (0 << 16) | (1 << 8) | 1; 
                              NVIC_SET_PRIORITY(IRQ_LPUART7, 64); 
                              */
                              // ^^^ //Adjust UART clock to handle 20Mbps.
                              // https://forum.pjrc.com/index.php?threads/teensy4-1-max-baud-rate.67150/
                              p->flush();
                              p->clear();  

                              Serial.println("Emulation  = 20.0 Mb/s");
                              break;
    default:
                              Serial.println("ERR: unknown transputer link speed.");
                              Serial.println("     halting...");
                              while(1);
  }
}


inline __attribute__((always_inline))
long asTpLinkTeensy::link0_rx_buffer_count()
{
  long cnt = 0;
  
  if (received_data_head >= received_data_tail)
    cnt = (received_data_head - received_data_tail);
  else
    cnt = (received_data_head) + (LINK0_RECEIVE_BUFFER_SIZE - received_data_tail);   // head points to empty space.

  return cnt;
}

inline __attribute__((always_inline))
int asTpLinkTeensy::link0_get_data_nowait()
{
  byte b;

  if (received_data_head == received_data_tail)           // empty queue
  {
    // if (1 | outputDEBUG) Serial.println("ERR: get_data() fail due to no data received.");
    b = -1;
  }
  else
  {
    b = link0_rx_buffer[received_data_tail];
    received_data_tail = (received_data_tail + 1) % LINK0_RECEIVE_BUFFER_SIZE;
    // if (1 | outputDEBUG) {Serial.print("DATA Buffer: returning 0x"); Serial.println(b, HEX); }
    if (we_have_pending_ack)
    {
      // Buffer was full so we didn't ack the last rx data, now we can...
      link0_send_ack();               // ok b/c cpu is on hold.
      we_have_pending_ack = false;    // interrupts must still be disabled.
    }
  }
  
  return b;
}

inline __attribute__((always_inline))
int asTpLinkTeensy::link0_wait_get_data(unsigned int c)
{
  int b;
  while( (b = link0_get_data_nowait()) == -1)
  {
    c--;
    if (c == 0)
    {
      // if (1 | outputDEBUG) Serial.println("ERR: no data received.");
      return -1;      
    }
  } 
  return b;
}

inline __attribute__((always_inline))
byte asTpLinkTeensy::link0_tx_buffer_get_front()
{
  byte d;

  // Serial.print("link0_tx_buffer_get_front: 0x");

  if (tx_buffer_head == tx_buffer_tail)
  {
    // Option 1: return 0x00 or 0xFF
    // Option 2: return the tail again...
    // Serial.printf("--\n");
    d = 0x00;
    link0_tx_buffer_full = false;
  } 
  else
  {
    d = link0_tx_buffer[tx_buffer_tail];
    tx_buffer_tail = (tx_buffer_tail + 1) % LINK0_TRANSMIT_BUFFER_SIZE;
    link0_tx_buffer_full = false;
    // Serial.printf("%02x\n", d);
  }
  return d;
}

inline __attribute__((always_inline))
void asTpLinkTeensy::link0_send_data_noack(byte b)
{
  word c1 = ((byte) (~b)) << 1;
  // Serial.printf("write9bit %03x\n", c1);
  serialPort->write9bit(c1);
  link0_tx_buffer_waiting_for_ack = true;  
  // ^^^ Set flag - we won't send another until ACK is received. link0_rxtx()

  // delayNanoseconds(200*15);  //  TODO: do we need this?
}

inline __attribute__((always_inline))
bool asTpLinkTeensy::link0_wait_for_ack(unsigned int c)
{
  while( !link0_check_for_ack())
  {
    // if (1 | outputDEBUG) Serial.print(".");
    c--;
    if (c == 0)
    {
      // if (1 | outputDEBUG) Serial.println("NACK");
      return false;
    }
  }
  // if (1 | outputDEBUG) Serial.println("ACK");
  return true;
}

inline __attribute__((always_inline))
void asTpLinkTeensy::link0_send_data_wait_for_ack(byte b, unsigned int timeout)
{
  link0_send_data_noack(b);
  if (timeout != 0) link0_wait_for_ack(timeout);
  // link0_tx_buffer_print_info();  
}

inline __attribute__((always_inline))
void asTpLinkTeensy::link0_send_ack()
{
  // Serial.println("Sending ACK to transputer.");
  word c1 = 0x1FF;        // invert 8bit data, shift <<1, or 1 (data indicator bit)
  serialPort->write9bit(c1);
  // Serial.printf("send_ack %02x\n", c1);
}

inline __attribute__((always_inline))
bool asTpLinkTeensy::link0_tx_buffer_empty()
{
  if (tx_buffer_head == tx_buffer_tail)
    return true;
  else
  {
    // Serial.printf("TX_BUF EMPTY? No.\n");
    return false;
  }
}

inline __attribute__((always_inline))
bool asTpLinkTeensy::link0_check_for_ack()
{
  bool result = false;
 
  if (received_ack_count > 0)
  {
    received_ack_count--;
    // if (1 | outputDEBUG) Serial.println("ACK accepted.");
    link0_tx_buffer_waiting_for_ack = false;
    result = true;
  }
  else
  {
    // if (1 | outputDEBUG) Serial.println("MISSING ACK");
    result = false;
  }
  
  // if (result) {Serial.println("ACK accepted from queue.");}
  return result;
}

inline __attribute__((always_inline))
bool asTpLinkTeensy::link0_add2_tx_queue(byte data)
{
  bool result = false; 
  // Serial.print("link0_add2_tx_queue: ");

  if (link0_tx_buffer_full)
  {
    /*Serial.println("link0_add2_tx_queue: FULL");*/
    result = false;
  }
  else
  {
    // Check if buffer is full using modulus arithmetic
    unsigned int next_head = (tx_buffer_head + 1) % LINK0_TRANSMIT_BUFFER_SIZE;
    
    if (next_head == tx_buffer_tail)
    {
      /*Serial.printf("ERR: Link TX buffer full. Dropping byte: %2x\n", data);*/
      link0_tx_buffer_full = true;
      result = false;
    }
    else
    {
      link0_tx_buffer[tx_buffer_head] = data;
      tx_buffer_head = next_head;
      link0_tx_buffer_full = false;
      result = true;
      
      // Check if buffer is almost full (only 1 spot left)
      unsigned int next_next_head = (tx_buffer_head + 1) % LINK0_TRANSMIT_BUFFER_SIZE;
      if (next_next_head == tx_buffer_tail)
      {
        /*Serial.println("WAR: Link TX buffer almost full.");*/
      }
    }
  }

  // link0_tx_buffer_print_info();
  return result;
}

inline __attribute__((always_inline))
void asTpLinkTeensy::link0_process_incoming()
{
  bool send_ack_for_rx_data = false;      // can we send an ACK for received data?
                                          // if buffers are full, do not send ACK
                                          // use "we_have_pending_ack" bool to track it.

  // if (1 | outputDEBUG) {Serial.println("\nPress a key to Process Packet.\n"); while(!Serial.available()); Serial.read();}
  // delayMicroseconds(20);      // wait for signal to propagate through TXB0108...

  // SERIAL5 9Bit Mode
  if (!serialPort->available())
    return;                           // nothing received.

  word msg = ~(serialPort->read());
  byte msg_type = msg & 0x01;

  // Is it ACK?
  if (msg_type == 0)
  {
    // Serial.println("HW ACK received and put into queue.");
    received_ack_count++;
  }
  else
  {
    // DATA received
    byte data = (msg >> 1) & 0xFF;

    // if (1 | outputDEBUG) {Serial.printf("Serial5.available(): %d\n", Serial5.available());}
    // if (1 | outputDEBUG) {Serial.printf("SERIAL5: Received %02x\n", data);}
    
    // Save data in buffer using modulus arithmetic
    unsigned int next_head = (received_data_head + 1) % LINK0_RECEIVE_BUFFER_SIZE;
    
    if (next_head == received_data_tail)
    {
      //Serial.printf("ERR: Link RX buffer full. Dropping byte: %2x\n", data);
      send_ack_for_rx_data  = false;
      we_have_pending_ack   = true;
    }
    else
    {
      link0_rx_buffer[received_data_head] = data;
      received_data_head = next_head;
      
      // Check if buffer is almost full (only 1 spot left)
      unsigned int next_next_head = (received_data_head + 1) % LINK0_RECEIVE_BUFFER_SIZE;
      if (next_next_head == received_data_tail)
      {
        Serial.println("WAR: Link RX buffer almost full.");
        send_ack_for_rx_data  = false;
        we_have_pending_ack   = true;
      }
      else
      {
        send_ack_for_rx_data  = true;
        we_have_pending_ack   = false;
      }
    }

    if (send_ack_for_rx_data) 
    {
      // Serial.printf("INFO: Sending ACK for received data %2x\n", data);
      link0_send_ack();
    }
    // link0_rx_buffer_print_info();
  }

  return;
}


/* ----------------------------------------- */




asTpLinkTeensy::asTpLinkTeensy(HardwareSerialIMXRT * inPort, int pinReset,int pinAnalyse,int pinError) 
   : asTransputerLinkBase(pinReset, pinAnalyse, pinError){
    serialPort = inPort;

    Serial.print("asTpLinkTeensy: ");


 


    link0_init(serialPort,LINK0_SPEED_20MBPS);

    adapterReset();
     
}

asTpLinkTeensy::~asTpLinkTeensy(){}

char * asTpLinkTeensy::AdapterName() { return "asTpLinkTeensy"; }


AS_FASTRUN void asTpLinkTeensy::heartbeat(){
  // ##################################################
  // Handle incoming
  link0_process_incoming();

  // ##################################################
  // Handle outgoing if we are not waiting for an ACK
  if (link0_tx_buffer_waiting_for_ack)  {
    if (link0_check_for_ack())    {
      // Serial.print("Received ACK. "); Serial.println(received_ack_count);
    }
  } else if ( !link0_tx_buffer_empty() /* && !link0_tx_buffer_waiting_for_ack */ ) {
    
    byte d = link0_tx_buffer_get_front();
    /*printf("taking  one from tx buffer and sending to transputer. (%x) \r\n",d);*/
    /*delayNanoseconds(500); */
    link0_send_data_wait_for_ack(d,0);
    
    /*while(!serialPort->available()) {
      
    }
    */
    
    /*while(link0_tx_buffer_waiting_for_ack){
      link0_process_incoming();
      link0_check_for_ack();
    }
    */
  }
}

 

int asTpLinkTeensy::OpenLink(    char *Name){

#ifdef OPENCLOSE_LOG   
  printf("asTpLinkTeensy::OpenLink %s\r\n",Name);
#endif
  // *Name=0;
  // sprintf(lName,"TeensyLinkDummy");

  //ResetLink(    );
  ResetCounts();
  m_openFlag = true;
  m_openStamp = micros();

  /*adapterReset();*/


  return 1;   
}
int asTpLinkTeensy::CloseLink(    ){
  if(isOpen()) {
#ifdef OPENCLOSE_LOG       
    unsigned long long  closeStamp = micros();
    printf("asTpLinkTeensy::CloseLink Write %d Read: %d  Bytes RunTime %llu us\r\n",writeCount(),readCount() ,(closeStamp - m_openStamp));
#endif    
  }
  m_openFlag = false;
  return 1;   
}




AS_FASTRUN int asTpLinkTeensy::ReadLink(    char *Buffer, unsigned int Count, int Timeout ){
    
  unsigned int inCount=0;

#ifdef DEBUG_ON   
  printf("asTpLinkTeensy::ReadLink count %d Tout %d ",Count,Timeout);
#endif

  bool timeOutFlag=false;

 //Timeout = 100;
  Timeout = (Timeout > 0) ? Timeout * 100 : 10000*100;

  //irgendwas ist schief gegangen
  //if(digitalRead(C011_OACK)) return 0;

  const unsigned long in_time= millis()+Timeout;

  if ( !m_openFlag )    return ER_LINK_BAD;
  if ( Count < 1 )      return ER_LINK_CANT;
  if ( Count > 0x7FFF ) return ER_LINK_CANT;
  

  //falls ein vorheriger Transfer noch nicht abgeschlossen ist
  while(!link0_tx_buffer_empty()){
     heartbeat();
     if(millis() > in_time){
        timeOutFlag = true;
        break;
     }
  }


 #ifdef DEBUG_ON
printf(" --->  [");  
#endif

  while((inCount < Count) && !timeOutFlag){

    if(link0_rx_buffer_count()>0){
       Buffer[inCount] = link0_wait_get_data(1);
       inCount++;

    }else{
        heartbeat();
        if(millis() > in_time){
            timeOutFlag = true;

            #ifdef DEBUG_ON
            printf("!!!raus A!!!\r\n");
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



AS_FASTRUN  int asTpLinkTeensy::WriteLink(   char *Buffer, unsigned int Count, int Timeout ){
 

#ifdef DEBUG_ON   
  printf("asTpLinkTeensy::WriteLink count %d Tout: %d [",Count,Timeout);
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
  while(!link0_tx_buffer_empty()){
     heartbeat();
     if(millis() > out_time){
        timeOutFlag = true;
        break;
     }
  }
//#ifdef TPT_CODE 
  /*printf("A ivalid %d\r\n",digitalRead(C011_IVALID));*/
  while((outCount < Count) && !timeOutFlag ){

       link0_add2_tx_queue(Buffer[outCount]);
       heartbeat();

#ifdef DEBUG_ON   
       if(outCount < 16) {
         printf(" %02x",Buffer[outCount]);
       }
#endif

      while(!link0_tx_buffer_empty() || link0_tx_buffer_waiting_for_ack){

        heartbeat();

        if(millis() > out_time){
            timeOutFlag = true;
            break;
        }
      }

      outCount++;


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

// #else
// outCount = Count;
// printf("asTpLinkTeensy::WriteLink count %d Tout: %d \r\n",Count,Timeout);
// #endif  
  return outCount;
  
}



/*
gegen hängenden Adapter, sehr tricky
*/
void asTpLinkTeensy::adapterReset(){
    // Reset RX buffer & variables
    received_ack_count = 0;
    received_data_head = 0;
    received_data_tail = 0;
    we_have_pending_ack = false;
    for(int i=0; i < LINK0_RECEIVE_BUFFER_SIZE; i++)
      link0_rx_buffer[i] = 0;

    // Reset TX buffer & variables
    link0_tx_buffer_waiting_for_ack = false;
    tx_buffer_head = 0;
    tx_buffer_tail = 0;
    for(int i=0; i < LINK0_TRANSMIT_BUFFER_SIZE; i++)
      link0_tx_buffer[i] = 0;

    serialPort->flush();
    serialPort->clear();
    /*while(serialPort->available()) serialPort->read();*/

}

int asTpLinkTeensy::TestRead(     ){
  if ( !m_openFlag )    return ER_LINK_BAD;
  /*if(digitalRead(C011_OVALID)) return 1;*/
  return 0;  
}
int asTpLinkTeensy::TestWrite(    ){
  if ( !m_openFlag )    return ER_LINK_BAD;
  return 1;  
}

bool asTpLinkTeensy::hasInByte(){
   /*return digitalRead(C011_OVALID);*/
   heartbeat();
   if(link0_rx_buffer_count()>0) return true;
   return false;
}

