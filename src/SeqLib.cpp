#ifdef NOARDUINO
#include <ESP.h>
#include <Serial.h>

#include <util.h>
#else
#include <Arduino.h>
#endif

#include <SeqLib.h>

#include <WiFi.h>

#include <SENG.h>

Blink::Blink( 
        const int aLed 
        )
        : ivState( false )
        , ivInv( 500 )
        , ivLed( aLed )
        , ivNxtTime( 0 )
    {
    Seq * s = Seq::instance();
    s->reg( *this );
    }

Blink::Blink( 
        const Blink & anObj 
        )
        : Seq::Task( anObj )
    {
    (void) anObj;
    }

Blink::~Blink(
        )
    {
    
    }

Blink & Blink::operator = ( 
        const Blink & anObj 
        )
    {
    Serial.printf( "Blink::operator =(entered)\r\n" );

    if ( this != &anObj )
        {
        }
    return *this;
    }

void Blink::lp(
        )
    {
    unsigned long int now = millis();

    if ( now > ivNxtTime )
        {
        ivNxtTime = millis() + ivInv;

        swtch();
        }
    }

void Blink::stp(
        )
    {
    Serial.printf( "Blink::stp(entered) - ivLed: %d, ivInv: %d\r\n",
            ivLed, ivInv );

    pinMode( ivLed, OUTPUT );
    }

void Blink::setInv( 
        int aMilliSeconds
        )
    {
    ivInv = aMilliSeconds;
    swtch();
    }

void Blink::swtch(
        )
    {
    digitalWrite( ivLed, (ivState) ? HIGH : LOW );
    ivState = !(ivState);
    }
 
SENG::SENG( 
        const char * const aFlow, 
        const int aVerbose
        )
        : ivFlow( aFlow )
        , ivVerbose( aVerbose )
    {
    Seq * s = Seq::instance();
    s->reg( *this );
    }

SENG::SENG( 
        const char * const aHost, 
        const char * const aURI, 
        const int aVerbose,
        const int aPort
        )
        : ivFlow( NULL )
        , ivPort( aPort )
        , ivVerbose( aVerbose )
        , ivHost( aHost )
        , ivURI( aURI )
    {
    Seq * s = Seq::instance();
    s->reg( *this );
    }

SENG::SENG( 
        const SENG & anObj 
        )
        : Seq::Task( anObj )
    {
    (void) anObj;
    }

SENG::~SENG(
        )
    {
    }

SENG & SENG::operator = ( 
        const SENG & anObj 
        )
    {
    if ( this != &anObj )
        {
        }
    return *this;
    }

void SENG::lp(
        )
    {
    static std::string _TMO( "tmo" );

    static SM * sm = SM::instance();

    while( sm->exe() );

    sm->tick();
    }

void SENG::stp(
        )
    {
    /*
    ** Load the state flow and then signal init.
    */
    Serial.printf( "SENG::stp(entered) - ivHost: %s, ivURI: %s, ivPort: %d, ivVerbose: %d, ivFlow: %s\r\n", 
            ivHost.c_str(), ivURI.c_str(), ivPort, ivVerbose, 
            (ivFlow != NULL) ? "TRUE" : "FALSE" );

    SM * sm = SM::instance();

    sm->setVerbose( ivVerbose );

    if ( ivFlow != NULL )
        {
        while ( ! sm->load( ivFlow ) )
            {
            delay( 10000 );
            }
        }
    else
        {
        while ( ! sm->loadHttp( ivHost.c_str(), ivURI.c_str(), ivPort ) )
            {
            Serial.printf( "ivHost: %s, ivURI: %s, ivPort: %d - loadHttp FAILED\r\n", 
                    ivHost.c_str(), ivURI.c_str(), ivPort );
            delay( 10000 );
            }
        }

    Serial.println( "SENG::stp(return)" );
    }

void SENG::setTimeout( 
        const unsigned int aTmo 
        )
    {
    static SM * sm = SM::instance();
    return sm->setTmo( aTmo );
    }

