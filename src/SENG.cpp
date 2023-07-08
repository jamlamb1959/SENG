
#include <Arduino.h>

#include <ArduinoHttpClient.h>
#include <WiFi.h>

#include <iostream>

#include <string.h>

#include "SENG.h"
#include "Locker.h"

// #include <freertos/FreeRTOS.h>

// #include <freertos/task.h>

#ifndef TOUT
#define TOUT std::cout << __FILE__ << "(" << __LINE__ << "): "
#endif

SM * SM::instance(
        )
    {
    static SM * inst = new SM();

    return inst;
    }

bool SM::load( 
        const char * aConfig 
        )
    {
    const char * wp;

    std::string cmd;
    std::string lin;

    while( *aConfig != '\0' )
        {
        getTkn( lin, aConfig, "\r\n" );
        wp = lin.c_str();

        getTkn( cmd, wp, " \n\r\t" );
        
        if ( cmd == "event" )
            {
            _prcsEvent( wp );
            }
        else if ( cmd == "state" )
            {
            _prcsState( wp );
            }
        }

    return true;
    }

extern void * pxCurrentTCB;

void SM::signal( 
        const std::string & aSigName 
        )
    {
    if ( ivVerbose )
        {
        TOUT << "SM::signal(enter) - aSigName: " << aSigName 
                << "(depth: " << (ivInIdx - ivOutIdx) << "), (ivCur: '"
                <<  ((ivCur != NULL) ? ivCur->getName().c_str() : "(NULL)") << "')" << std::endl;
        }

    if ( (aSigName.length() + 1) > (sizeof( ivEvtBuf ) - (ivInIdx - ivOutIdx)) )
        {
        Serial.println( "ivEvtBuf(overflow)" );
        return;
        }

    const char * sPtr;

    for( sPtr = aSigName.c_str(); *sPtr != '\0'; sPtr ++, ivInIdx ++ )
        {
        ivEvtBuf[ ivInIdx % sizeof( ivEvtBuf ) ] = *sPtr;
        }

    /*
    ** include the null character to avoid having to use outIdx;
    */
    ivEvtBuf[ ivInIdx % sizeof( ivEvtBuf ) ] = '\0';
    ivInIdx ++;

    TOUT << "queued signal: " << aSigName << ", ivInIdx: " << ivInIdx << ", ivOutIdx: " << ivOutIdx
            << ", ivCur: " <<  ((ivCur != NULL) ? ivCur->getName().c_str() : "(NULL)") << "')" << std::endl;
    return;
    }

