
/*#include <Ethernet.h>*/
#include "as_ethernet.h"
#include "mb_b300.h"
#include "mb_linkio.h"
#include "as_transputerlink.h"

#include <LibPrintf.h>


//https://github.com/embeddedartistry/arduino-printf

#define htons(x) ( ((x)<< 8 & 0xFF00) | \
                   ((x)>> 8 & 0x00FF) )
#define ntohs(x) htons(x)

#define htonl(x) ( ((x)<<24 & 0xFF000000UL) | \
                   ((x)<< 8 & 0x00FF0000UL) | \
                   ((x)>> 8 & 0x0000FF00UL) | \
                   ((x)>>24 & 0x000000FFUL) )

#define ntohl(x) htonl(x)

#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif

int WSAGetLastError(){
  return -1;
}

static BOOL ProcessMessage( EthernetClient & client, CHAR *pBuffer, asTransputerLinkBase * tLink /*LINK Link*/ );
static VOID SetTransputerId( CHAR* Id );
static BOOL IsForTransputer( CHAR* Id );
//-------------------------------------------------------------------------------------------------


asTransputerLinkBase * transputerLink=NULL;

extern asTransputerLinkBase * tpLinkMain;

void as_linkIO_init(){
  
}
//auf jeden Fall mehr als OPS_MAX_PACKET allozieren
//siehe Mike HeapAlloc( GetProcessHeap(), 0, OPS_MAX_PACKET + 4 ))
char ops_buffer[OPS_MAX_PACKET*4]/* FASTRUN*/;

//bool openFlag=false;
void as_closedClientB300(EthernetClient & client){
  if(transputerLink) transputerLink->CloseLink();
  transputerLink=NULL;
  //openFlag=false;
#ifdef DEBUG_ON    
  printf("as_closedClient\r\n");
#endif  
}


