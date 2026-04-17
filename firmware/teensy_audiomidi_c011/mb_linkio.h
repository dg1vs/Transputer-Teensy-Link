#pragma once
///////////////////////////////////////////////////////////////////////////////////////////////////
//
//               L I N K I O
//
//
//  FILE:        linkio.h
//
//  PURPOSE:     Headerfile (DOS, OS2, WIN32, LINUX, MINIX, VMS & TOS)
//  A.Saischowa  Teensy
//
//               Copyright (C) 2016 M. Br"ustle, Vienna (AT)
//
///////////////////////////////////////////////////////////////////////////////////////////////////

    //  the linkio file header

    #define LINK_SUCCEEDED   0
    #define ER_LINK_BAD     -1
    #define ER_LINK_CANT    -2
    #define ER_LINK_SOFT    -3
    #define ER_LINK_NODATA  -4
    #define ER_LINK_NOSYNC  -5
    #define ER_LINK_BUSY    -6
    #define ER_NO_LINK      -7
    #define ER_LINK_SYNTAX  -8

    #define NO_LINK_TIMEOUT  0

    #define SYSTEM_CONTROL_DELAY_ENV_NAME "LINKIO_SYSTEM_CONTROL_DELAY"
    #define SYSTEM_CONTROL_DELAY_DEFAULT 100

    #ifdef _TOS
        #define TYPEMOD
        #define LINKAGE
        typedef void *LINK;
        #define INVALID_LINK NULL
        typedef struct _FUNCPTR {
            int cdecl (*OpenLink)(    char *Name, LINK *link );
            int cdecl (*CloseLink)(   LINK link );
            int cdecl (*ReadLink)(    LINK Link, char *Buffer, unsigned int Count, int Timeout );
            int cdecl (*WriteLink)(   LINK Link, char *Buffer, unsigned int Count, int Timeout );
            int cdecl (*ResetLink)(   LINK Link );
            int cdecl (*AnalyseLink)( LINK Link );
            int cdecl (*TestError)(   LINK link );
            int cdecl (*TestRead)(    LINK link );
            int cdecl (*TestWrite)(   LINK link );
        } FUNCPTR;
    #endif

    typedef void *LINK;
    
    int OpenLink(    char *Name, LINK *link );
    int CloseLink(   LINK link );
    int ReadLink(    LINK Link, char *Buffer, unsigned int Count, int Timeout );
    int WriteLink(   LINK Link, char *Buffer, unsigned int Count, int Timeout );
    int ResetLink(   LINK Link );
    int AnalyseLink( LINK Link );
    int TestError(   LINK link );
    int TestRead(    LINK link );
    int TestWrite(   LINK link );