bool SM::exe(
        )
    {
    bool haveSig;

    std::map< std::string, std::string >::const_iterator it;
    std::map< std::string, ST * >::const_iterator sit;

    std::string * nsPtr;
    char sigName[ 20 ];

    int depth;

    ST * ost;

    uint8_t destIdx;
    uint8_t idx;

    if ( ivInIdx == ivOutIdx )
        {
        return false;
        }

    for( destIdx = 0; 
            (ivInIdx != ivOutIdx) &&
            (destIdx < sizeof( sigName )) && (ivEvtBuf[ ivOutIdx % sizeof( ivEvtBuf ) ] != '\0');
             destIdx ++, ivOutIdx ++ ) 
        {
        sigName[ destIdx ] = ivEvtBuf[ ivOutIdx % sizeof( ivEvtBuf ) ];
        }

    if ( destIdx < sizeof( sigName ) )
        {
        sigName[ destIdx ] = '\0';
        ivOutIdx ++;
        }
    else
        {
        Serial.println( "buffer underrun..." );
        }

    Serial.printf( "%s(%d) - %lu - SM::exe - sigName: %s, ivCur: %s\r\n", __FILE__, __LINE__, 
            millis(), sigName, ((ivCur != NULL) ? ivCur->getName().c_str() : "(NULL)") );

    if ( ivCur != NULL )
        {
        nsPtr = ivCur->signal( std::string( sigName ) );

        if ( nsPtr != NULL )
            {
            Serial.printf( "%s(%d) - *nsPtr: %s\r\n", 
                    __FILE__, __LINE__, (*nsPtr).c_str() );

            if ( nsPtr->length() != 0 )
                {
                sit = ivST.find( *nsPtr );
                if ( sit != ivST.end() )
                    {
                    ost = ivCur;
    
                    ivCur = sit->second;
                     
                    if ( ost != ivCur )
                        {
                        ivTmo = 0;
                        }

                    if ( ivCur != NULL )
                        {
                        Serial.printf( "%s(%d) - ivCur: %s ->exec (return true)\r\n",
                                __FILE__, __LINE__, ivCur->getName().c_str() );

                        ivCur->exec();
                        return true;
                        }
                    }
                }
            }
        }

    it = ivGE.find( std::string( sigName ) );

    if ( it == ivGE.end() )
        {
        Serial.printf( "'%s' - not found in global event.(return true)\r\n", sigName );
        return true;
        }

    sit = ivST.find( it->second );

    if ( sit != ivST.end() )
        {
        ost = ivCur;

        ivCur = sit->second;
                    
        if ( ost != ivCur )
            {
            ivTmo = 0;
            }
            
        if ( ivCur != NULL )
            {
            Serial.printf( "%s(%d) - ivCur: %s ->exec (return true)\r\n",
                    __FILE__, __LINE__, ivCur->getName().c_str() );
            ivCur->exec();
            return true;
            }
        }

    Serial.printf( "SM::exe(exit) - sigName: %s, ivDepth: %u, ivCur: %s (return true)\r\n", 
                sigName, (ivInIdx - ivOutIdx),
                ((ivCur != NULL) ? ivCur->getName().c_str() : "(NULL)") );
    return true;
    }
    
void SM::signal( 
        const char * const aSigName 
        )
    {
    std::string sig( aSigName );
    signal( sig );
    }

SM::SM(
        )
        : ivInIdx( 0 )
        , ivOutIdx( 0 )
        , ivVerbose( 1 )
        , ivCur( NULL )
    {
    // ivEvtQ = xQueueCreate( 10, sizeof( const char * ) );
    }

SM::~SM(
        )
    {
    }

SPMOM::SPMOM( 
        const char * const aClsName, 
        SP * (*aMOM) () 
        )
        : ivMOM( aMOM )
    {
    static SPFactory * spf = SPFactory::instance();

    spf->addSPMOM( aClsName, this );
    }

void SM::_prcsEvent( 
        const char * & aWP 
        )
    {
    std::string evtName;
    std::string stName;

    getTkn( evtName, aWP, " \t\r\n" );

    skipWS( aWP );

    if ( (*aWP != '\0') && (strchr( "\r\n", *aWP ) == NULL ) )
        {
        getTkn( stName, aWP, " \t\r\n" );
        }
        
    if ( ivCur != NULL )
        {
        ivCur->addEvent( evtName, stName );
        }
    else
        {
        ivGE[ evtName ] = stName;
        }
    }

void SM::_prcsState( 
        const char * & aWP 
        )
    {
    std::string stName;

    getTkn( stName, aWP, " \t\r\n" );

    ST * st = new ST( stName );

    ivST[ stName ] = st;

    st->load( aWP );

    ivCur = st;
    }

SPFactory * SPFactory::instance(
        )
    {
    static SPFactory * inst = new SPFactory();

    return inst;
    }

void SPFactory::addSPMOM( 
        const char * const aSPName, 
        SPMOM * aSPMOM
        )
    {
    std::string key( aSPName );

    ivSPMOM[ key ] = aSPMOM;
    }

SP * SPFactory::create( 
        const std::string & aClsName 
        )
    {
    SPMOM * spmom;

    std::map< std::string, SPMOM * >::const_iterator it;

    it = ivSPMOM.find( aClsName );
    if ( it == ivSPMOM.end() )
        {
        return NULL;
        }

    spmom = it->second;

    return (*(spmom->ivMOM)) ();
    }