void as_processClientB300(EthernetClient & client){
  int available= client.available();  
  if( available > 0){
     int rc,SendSize;

     

     if(transputerLink!=NULL){
      
       //as_processMessage(client,  ops_buffer,  RecvSize);
       /*printf("Prozessmessage\r\n");*/
       ProcessMessage( client,ops_buffer, transputerLink );

     }else{
        /*printf("as_processClientB300 A Anzahl %d\r\n",available);*/
            uint8_t * o_buf =(uint8_t*)ops_buffer;
            int RecvSize = client.read(o_buf, 512);

            /*printf("client.read = %d\r\n",RecvSize);*/
                
            if (RecvSize  < 0 ) {
                printf( "ERROR: recv() failed. rn");
            } else if ( RecvSize < INT(sizeof( LINKOPS_OPEN_COMMAND ))) {
                printf( "ERROR: recv() returns to less data. \r\n" );
            } else if ( strncmp( ops_buffer, SERVICE_MSG, strlen( SERVICE_MSG )) != 0 ) {
                printf( "ERROR: recv() returns wrong service msg\r\n" );
            } else if ( ops_buffer[ 40 ] != OCMD_OPEN ) {
                printf( "ERROR: recv() returns wrong service tag\r\n" );
            } else {
                  LINKOPS_OPEN_REPLY *Reply = (LINKOPS_OPEN_REPLY*)ops_buffer;
                  char Name[ sizeof( Reply->DeviceName )];
#ifdef DEBUG_ON  
                  printf("INFO: OPEN %s\r\n",((LINKOPS_OPEN_COMMAND*)ops_buffer)->ResourceName);
#endif                  
                  //Serial.print( "INFO: OPEN ");
                  //Serial.println(((LINKOPS_OPEN_COMMAND*)ops_buffer)->ResourceName );
                  
                  //asTransputerLinkBase * l  = new asTransputerLinkBase();
                  if (( rc = tpLinkMain->OpenLink( Name)) < 0 ) {
                  /*if (( rc = OpenLink( Name,  NULL )) < 0 ) {*/
                      printf( "ERROR: OpenLink() failed-(%d)", rc );
                      Reply->Status = ( rc == ER_LINK_BUSY ) ? STATUS_TARGET_UNAVAILABLE : STATUS_TARGET_NOTWORKING;
                  } else {
                      Reply->Status = STATUS_NOERROR;
                  } // endif 
          
                  Reply->Tag = OREPLY_OPEN;

                  memset( Reply->DeviceName, 0, sizeof( Reply->DeviceName ));
                  strncpy( Reply->DeviceName, Name, sizeof( Reply->DeviceName ) - 1 );
        

                  if (( SendSize = client.write(/* hSocket
                                       , */ops_buffer
                                       , sizeof( *Reply )
                                       /*, 0*/ )) == SOCKET_ERROR ) {
                      printf( "ERROR: send() failed. rc=%d\n", WSAGetLastError());
                      //delete(l);
                  } else if ( SendSize < INT(sizeof( *Reply ))) {
                      printf( "ERROR: send() to less data. Only %d\n", SendSize );
                      //delete(l);
                  } else {
                    transputerLink = tpLinkMain;
                    //openFlag = true;
                    // return Link;
                  } // endif 

            }
     }
  }else{
    printf("kein client verfügbar\r\n");
  }
}
//-------------------------------------------------------------------------------------------------
static BOOL ProcessMessage( EthernetClient & client, CHAR *pBuffer, asTransputerLinkBase * tLink /*LINK Link*/ ) {
//-------------------------------------------------------------------------------------------------

    BOOL fContinue = FALSE;
    INT n=0, rc=0, RecvSize=0, SendSize=0, PacketSize=0;

    if (( RecvSize = client.read( /*hSocket,*/
                         (uint8_t *)pBuffer
                         , OPS_MAX_PACKET
                         /*, 0 */)) == SOCKET_ERROR ) {
        printf( "ERROR: recv() failed. rc=%d\n", WSAGetLastError());
    } else if ( RecvSize < 3 ) {
        printf( "ERROR: recv() returned with %d. 3 required.\n", RecvSize );
    } else {

        PacketSize = ntohs( *( (USHORT*)pBuffer ));

        switch ( pBuffer[ 2 ] ) {

            case OCMD_RESET : {
                LINKOPS_RESET_COMMAND *Command = (LINKOPS_RESET_COMMAND*)pBuffer;
 #ifdef DEBUG_ON                 
                printf( "INFO: -------- RESET-------------\n" );
 #endif               
                if ( PacketSize < INT(sizeof( *Command ))) {
                    printf( "ERROR: Packet to small\n" );
                } else if ( ! IsForTransputer( Command->ProcessorID )) {
                    printf( "ERROR: ID is not transputer\n" );
                } else if (( rc = tLink->ResetLink(  )) < 0 ) {
                    printf( "ERROR: ResetLink() failed-(%d)", rc );
                } else {
                    fContinue = TRUE;
                } /* endif */
                n = 0;
                break;
            }
            case OCMD_ANALYSE : {
                LINKOPS_ANALYSE_COMMAND *Command = (LINKOPS_ANALYSE_COMMAND*)pBuffer;
#ifdef DEBUG_ON                  
                printf( "INFO: ------------ANALYSE-------------\n" );
#endif                
                if ( PacketSize < INT(sizeof( *Command ))) {
                    printf( "ERROR: Packet to small\n" );
                } else if ( ! IsForTransputer( Command->ProcessorID )) {
                    printf( "ERROR: ID is not transputer\n" );
                } else if (( rc = tLink->AnalyseLink(  )) < 0 ) {
                    printf( "ERROR: AnalyseLink() failed-(%d)", rc );
                } else {
                    fContinue = TRUE;
                } /* endif */
                n = 0;
                break;
            }
            case OCMD_TESTERROR : {
                LINKOPS_TESTERROR_COMMAND *Command = (LINKOPS_TESTERROR_COMMAND*)pBuffer;
                LINKOPS_TESTERROR_REPLY   *Reply   = (LINKOPS_TESTERROR_REPLY*)pBuffer;
#ifdef DEBUG_ON                  
                printf( "INFO: -----------TESTERROR--------------\n" );
#endif                
                if ( PacketSize < INT(sizeof( *Command ))) {
                    printf( "ERROR: Packet to small\n" );
                } else if ( ! IsForTransputer( Command->ProcessorID )) {
                    printf( "ERROR: ID is not transputer\n" );
                } else if (( rc = tLink->TestError(  )) < 0 ) {
                    printf( "ERROR: TestError() failed-(%d)", rc );
                    Reply->Status = STATUS_TARGET_NOTWORKING;
                } else {
                    Reply->Status = ( rc == 0 ) ? STATUS_NOERROR : STATUS_TARGET_ERROR;
                    fContinue = TRUE;
                } /* endif */
                n                 = sizeof( *Reply );
                Reply->PacketSize = htons( (USHORT)n );
                Reply->Tag        = OREPLY_TESTERROR;
                SetTransputerId( Reply->ProcessorID );
                break;
            }
          
            case OCMD_WRITELINK : {
                LINKOPS_WRITE_COMMAND *Command = (LINKOPS_WRITE_COMMAND*)pBuffer;
                LINKOPS_WRITE_REPLY   *Reply   = (LINKOPS_WRITE_REPLY*)pBuffer;
                INT Timeout = ntohs( Command->Timeout );
#ifdef DEBUG_ON               
                printf( "INFO: -----------OCMD_WRITELINK-----\n");
#endif                
                
                while (( n = ( PacketSize - RecvSize )) > 0 ) {
                    INT RecvNext = 0;
                    //if (n > 1024) n = 1024;
                    //printf("warte\r\n");
                    /*
                    while((n=client.available())==0){
                      
                    }*/
                    if((n=client.available()) > 0) { 
                      //printf("client.available() %d\r\n",n);
                      
                      if (( RecvNext = client.read( /*hSocket
                                           ,*/ (uint8_t *)(pBuffer + RecvSize)
                                           , size_t(n)
                                           /*, 0*/ )) == SOCKET_ERROR ) {
                          printf( "ERROR: recv() failed. rc=%d  %d\n", WSAGetLastError(),SOCKET_ERROR);
                          break;
                      } else if ( RecvNext <= 0 ) {
                          printf( "ERROR: recv() returned with %d\n", RecvNext );
                          break;
                      } else {
                          RecvSize += RecvNext;
                          //printf("rec: %d %d\r\n",RecvNext,RecvSize);
                      } /* endif */
                    }else{
                       //hier noch Timeout einbauen
                    }
                    
                } /* endwhile */
#ifdef DEBUG_ON                
                printf( "INFO: WRITELINK count: %d, timeout: %d %x/%x \n", PacketSize - sizeof( *Command ), Timeout, Timeout,Command->Timeout );
#endif
                if (( rc = tLink->WriteLink(  pBuffer    + sizeof( *Command )
                                    , PacketSize - sizeof( *Command )
                                    , Timeout )) < 0 ) {
                                      
               
                    printf( "ERROR: WriteLink() failed-(%d)", rc );
                    Reply->Status = STATUS_TARGET_NOTWORKING;
                } else {
                    Reply->Status = STATUS_NOERROR;
                } /* endif */
                
                n                 = sizeof( *Reply );
                Reply->PacketSize = htons( (USHORT)n );
                Reply->Tag        = OREPLY_WRITELINK;
                Reply->Written    = htons( (USHORT)rc );
                fContinue = TRUE;

                break;
            }
          
         
            case OCMD_READLINK : {
                LINKOPS_READ_COMMAND *Command = (LINKOPS_READ_COMMAND*)pBuffer;
                LINKOPS_READ_REPLY   *Reply   = (LINKOPS_READ_REPLY*)pBuffer;
                INT Count   = ntohs( Command->Count );
                INT Timeout = ntohs( Command->Timeout );
#ifdef DEBUG_ON                  
                printf( "INFO: --------  READLINK  count: %d, timeout: %d %x/%x\n", Count, Timeout,Timeout,Command->Timeout );
#endif                
                if ( PacketSize < INT(sizeof( *Command ))) {
                    printf( "ERROR: Packet to small\n" );
                    n = 0;
                } else if (( rc = tLink->ReadLink((char*)Reply->Data, Count , Timeout )) < 0 ) {
                    printf( "ERROR: ReadLink() failed-(%d)", rc );
                    n             = READ_REPLY_OVERHEAD;
                    Reply->Status = STATUS_TARGET_NOTWORKING;
                } else {
                    n             = rc + READ_REPLY_OVERHEAD;
                    Reply->Status = STATUS_NOERROR;
                } /* endif */
                Reply->PacketSize = htons( (USHORT)n );
                Reply->Tag        = OREPLY_READLINK;
                fContinue = TRUE;
                break;
            }
            case OCMD_CLOSE : {
                LINKOPS_CLOSE_COMMAND *Command = (LINKOPS_CLOSE_COMMAND*)pBuffer;
                LINKOPS_CLOSE_REPLY   *Reply   = (LINKOPS_CLOSE_REPLY*)pBuffer;
 #ifdef DEBUG_ON                 
                printf( "INFO: CLOSE\n" );
 #endif               
                if ( PacketSize < INT(sizeof( *Command ))) {
                    printf( "ERROR: Packet to small\n" );
                } else if (( rc = tLink->CloseLink(  )) < 0 ) {
                    printf( "ERROR: CloseLink() failed-(%d)", rc );
                } /* endif */
                n                 = sizeof( *Reply );
                Reply->PacketSize = htons( (USHORT)n );
                Reply->Tag        = OREPLY_CLOSE;
                Reply->Status     = STATUS_NOERROR;
                break;
            }
            default : {
                printf( "ERROR: UNKNOWN TAG %u\n", pBuffer[ 2 ] );
                n = 0;
                break;
            }
        } /* endswitch */

        if ( n == 0 ) {
            ;
        } else if (( SendSize = client.write(/* hSocket
                                   , */pBuffer
                                   , n
                                  /* , 0*/ )) == SOCKET_ERROR ) {
            printf( "ERROR: send() failed. rc=%d\n", WSAGetLastError());
            fContinue = FALSE;
        } else if ( SendSize < n ) {
            printf( "ERROR: send() to less data. Only %d of %d\n", SendSize, n );
            fContinue = FALSE;
        } else {
            ;
        } /* endif */
        //printf("ProcessMessage durch send %d \r\n",SendSize);
    } /* endif */

    return fContinue;
}
static BOOL IsForTransputer( CHAR* Id ) {
//-------------------------------------------------------------------------------------------------
    return ( Id[ 0 ] | Id[ 1 ] | Id[ 2 ] | Id[ 3 ] ) ? FALSE : TRUE;
}

//-------------------------------------------------------------------------------------------------
static VOID SetTransputerId( CHAR* Id ) {
//-------------------------------------------------------------------------------------------------
    Id[ 0 ] = Id[ 1 ] = Id[ 2 ] = Id[ 3 ] = 0;
}
