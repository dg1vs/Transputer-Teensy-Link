

#include "as_ethernet.h"
#include "as_usbhub.h"
#include "as_linkusb.h"
#include "as_transputerlink.h"
#include <SPI.h>
#include <LibPrintf.h>

#ifdef MIT_OLEDDISP

#include "as_tpdisplay.h"

#endif

//#define SPI_ETHERNET_SETTINGS SPISettings(30000000, MSBFIRST, SPI_MODE0) in w5100.h, EthernetLib

void as_processClientB300(EthernetClient & client);
void as_closedClientB300(EthernetClient & client);

extern asTransputerLinkBase * tpLinkMain;
extern asTransputerLinkBase * tpLinkUSB;

extern ASUSBLink usbLinkHandler;

extern int usb_payloadpakets;

byte as_mac[] = {
  /*0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED  Linkadapter V1 auf Tennes 4.0*/
    0xDE, 0xAD, 0xBE, 0xEF, 0xF0, 0xE0 /*Linkadapter V2 auf Tennes 4.1, natives Ethernet*/
};
/*IPAddress as_ip(192, 168, 1, 177);*/

// Initialize the Ethernet server library
// with the IP address and port you want to use

//B300 Standardport
EthernetServer as_Server_B300(AS_B300_MAIN_PORT);
//Speedtest
EthernetServer as_Server_Speed(4040);
//WWW, Monitor
EthernetServer as_Server_WWW(80);


static EthernetClient clientB300;

bool isB300Connected(){
   return clientB300 && clientB300.connected();
}

static bool lastB300 = false;
static long usbpkt = 0;

void displayB300Status(){
     char buf[32];
     *buf=0;
     #ifdef MIT_OLEDDISP

      disp_clearLine(2);
      sprintf(buf,"B300 (%d) %s",AS_B300_MAIN_PORT, (lastB300 ? "connected" : "listen"));
      disp_writeLine(buf, 2);
      
      disp_clearLine(3);
      sprintf(buf,"USB Pkt: %d",usbpkt);
      disp_writeLine(buf, 3);

      disp_flush();
      
     #endif
}

void as_eth_setup(){
  
  // start the Ethernet connection and the server:
  //nutze DHCP
  char dispBuf[64];
  *dispBuf=0;
  printf("suche IP ... ");

#ifdef MIT_OLEDDISP

  disp_clearLine(1);
  disp_writeLine("Suche IP...", 2);
  disp_flush();

#endif

  Ethernet.begin(as_mac);//, ip);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  // start the server
  as_Server_B300.begin();
  as_Server_Speed.begin();
  as_Server_WWW.begin();

  IPAddress _ip = Ethernet.localIP();
  printf(" %d.%d.%d.%d",_ip[0],_ip[1],_ip[2],_ip[3]);
  


#ifdef MIT_OLEDDISP

  disp_clearLine(2);
  disp_clearLine(1);
  sprintf(dispBuf,"IP: %d.%d.%d.%d",_ip[0],_ip[1],_ip[2],_ip[3]);
  disp_writeLine(dispBuf, 1);
  displayB300Status();

#endif  

#ifdef HOST_NAME  
  printf(" / %s%x%x%x", HOST_NAME,as_mac[3],as_mac[4],as_mac[5]);
#endif  
  printf("  MAC [%X:%X:%X:%X:%X:%X]\r\n", as_mac[0],as_mac[1],as_mac[2], as_mac[3],as_mac[4],as_mac[5]);
 
  
  printf("Port B300   %d\r\n",AS_B300_MAIN_PORT);
  printf("Port Speed  4040\r\n");
  printf("Port WWW    80\r\n");
  
  
}

int wwwTick=0;
void printToHtml(EthernetClient * client);



