
///////////////////////////////////////////////////////////////////////////////////////////////////
//
//               L I N K 3 0 0
//
//
//  FILE:        link300.h
//
//  PURPOSE:     Headerfile
//
//
//               Copyright (C) 2016 M. Br"ustle, Vienna (AT)
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#define DEFAULT_NAME "B300"

  #define OCMD_OPEN                1
  #define OCMD_CLOSE               2
  #define OCMD_WRITELINK           3
  #define OCMD_READLINK            4
  #define OCMD_RESET               5
  #define OCMD_ANALYSE             6
  #define OCMD_TESTERROR           7

  #define OREPLY_OPEN             65
  #define OREPLY_CLOSE            66
  #define OREPLY_WRITELINK        67
  #define OREPLY_READLINK         68
  #define OREPLY_TESTERROR        71

  #define OEVENT_MESSAGE         128

  #define INMOSREPLY_BAD_SERVICE 255
  #define INMOSREPLY_BAD_VERSION 254


  #define STATUS_NOERROR            0
  #define STATUS_BAD_COMMS_MODE     1
  #define STATUS_BAD_SERVER         2
  #define STATUS_BAD_METHOD         3
  #define STATUS_BAD_TARGET_NAME    4
  #define STATUS_TARGET_UNAVAILABLE 5
  #define STATUS_TARGET_NOTWORKING  6
  #define STATUS_COMMS_CANT         7
  #define STATUS_COMMS_FATAL        8
  #define STATUS_LINK_FATAL         9
  #define STATUS_SP_ERROR          10
  #define STATUS_BAD_OPERATION     11
  #define STATUS_TARGET_ERROR      12
  #define STATUS_TIMEDOUT          13
  #define STATUS_ENDOFFILE         14

  #define SERVICE_MSG "inmos.com(tcplink-linkops-01.00)"

  #define OPS_MAX_PACKET 8192

  #pragma pack( 1 )

  #define MAX_RESOURCE_NAME 64
  
#ifndef CHAR
#define CHAR char
#endif
#ifndef BYTE
#define BYTE uint8_t
#endif
#ifndef USHORT
#define USHORT uint16_t
#endif

#ifndef INT
#define INT int
#endif


#ifndef BOOL
#define BOOL bool
#endif
#ifndef VOID
#define VOID void
#endif
#ifndef TRUE
#define TRUE true
#endif
#ifndef FALSE
#define FALSE false
#endif
  typedef struct _LINKOPS_OPEN_COMMAND {
      CHAR ServiceMsg[ 40 ];
      BYTE Tag;
      CHAR ResourceName[ 65 ];
  } LINKOPS_OPEN_COMMAND;
  

  typedef struct _LINKOPS_OPEN_REPLY {
      CHAR ServiceMsg[ 40 ];
      BYTE Tag;
      BYTE Status;
      CHAR DeviceName[ 65 ];
  } LINKOPS_OPEN_REPLY;

  #define MIN_OPEN_REPLY  42
  #define MIN_BASIC_REPLY  4

  typedef struct _LINKOPS_MESSAGE {
      USHORT PacketSize;
      BYTE   Tag;
      BYTE   Fatal;
      CHAR   Text[ 1 ];
  } LINKOPS_MESSAGE;

  typedef struct _LINKOPS_BASIC_REPLY {
      USHORT PacketSize;
      BYTE   Tag;
      BYTE   Status;
  } LINKOPS_BASIC_REPLY;

  typedef struct _LINKOPS_CLOSE_COMMAND {
      USHORT PacketSize;
      BYTE   Tag;
  } LINKOPS_CLOSE_COMMAND;

  typedef struct _LINKOPS_CLOSE_REPLY {
      USHORT PacketSize;
      BYTE   Tag;
      BYTE   Status;
  } LINKOPS_CLOSE_REPLY;

  typedef struct _LINKOPS_WRITE_COMMAND {
      USHORT PacketSize;
      BYTE   Tag;
      USHORT Timeout;
      /* note the data is transmitted separately */
  } LINKOPS_WRITE_COMMAND;

  typedef struct _LINKOPS_WRITE_REPLY {
      USHORT PacketSize;
      BYTE   Tag;
      BYTE   Status;
      USHORT Written;
  } LINKOPS_WRITE_REPLY;

  typedef struct _LINKOPS_READ_COMMAND {
      USHORT PacketSize;
      BYTE   Tag;
      USHORT Timeout;
      USHORT Count;
  } LINKOPS_READ_COMMAND;

  typedef struct _LINKOPS_READ_REPLY {
      USHORT PacketSize;
      BYTE   Tag;
      BYTE   Status;
      BYTE   Data[ 1 ];
  } LINKOPS_READ_REPLY;

  #define READ_REPLY_OVERHEAD  4

  typedef struct _LINKOPS_RESET_COMMAND {
      USHORT PacketSize;
      BYTE   Tag;
      CHAR   ProcessorID[ 4 ];
  } LINKOPS_RESET_COMMAND;

  typedef struct _LINKOPS_ANALYSE_COMMAND {
      USHORT PacketSize;
      BYTE   Tag;
      CHAR   ProcessorID[ 4 ];
  } LINKOPS_ANALYSE_COMMAND;

  typedef struct _LINKOPS_TESTERROR_COMMAND {
      USHORT PacketSize;
      BYTE   Tag;
      CHAR   ProcessorID[ 4 ];
  } LINKOPS_TESTERROR_COMMAND;

  typedef struct _LINKOPS_TESTERROR_REPLY {
      USHORT PacketSize;
      BYTE   Tag;
      BYTE   Status;
      CHAR   ProcessorID[ 4 ];
  } LINKOPS_TESTERROR_REPLY;
#ifdef MIKEB_CODE  
#endif  
