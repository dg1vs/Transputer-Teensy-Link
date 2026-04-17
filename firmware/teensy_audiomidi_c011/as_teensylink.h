

#pragma once


#define DEBUG_ON_NO 1
#include "as_transputerlink.h"

#define LINK0_SPEED_5MBPS  1
#define LINK0_SPEED_10MBPS 2
#define LINK0_SPEED_20MBPS 3


/*

#define LINK0_TRANSMIT_BUFFER_SIZE (64*1024)  
#define LINK0_RECEIVE_BUFFER_SIZE (4*1024)
*/
#define LINK0_TRANSMIT_BUFFER_SIZE (4)  
#define LINK0_RECEIVE_BUFFER_SIZE (8)

class asTpLinkTeensy : public asTransputerLinkBase{

   private:
      /* xlink funktionen */
      void link0_process_incoming();
      void link0_send_ack();
      bool link0_check_for_ack();
      bool link0_tx_buffer_empty();
      byte link0_tx_buffer_get_front();
      void link0_send_data_wait_for_ack(byte b, unsigned int timeout);
      void link0_send_data_noack(byte b);
      bool link0_wait_for_ack(unsigned int c);
      bool link0_add2_tx_queue(byte data);
      long link0_rx_buffer_count();
      int link0_wait_get_data(unsigned int c);
      int link0_get_data_nowait();
      
   protected:

      void heartbeat();

   protected:
      HardwareSerialIMXRT * serialPort;
      byte link0_tx_buffer[LINK0_TRANSMIT_BUFFER_SIZE];
      unsigned int tx_buffer_head           = 0;
      unsigned int tx_buffer_tail           = 0;
      bool link0_tx_buffer_waiting_for_ack  = false;
      bool link0_tx_buffer_full             = false;

      byte link0_rx_buffer[LINK0_RECEIVE_BUFFER_SIZE];
      unsigned int received_data_head  = 0;
      unsigned int received_data_tail  = 0;
      unsigned int received_ack_count  = 0;
      bool we_have_pending_ack         = false;



   public:
      asTpLinkTeensy(HardwareSerialIMXRT * SerialPort, int pinReset,int pinAnalyse,int pinError);
      virtual ~asTpLinkTeensy();
      virtual char * AdapterName();

      virtual     int OpenLink(    char *Name);
      virtual     int CloseLink(    );
      virtual     int ReadLink(    char *Buffer, unsigned int Count, int Timeout );
      virtual     int WriteLink(   char *Buffer, unsigned int Count, int Timeout );


      virtual     int TestRead(     );
      virtual     int TestWrite(    );


      /*sehr gefährlich*/
      virtual void    adapterReset(void);
      void    write_LinkTeensy(uint8_t inV);
      uint8_t read_LinkTeensy();

      /*
      unabhängig vom Openstatus, damit einkommende daten geprüft werden können
      */
      bool hasInByte();
};