void as_eth_beat(){
  static long lastmicros=0;
    // listen for incoming clients
  EthernetClient clientSpeed = as_Server_Speed.available();
    //blockiert nicht

  if (clientSpeed) {
      //blockiert
      char speedBuffer[1024];
      printf("new client Speed Messung\r\n");
      //cat helios-1.31.tar | nc 192.168.0.28 4040
      uint64_t anzahl=0;
      uint64_t startMillis = millis();
      while (clientSpeed.connected()) {
        if (clientSpeed.available()) {
            int RecvSize = clientSpeed.read((uint8_t*)speedBuffer, 1024);
            anzahl += RecvSize;
            
        }
      }
      int deltaMS = int(millis()-startMillis);
      double recVal = anzahl;
      recVal /= (1024*1024);

      double b_sek = anzahl;
      b_sek *=1000;
      b_sek /= deltaMS;
      b_sek /=1024;

      printf("%.02f MB %d ms %.02f kBytes/Sek\r\n",recVal,deltaMS,b_sek);



      delay(1);
      // close the connection:
      clientSpeed.stop();
      printf("clientSpeed disconnected\r\n");
  }
  

  EthernetClient clientWWW = as_Server_WWW.available();

  if (clientWWW) {
    //blockiert
    /*printf("new client WWW\r\n");*/
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    //uint64_t anzahl=0;
    //uint64_t startMillis = millis();
    
    while (clientWWW.connected()) {
      //Dummyausgabe aus Webserver 
      if (clientWWW.available()) {
         char c = clientWWW.read();
         char outBuf[128];
         *outBuf=0;

        
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          clientWWW.println("HTTP/1.1 200 OK");
          clientWWW.println("Content-Type: text/html");
          clientWWW.println("Connection: close");  // the connection will be closed after completion of the response
          clientWWW.println("Refresh: 5");  // refresh the page automatically every 5 sec
          clientWWW.println();
          clientWWW.println("<!DOCTYPE HTML>");
          clientWWW.println("<html>");
          clientWWW.println("<head>");
          clientWWW.println("<style>");
          clientWWW.println("body {");
          clientWWW.println("  font-family: courier, serif;");
          clientWWW.println("}");
          clientWWW.println("</style>");
          clientWWW.println("</head>");
          clientWWW.println("<body>");
          /*clientWWW.print("Teensy Linkadapter Monitor");*/
          clientWWW.println("****************************************************<br>");

          clientWWW.println("TransputerLink Adapter V3 <br>");
          clientWWW.println("C011, TeensyLink, B300-Emu, Audio, USB-Host<br>");
          clientWWW.println("Andre Saischowa 2026<br>");
          clientWWW.print( F("Compiled: "));
          clientWWW.print( F(__DATE__));
          clientWWW.print( F(", "));
          clientWWW.println( F(__TIME__));
          clientWWW.println( F("WWW-Tick: "));
          clientWWW.println( wwwTick++);
          
          clientWWW.println("<br>");
          clientWWW.println("****************************************************<br>");

          clientWWW.println("<h4>Network</h4>");
          clientWWW.println("<ul>");
            IPAddress _ip = Ethernet.localIP();
            sprintf(outBuf,"<li>IP: %d.%d.%d.%d / MAC [%X:%X:%X:%X:%X:%X]</li>",_ip[0],_ip[1],_ip[2],_ip[3],as_mac[0],as_mac[1],as_mac[2], as_mac[3],as_mac[4],as_mac[5]);
            clientWWW.println(outBuf);
            sprintf(outBuf,"<li>Ports B300 %d / WWW 80 / Speed 4040(debug)</li>",AS_B300_MAIN_PORT);
            clientWWW.println(outBuf);
                     
          clientWWW.println("</ul>");
 
          if(tpLinkMain){
             float writeSum = tpLinkMain->writeCountSum();
             float readSum  = tpLinkMain->readCountSum();
             sprintf(outBuf,"<h4>Main-Link: %s</h4>",tpLinkMain->AdapterName());
             clientWWW.println(outBuf); 
             clientWWW.println("<ul>");
             clientWWW.println("  <li>");

               clientWWW.println("     Write ");
               if(writeSum > (1024*1024)){
                 sprintf(outBuf,"%.03f MB",writeSum/(1024*1024));
               }else{
                 sprintf(outBuf,"%.03f kB",writeSum/(1024));
               }
               clientWWW.println(outBuf);            
               clientWWW.println("     Read ");
               if(readSum > (1024*1024)){
                 sprintf(outBuf,"%.03f MB",readSum/(1024*1024));
               }else{
                 sprintf(outBuf,"%.03f kB",readSum/(1024));
               }               
               clientWWW.println(outBuf);

             clientWWW.println("  </li>");

             /*sprintf(outBuf,"<li>USB Payload-Pkt. Send %d</li>",usbLinkHandler.usb_payloadpakets);
             clientWWW.println(outBuf);*/
            

             sprintf(outBuf,"<li>TestRead %d TestWrite %d,TestError %d</li>",tpLinkMain->TestRead(),tpLinkMain->TestWrite(),tpLinkMain->TestError());
             clientWWW.println(outBuf);

             /*sprintf(outBuf,"<li>IAck %d IValid %d OAck %d OValid %d (Pins)</li>",digitalRead(C011_IACK),digitalRead(C011_IVALID),digitalRead(C011_OACK),digitalRead(C011_OVALID));*/

             //clientWWW.println(outBuf);

             /*sprintf(outBuf,"<li>Error %d Reset %d Analyze %d (Pins)</li>",digitalRead(C011_ERROR),digitalRead(C011_RESET),digitalRead(C011_ANALYSE));
             clientWWW.println(outBuf);*/

             sprintf(outBuf,"<li>hasInByte %d (Pins)</li>",(tpLinkMain->hasInByte()));
             clientWWW.println(outBuf);


             clientWWW.println("</ul>");

          }else{
            clientWWW.println("tpLinkMain nicht verfügbar<br>");
          }

          clientWWW.println("<h4>B300-Client</h4>");
          if(clientB300) {
             clientWWW.println("<ul><li><b>active</b></li></ul>");
          }else{
            clientWWW.println("<ul><li>not active</li></ul>");
          }
          

          if(tpLinkUSB){
             float writeSum = tpLinkUSB->writeCountSum();
             float readSum  = tpLinkUSB->readCountSum();
             sprintf(outBuf,"<h4>USB-Link: %s</h4>",tpLinkUSB->AdapterName());
             clientWWW.println(outBuf); 

             clientWWW.println("<ul>");
             clientWWW.println("  <li>");

               clientWWW.println("     Write ");
               if(writeSum > (1024*1024)){
                 sprintf(outBuf,"%.03f MB",writeSum/(1024*1024));
               }else{
                 sprintf(outBuf,"%.03f kB",writeSum/(1024));
               }
               clientWWW.println(outBuf);            
               clientWWW.println("     Read ");
               if(readSum > (1024*1024)){
                 sprintf(outBuf,"%.03f MB",readSum/(1024*1024));
               }else{
                 sprintf(outBuf,"%.03f kB",readSum/(1024));
               }               
               clientWWW.println(outBuf);

             clientWWW.println("  </li>");

             sprintf(outBuf,"<li>USB Payload-Pkt. Send %d</li>",usbLinkHandler.usb_payloadpakets);
             clientWWW.println(outBuf);
            

             sprintf(outBuf,"<li>TestRead %d TestWrite %d,TestError %d</li>",tpLinkUSB->TestRead(),tpLinkUSB->TestWrite(),tpLinkUSB->TestError());
             clientWWW.println(outBuf);

             /*sprintf(outBuf,"<li>IAck %d IValid %d OAck %d OValid %d (Pins)</li>",digitalRead(C011_IACK),digitalRead(C011_IVALID),digitalRead(C011_OACK),digitalRead(C011_OVALID));*/

             //clientWWW.println(outBuf);

             /*sprintf(outBuf,"<li>Error %d Reset %d Analyze %d (Pins)</li>",digitalRead(C011_ERROR),digitalRead(C011_RESET),digitalRead(C011_ANALYSE));
             clientWWW.println(outBuf);*/

             sprintf(outBuf,"<li>hasInByte %d (Pins)</li>",(tpLinkUSB->hasInByte()));
             clientWWW.println(outBuf);


             clientWWW.println("</ul>");

          }else{
            clientWWW.println("tpLinkUSB nicht verfügbar<br>");
          }
          printToHtml(&clientWWW);
          clientWWW.println("</body></html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }

      }
      
    }

    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    clientWWW.stop();
    
    
    /*printf("clientWWW disconnected\r\n");*/
  }
  /*
  //Handling Client B300  , OK, blockiert
  EthernetClient clientB300 = as_Server_B300.available();
  if(clientB300){
    Serial.println("new client B300");

    while (clientB300.connected()) {
      if (clientB300.available()) {
        
        as_processClientB300(clientB300);

      }
      
    }
    // close the connection:
    clientB300.stop();
   
    as_closedClientB300(clientB300);
    Serial.println("clientB300 disconnected");

  }
  */

  
  EthernetClient newClientB300 = as_Server_B300.accept();

  if(newClientB300){
    printf("new clientB300 accepted\r\n");
    clientB300 = newClientB300;
  }
  
  do{
    if(!clientB300) break;

    /* printf("new clientB300 accepted A\r\n");*/
    if(!clientB300.connected()) {
        /*printf("new clientB300 accepted B\r\n");*/
        clientB300.stop();
        /*printf("new clientB300 accepted C\r\n");*/
        as_closedClientB300(clientB300);
        /*printf("clientB300 disconnected\r\n");*/
        break;
    }
     /*printf("new clientB300 accepted D\r\n");*/
    if (!clientB300.available()) break;
    /* printf("new clientB300 accepted E\r\n");*/
    as_processClientB300(clientB300);
    
    break;
    
  }while(clientB300);

  if(micros() > lastmicros){

    if(lastB300 != clientB300.connected() || usbpkt != usbLinkHandler.usb_payloadpakets){
      lastB300 = clientB300.connected();
      usbpkt =usbLinkHandler.usb_payloadpakets;
      displayB300Status();

    }

    lastmicros = micros()+400000;
  }


}
