#include <Arduino.h>

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
        const char * const aHost, 
        const char * const aURI, 
        const int aPort
        )
        : ivPort( aPort )
        , ivHost( aHost )
        , ivURI( aURI )
    {
    Seq * s = Seq::instance();
    s->reg( *this );
    }

SENG::SENG( 
        const SENG & anObj 
        )
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
    Serial.printf( "SENG::stp(entered) - ivHost: %s, ivURI: %s, ivPort: %d\r\n", 
            ivHost.c_str(), ivURI.c_str(), ivPort );

    SM * sm = SM::instance();

    while ( ! sm->loadHttp( ivHost.c_str(), ivURI.c_str(), ivPort ) )
        {
        Serial.printf( "ivHost: %s, ivURI: %s, ivPort: %d - loadHttp FAILED\r\n", 
                ivHost.c_str(), ivURI.c_str(), ivPort );
        delay( 10000 );
        }
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
        : ivInfo( anObj.ivInfo )
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