MyWiFi::MyWiFi( 
        const WiFiInfo_t * aWiFiSet
        )
        : ivInfo( aWiFiSet )
    {
    Seq * s = Seq::instance();
    s->reg( *this );
    }

MyWiFi::MyWiFi( 
        const MyWiFi & anObj 
        )
        : Seq::Task( anObj )
        , ivInfo( anObj.ivInfo )
    {
    (void) anObj;
    }

MyWiFi::~MyWiFi(
        )
    {
    }

MyWiFi & MyWiFi::operator = ( 
        const MyWiFi & anObj 
        )
    {
    if ( this != &anObj )
        {
        ivInfo = anObj.ivInfo;
        }

    return *this;
    }

void MyWiFi::lp(
        )
    {
    }

void MyWiFi::stp(
        )
    {
    int cnt;

    unsigned long eTime;
    unsigned long sTime = millis();

    for( ivCurrent = ivInfo; (ivCurrent != NULL) && (ivCurrent->ssid != NULL);
            ivCurrent ++ )
        {
        Serial.printf( "'%s', Attempt to attach.\r\n", ivCurrent->ssid );
        WiFi.begin( ivCurrent->ssid, ivCurrent->passwd );

        for( cnt = 0; (cnt < 100) && (WiFi.status() != WL_CONNECTED); cnt ++ )
            {
            Serial.print( "." );
            delay( 100 );
            }
        Serial.println( "" );

        if ( WiFi.status() == WL_CONNECTED )
            {
            eTime = millis();
            Serial.printf( "'%s', CONNECTED\r\n", ivCurrent->ssid );
            Serial.print( "LocalIP: " ); Serial.println( WiFi.localIP() );
            Serial.printf( "eplased time: %lu (millis)\r\n", eTime - sTime );
            return;
            }
        }

    ivCurrent = NULL;
    }

RTLIMIT::RTLIMIT( 
        const int aRTLimitSeconds
        , const int anInv
        )
        : ivRTLimit( aRTLimitSeconds )
        , ivInv( anInv )
        , ivTmo( 0 )
    {
    Seq * s = Seq::instance();
    s->reg( *this );
    }

RTLIMIT::RTLIMIT( 
        const RTLIMIT & anObj 
        )
        : Seq::Task( anObj )
        , ivRTLimit( anObj.ivRTLimit )
        , ivTmo( anObj.ivTmo )
    {
    (void) anObj;

    Seq * s = Seq::instance();
    s->reg( *this );
    }

RTLIMIT::~RTLIMIT(
        )
    {
    }

RTLIMIT & RTLIMIT::operator = ( 
        const RTLIMIT & anObj 
        )
    {
    if ( this != &anObj )
        {
        ivRTLimit = anObj.ivRTLimit;
        ivTmo = anObj.ivTmo;
        }

    return *this;
    }

void RTLIMIT::lp(
        )
    {
    unsigned long now = millis();

    if ( ivTmo == 0 )
        {
        return;
        }

    if ( now > ivRpt )
        {
        ivRpt = now + (ivInv * 1000);

        if ( ivTmo > now )
            {
            unsigned long dt = (ivTmo - now) / 1000;

            Serial.printf( "%s(%d) - time til restart: %lu (sec(s))\r\n",
                    __FILE__, __LINE__, dt );
            }
        }

    if ( now > ivTmo )
        {
        unsigned long dt = now - ivStTime;

        Serial.printf( "%s(%d) - restart - dt: %lu\r\n", __FILE__, __LINE__, dt );
        Serial.flush();
        ESP.restart();
        }
    }

void RTLIMIT::stp(
            )
    {
    ivStTime = millis();

    if ( ivRTLimit != 0 )
        {
        Serial.printf( "%s(%d) - ivRTLimit: %d\r\n", __FILE__, __LINE__, ivRTLimit );
        ivTmo = ivStTime + (ivRTLimit * 1000);
        }

    if ( ivInv != 0 )
        {
        Serial.printf( "%s(%d) - ivInv: %d\r\n", __FILE__, __LINE__, ivInv );
        ivRpt = ivStTime + (ivInv * 1000);
        }
    }
