
#ifndef __SENG_h__
#define __SENG_h__

#include <iostream>

#include <map>
#include <string>

#include "Fifo.h"
#include "Filo.h"

#ifndef NOARDUINO
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#endif

void evalState( const std::string & aLin );

void getTkn( std::string & aTkn, const char * & aWP, const char * const aSepList );

void resetTmo();

void skipWS( const char * & aWP );

time_t tmo();

class CS
        : public Filo< std::string >
    {
  public:
    static CS * instance(
            )
        {
        static CS * inst = new CS();
        return inst;
        }

  private:
    CS(
            )
        {
        }
    };

class SQ
        : public Fifo< std::string >
    {
  public:
    static SQ * instance();

  private:
    SQ();
    };

class SP
    {
  public:
    virtual const char * className() const = 0;
    virtual void exec() = 0;
    virtual void load( const char * & aWP ) = 0;
    virtual void print( std::ostream & aStream ) const = 0;
    };

std::ostream & operator << ( std::ostream & aStream, const SP & anObj );


class ST
    {
  public:
    ST(
            const std::string & aName
            )
            : ivSP( NULL )
            , ivName( aName )
        {
        }

    void addEvent( const std::string & anEvtName,
            const std::string & aStateName );
            
    void exec();

    const std::string & getName(
                ) const
        {
        return ivName;
        }

    const char * getSPName(
            ) const
        {
        if ( ivSP != NULL )
            {
            return ivSP->className();
            }

        return "(NULL)";
        }

    void load( const char * &aWP );

    void print();

    /*
    ** return name of the next state.
    */
    std::string * signal( const std::string & anEventName );

  private:
    SP * ivSP;

    std::string ivName;

    std::map< std::string, std::string > ivEvent;
    };

class SPMOM
    {
  public:
    SPMOM( const char * const aClsName, SP * (*aMOM) () );

    SP * (* ivMOM) ();
    };

#define MOM( clsName ) \
        SP * clsName##_MOM() { return (SP *) new clsName(); } \
        static SPMOM clsName##MOM( #clsName, clsName##_MOM );

class SPFactory
    {
  public:
    static SPFactory * instance();

    void addSPMOM( const char * const aSPName, SPMOM * aSP );

    SP * create( const std::string & aClsName );

  private:
    SPFactory(
            )
        {
        }

    std::map< std::string, SPMOM * > ivSPMOM;
    };

class SM
    {
  public:
    static SM * instance();

    /*
    ** check the ivEveBuf
    ** if empty return false.
    ** else exec the oldest event in the buf.
    */
    bool exe();

    bool load( const char * aConfig );
    bool loadHttp( const char * const aHost, const char * const aURI, const int aPort = 80 );

    time_t now() const;

    void print();

    void signal( const char * const aSigName );
    void signal( const std::string & aSigName );

    /*
    ** Now + anInv
    */
    void setInv( const int anInv );

    int getVerbose(
            ) const
        {
        return ivVerbose;
        }

    void setVerbose( 
            const int aVerbose 
            )
        {
        ivVerbose = aVerbose;
        Serial.print( "ivVerbose: " ); Serial.println( ivVerbose );
        }

    /*
    ** sets ivTmo to aTmo
    */
    void setTmo( const time_t aTmo );

    /*
    ** if (ivTmo != 0) and (now >= ivTmo) 
    **   ivTmo = 0;
    **   signal(tmo)
    */
    void tick();

    void * readSMPUB();
    void writeSMPUB( void * aMsg );
    
  private:
    SM();
    ~SM();

    void _prcsEvent( const char * & aWP );
    void _prcsState( const char * & aWP );

    char ivEvtBuf[ 200 ];

    Fifo< void * > ivSMPUB;

    unsigned int ivInIdx;
    unsigned int ivOutIdx;

    int ivDepth;
    int ivVerbose;
    
    ST * ivCur;

    std::map< std::string, std::string > ivGE;
    std::map< std::string, ST * > ivST;

    // SemaphoreHandle_t ivMut;
    // QueueHandle_t ivEvtQ;

    time_t ivTmo;
    };

#endif
