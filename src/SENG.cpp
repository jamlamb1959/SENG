
#include <Arduino.h>

#include <iostream>

#include <string.h>

#include "SENG.h"
#include "Locker.h"

#include <freertos/FreeRTOS.h>

#include <freertos/task.h>

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
                << "\n";
        }

    static char ringBuffer[ 100 ];
    static uint8_t inIdx = 0;

    bool haveSig;

    std::map< std::string, std::string >::const_iterator it;
    std::map< std::string, ST * >::const_iterator sit;

    std::string * nsPtr;
    std::string sigName;

    int depth;

    ST * ost;

    if ( ivVerbose > 2 )
        {
        TOUT << "(" << ivDepth << ") - signal( '" << aSigName << "' ), ivCur: '"
                <<  ((ivCur != NULL) ? ivCur->getName().c_str() : "(NULL)") << "'" << std::endl;
        }

    uint8_t idx;

    {
    Locker l( ivMut, __FILE__, __LINE__ );

    const char * sPtr;

    for( sPtr = aSigName.c_str(), idx = inIdx; *sPtr != '\0'; sPtr ++, inIdx = (inIdx + 1) % sizeof( ringBuffer ) )
        {
        ringBuffer[ inIdx ] = *sPtr;
        }

    /*
    ** include the null character to avoid having to use outIdx;
    */
    ringBuffer[ inIdx ] = *sPtr;
    inIdx ++;
    }

    xQueueGenericSend( ivEvtQ, &idx, 1000, queueSEND_TO_BACK );

    {
    Locker l( ivMut, __FILE__, __LINE__ );
    
    depth = ivDepth ++;

    if ( depth != 0 )
        {
        ivDepth --;
    
        return;
        }
    }

    /*
    ** only one task gets to this point because of the logic above.
    ** so the fifo should be ok because only 1 task is changing the out index.
    */
    for( ; ; )
        {
        haveSig = xQueueReceive( ivEvtQ, &idx, 60000 );

        if ( !haveSig )
            {
            break;
            }

        for( sigName.clear(); ringBuffer[ idx ] != '\0'; idx = (idx + 1) % sizeof( ringBuffer ) )
            {
            sigName += ringBuffer[ idx ];
            }

        if ( ivCur != NULL )
            {
            nsPtr = ivCur->signal( sigName );

            if ( nsPtr != NULL )
                {
                if ( nsPtr->length() == 0 )
                    {
                    /*
                    ** If the event is found but the next state name is empty.
                    */
                    continue;
                    }

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
                        ivCur->exec();
                        continue;
                        }
                    }
                }
            }

        it = ivGE.find( sigName );

        if ( it == ivGE.end() )
            {
            continue;
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
                ivCur->exec();
                continue;
                }
            }
        }

    /*
    ** need to lock up so only a single thread is allowed to change the state.
    */
    Locker l( ivMut, __FILE__, __LINE__ );

    ivDepth --;

    if ( ivVerbose )
        {
        TOUT << "SM::signal(exit) - aSigName: " << aSigName << "\n";
        }
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
        : ivDepth( 0 )
        , ivVerbose( 1 )
        , ivCur( NULL )
    {
    ivMut = xSemaphoreCreateMutex();
    ivEvtQ = xQueueCreate( 10, sizeof( const char * ) );
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
            TOUT << "State: '"<< ivName << "' SP: \"" << *ivSP << "\"\n";
            }
        ivSP->exec();
        }
    else
        {
        TOUT << "State: '" << ivName << "', (ivSP == NULL)\n";
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
Serial.printf( "signal( \"tmo\" ): t: %lu, ivTmo: %lu\n",
            t, ivTmo );

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