void ST::addEvent( 
        const std::string & anEvtName,
        const std::string & aStateName 
        )
    {
    ivEvent[ anEvtName ] = aStateName;
    }

void ST::load( 
        const char * &aWP 
        )
    {
    static SPFactory * spf = SPFactory::instance();

    std::string clsName;

    getTkn( clsName, aWP, " \r\n\t" );

    ivSP = spf->create( clsName );
    if ( ivSP != NULL )
        {
        ivSP->load( aWP );
        }
    }

std::string * ST::signal( 
        const std::string & anEvtName 
        )
    {
    std::map< std::string, std::string >::iterator it;

    it = ivEvent.find( anEvtName );
    if ( it == ivEvent.end() )
        {
        return NULL;
        }

    return &(it->second);
    }

void ST::exec(
        )
    {
    static SM * sm = SM::instance();

    if ( ivSP != NULL )
        {
        if ( sm->getVerbose() > 2 )
            {
            TOUT << "ST::exec -  State: '"<< ivName << "' SP: \"" << *ivSP << "\"\n";
            }
        ivSP->exec();
        }
    else
        {
        TOUT << "ST::exec - State: '" << ivName << "', (ivSP == NULL)\n";
        }
    }

void ST::print(
        )
    {
    std::map< std::string, std::string >::const_iterator it;

    std::cout << "Name: '" << ivName << "'\n";
    
    std::cout << "  SP: ";
    if ( ivSP != NULL )
        {
        std::cout << *ivSP;
        }

    std::cout << "\n  # Event Table:\n";
    
    for( it = ivEvent.begin(); it != ivEvent.end(); it ++ )
        {
        std::cout << "  '" << it->first << "' -> '" << it->second << "\n";
        }
    }

void SM::print(
        )
    {
    std::map< std::string, std::string >::iterator it;
    std::map< std::string, ST * >::iterator sit;

    std::cout << "# GE\n";
    for( it = ivGE.begin(); it != ivGE.end(); it ++ )
        {
        std::cout << " " << it->first << " -> " << it->second << "\n";
        }

    std::cout << "# State Table\n";

    for( sit = ivST.begin(); sit != ivST.end(); sit ++ )
        {
        std::cout << "# State: " << sit->first << "\n";

        if ( sit->second != NULL )
            {
            sit->second->print();
            }
        }
    }

time_t SM::now(
        ) const
    {
    return time( NULL );
    }

void SM::setInv(
        const int anInv
        )
    {
    if ( anInv == 0 )
        {
        ivTmo = 0;
        }
        
    ivTmo = now() + anInv;
    }

void SM::setTmo( 
        const time_t aTmo 
        )
    {
    ivTmo = aTmo;
    }
    
/*
** This gets invoked on each loop.
*/
void SM::tick( 
        )
    {
    time_t t;

    if ( ivTmo != 0 )
        {
        t = now();

        if ( t >= ivTmo )
            {
            ivTmo = 0;
            signal( "tmo" );
            }
        }
    }

std::ostream & operator << ( 
        std::ostream & aStream, 
        const SP & anObj 
        )
    {
    anObj.print( aStream );

    return aStream;
    }

bool SM::loadHttp( 
        const char * const aHost, 
        const char * const aURI, 
        const int aPort
        )
    {
    const static std::string _init( "init" );

    bool ret;

    WiFiClient w;
    HttpClient cl( w, aHost, aPort );

    cl.beginRequest();
    cl.get( aURI );
    cl.endRequest();

    int rc = cl.responseStatusCode();
    String rsp = cl.responseBody();

    if ( rc != 200 )
        {
        Serial.printf( "rc: %d\r\nrsp:\r\n", rc );
        Serial.println( rsp );

        return false;
        }

    ret = load( rsp.c_str() );

    Serial.printf( "ret: %s\r\n", (ret) ? "TRUE" : "FALSE" );

    if ( ret )
        {
        signal( _init );
        }

    return ret;
    }

