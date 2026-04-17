#pragma once


class asTransputerLinkBase{
   protected:
         
      void setReset_Pin(bool ein);
      void setAnalyse_Pin(bool ein);
      bool getError_Pin();

   public:
      asTransputerLinkBase(int pinReset=-1,int pinAnalyse=-1,int pinError=-1);
      virtual ~asTransputerLinkBase();
      virtual char * AdapterName() { return "asTransputerLinkBase"; }
      virtual int OpenLink(    char *Name);
      virtual int CloseLink(    );
      virtual int ReadLink(    char *Buffer, unsigned int Count, int Timeout );
      virtual int WriteLink(   char *Buffer, unsigned int Count, int Timeout );
      virtual int ResetLink(    );
      virtual int AnalyseLink(  );
      virtual int TestError(    );
      virtual int TestRead(     );
      virtual int TestWrite(    );
    
      int  readCount()  const { return m_readCount;};
      int  writeCount() const { return m_writeCount;};
      int  readCountSum()  const { return m_readCountSum;};
      int  writeCountSum() const { return m_writeCountSum;};      
      void ResetCounts();
      virtual bool isOpen() { return m_openFlag;}

      virtual void adapterReset(void) {};

      virtual  bool hasInByte() { return 0;}
   protected:
      bool m_openFlag;   
      int  m_readCount;
      int  m_writeCount;    
      int  m_readCountSum;
      int  m_writeCountSum;       
      unsigned long long m_openStamp;    

      int pin_reset;
      int pin_analyse;      
      int pin_error;  


};
