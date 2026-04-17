
#pragma once

#define DEBUG_ON_NO 1
#include "as_transputerlink.h"


#define C011_IACK     40
#define C011_IVALID   36
#define C011_OACK     37
#define C011_OVALID   27



class asTpLinkC011 : public asTransputerLinkBase{

   public:
      asTpLinkC011(int pinReset,int pinAnalyse,int pinError);
      virtual ~asTpLinkC011();
      virtual char * AdapterName() { return "asTpLinkC011"; }
      virtual     int OpenLink(    char *Name);
      virtual     int CloseLink(    );
      virtual     int ReadLink(    char *Buffer, unsigned int Count, int Timeout );
      virtual     int WriteLink(   char *Buffer, unsigned int Count, int Timeout );


      virtual     int TestRead(     );
      virtual     int TestWrite(    );
      /* Softreset des Linkadapters*/


      /*sehr gefährlich*/
      void adapterReset(void);
      void write_C011(uint8_t inV);
      uint8_t read_C011();

      /*
      unabhängig vom Openstatus, damit einkommende daten geprüft werden können
      */
      virtual bool hasInByte();
};
