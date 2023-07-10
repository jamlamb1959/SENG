// #define USE_PUBSUB

#include <Arduino.h>

#include <WiFi.h>

#include <iostream>
#include <vector>

#include <string.h>

#include "Fifo.h"
#include "TknDB.h"

// #include <driver/uart.h>

#include "freertos/FreeRTOS.h"

#include "Locker.h"

#include "Msg.h"

#include "SENG.h"

// void checkUpdate();
 
#ifndef TOUT
#define TOUT std::cout << "(" << __LINE__ << "): "
#endif
// #define SIGNAL( sig ) TOUT << "signal( '" << sig << "' )\n"; sm->signal( sig )
#define SIGNAL( sig ) return sm->signal( sig )

#include <PubSubClient.h>

static bool _macLoaded = false;
static char _mac[ 20 ];

#ifdef USE_PUBSUB
extern PubSubClient mqtt_g;
extern std::string topic_g;
#endif

// #include <semphr.h>

// extern SemaphoreHandle_t qMut_g;

QueueHandle_t SMPUB_g = NULL;

void skipWS(
        const char * & aWP
        )
    {
    char ch;

    for( ; (ch = *aWP) != '\0'; aWP ++ )
        {
        if ( strchr( " \t", ch ) == NULL )
            {
            return;
            }
        }

    return;
    }

void getTkn(
        std::string & aTkn,
        const char * & aWP,
        const char * const aSepList
        )
    {
    char ch;

    aTkn.clear();

    skipWS( aWP );

    for( ; (ch = *aWP) != '\0'; aWP ++ )
        {
        if ( strchr( aSepList, ch ) != NULL )
            {
            if ( aTkn.length() != 0 )
                {
                aWP ++;
                return;
                }
            }
        else
            {
            aTkn += ch;
            }
        }

    return;
    }

static void _expandVar( 
        std::string & anOStr, 
        const char * & anInStr 
        )
    {
    static TknDB * tdb = TknDB::instance();

    const char * ep;
    const char * wp;

    std::string tknName;
    std::string tknVal;

    wp = strchr( anInStr, '}' );

    if ( wp == NULL )
        {
        anOStr += "${";
        return;
        }

    ep = strchr( anInStr, ':' );

    if ( ep == NULL )
        {
        tknName.assign( anInStr, wp - anInStr );
        }
    else
        {
        tknName.assign( anInStr, ep - anInStr );
        }

    /*
    ** Move anInStr past the '}'
    */
    anInStr = wp + 1;

    tknVal = tdb->get( tknName );
    if ( tknVal.length() == 0 )
        {
        if ( ep != NULL )
            {
            tknVal.assign( ep + 2, wp - ep - 2 );
            }
        }

    anOStr += tknVal;
    }

static void _expand(
        std::string & anOStr,
        const char * & anIStr
        )
    {
    char ch;

    anOStr.clear();

    for( ; (ch = *anIStr) != '\0'; )
        {
        if ( ch == '$' )
            {
            if ( anIStr[ 1 ] == '{' )
                {
                anIStr += 2;

                _expandVar( anOStr, anIStr );

                continue;
                }
            else
                {
                anOStr += ch;
                anIStr ++;
                }
            }

        anOStr += ch;
        anIStr ++;
        }
    }

static void _loadStr(
        std::string & aStr,
        const char * & aWP
        )
    {
    char ch;

    aStr.clear();

    for( ; (ch = *aWP) != '\0'; aWP ++ )
        {
        if ( ch == '"' )
            {
            continue;
            }

        aStr += ch;
        }
    }

class ClearQs
        : public SP
    {
  public:
    ClearQs(
            )
        {
        }

    const char * className( 
            ) const
        {
        return "ClearQs";
        }

    void exec( 
            )
        {
        static SM * sm = SM::instance();
        static SQ * sq = SQ::instance();

//        Locker l( qMut_g, __FILE__, __LINE__  );
        sq->reset();

        SIGNAL( "ok" );
        }

    void load( 
            const char * & aWP 
            )
        {
        }

    void print(
            std::ostream & aStream
            ) const
        {
        aStream << "ClearQs";
        }

  private:
    };

MOM( ClearQs );

class ParseSMSUB
        : public SP
    {
  public:
    ParseSMSUB(
            )
        {
        }

    const char * className( 
            ) const
        {
        return "ParseSMSUB";
        }

    void exec( 
            )
        {
        static SM * sm = SM::instance();
        static TknDB * tdb = TknDB::instance();

        const char * wp;

        std::string cmd;
        std::string topic;
        std::string var = tdb->get( ivInVar );

        tdb->remove( ivInVar );

        if ( var.length() == 0 )
            {
            SIGNAL( "noValue" );
            }

        wp = var.c_str();
        getTkn( topic, wp, "," );

        _loadStr( cmd, wp );

        if ( cmd == "CHK" )
            {
            // checkUpdate();
            }
        else if ( cmd == "REBOOT" )
            {
            ESP.restart();
            }
        else if ( cmd == "TRACEON" )
            {
            int v = sm->getVerbose();
            v ++;
            sm->setVerbose( v );
            }
        else if ( cmd == "TRACEOFF" )
            {
            sm->setVerbose( 0 );
            }

        SIGNAL( "ok" );
        }

    void load( 
            const char * & aWP 
            )
        {
        getTkn( ivInVar, aWP, " \r\n\t" );
        getTkn( ivOutVar, aWP, " \r\n\t" );
        }

    void print(
            std::ostream & aStream
            ) const
        {
        aStream << "ParseSMSUB " << ivInVar << " " << ivOutVar;
        }

  private:
    std::string ivInVar;
    std::string ivOutVar;
    };

MOM( ParseSMSUB );

#ifdef LOCAL_SP
class Send
        : public SP
    {
  public:
    Send(
            )
        {
        }

    const char * className( 
            ) const
        {
        return "Send";
        }

    void exec( 
            )
        {
        static SM * sm = SM::instance();
        static TknDB * tdb = TknDB::instance();

        std::string msg;

        time_t now;

        const char * wp = ivMsg.c_str();

        _expand( msg, wp );
        
        if ( sm->getVerbose() )
            {
            std::cout << "(" << __LINE__ << ")S: " << msg << std::endl;
            }

#ifdef USE_PUBSUB
        if ( mqtt_g.connected() )
            {
            std::string m;

            m = "S: '";
            m += msg;
            m += "'";

            mqtt_g.publish( topic_g.c_str(), m.c_str() );
            }
#endif

        Serial2.print( msg.c_str() ); Serial2.print( "\r\n" );

        // (void) uart_write_bytes( UART_NUM_2, msg.c_str(), msg.length() );
        // (void) uart_write_bytes( UART_NUM_2, "\r\n", 2 );
        
        if ( ivTmo >= 0 )
            {
            sm->setInv( ivTmo );
            }
        }

    void load( 
            const char * & aWP 
            )
        {
        getTkn( ivMsg, aWP, " \r\n\t" );

        skipWS( aWP );

        ivTmo = atoi( aWP );
        }

    void print(
            std::ostream & aStream
            ) const
        {
        aStream << "Send " << ivMsg << " " << ivTmo;
        }

  private:
    std::string ivMsg;

    int ivTmo;
    };

MOM( Send );
#endif

class ClearCaptureStack
        : public SP
    {
  public:
    ClearCaptureStack(
            )
        {
        }

    const char * className( 
            ) const
        {
        return "ClearCaptureStack";
        }

    void exec( 
            )
        {
        static SM * sm = SM::instance();
        static CS * cs = CS::instance();

        cs->reset();

        SIGNAL( "ok" );
        }

    void load( 
            const char * & aWP 
            )
        {
        }

    void print(
            std::ostream & aStream
            ) const
        {
        aStream << "ClearCaptureStack";
        }

  private:
    };

MOM( ClearCaptureStack );

class EvalCNAct
        : public SP
    {
  public:
    EvalCNAct(
            )
        {
        }

    const char * className( 
            ) const
        {
        return "EvalCNAct";
        }

    void exec( 
            )
        {
        static SM * sm = SM::instance();
        static TknDB * tdb = TknDB::instance();

        const char * wp;

        std::string cnact = tdb->get( "CNACT" );

        wp = cnact.c_str();

        wp = strchr( wp, ',' );

        if ( wp != NULL )
            {
            std::string sig( cnact.c_str(), (wp - cnact.c_str()) );

            SIGNAL( sig.c_str() );
            }
        }

    void load( 
            const char * & aWP 
            )
        {
        }

    void print(
            std::ostream & aStream
            ) const
        {
        aStream << "EvalCNAct";
        }

  private:
    };

MOM( EvalCNAct );

class EvalGPS
        : public SP
    {
  public:
    EvalGPS(
            )
        {
        }

    const char * className( 
            ) const
        {
        return "EvalGPS";
        }

    void exec( 
            )
        {
        static SM * sm = SM::instance();
        static TknDB * tdb = TknDB::instance();

        TOUT << "tdb:\n" << *tdb << "\n";

/*
        if ( gpsPwr_g == 0 )
            {
            SIGNAL( "powerOff" );
            return;
            }

        switch( gpsFix_g )
            {
            case 0:
                SIGNAL( "notFixed" );
                break;
                
            case 1:
                SIGNAL( "fixed" );
                break;
            }
*/
        }

    void load( 
            const char * & aWP 
            )
        {
        }

    void print(
            std::ostream & aStream
            ) const
        {
        aStream << "EvalGPS";
        }

  private:
    };

MOM( EvalGPS );

class EvalRegState
        : public SP
    {
  public:
    EvalRegState(
            )
        {
        }

    const char * className( 
            ) const
        {
        return "EvalRegState";
        }

    void exec( 
            )
        {
        static SM * sm = SM::instance();
        static TknDB * tdb = TknDB::instance();

        const char * wp;

        std::string creg = tdb->get( "CREG" );
        
        wp = creg.c_str();

        wp = strchr( wp, ',' );
        if ( wp != NULL );
            {
            wp ++;
            if ( *wp != '\0' )
                {
                SIGNAL( wp );
                }
            }
            
// TOUT << "\n" << *tdb << "\n";
        }

    void load( 
            const char * & aWP 
            )
        {
        }

    void print(
            std::ostream & aStream
            ) const
        {
        aStream << "EvalRegState";
        }

  private:
    };

MOM( EvalRegState );

class EvalSMState
        : public SP
    {
  public:
    EvalSMState(
            )
        {
        }

    const char * className( 
            ) const
        {
        return "EvalSMState";
        }

    void exec( 
            )
        {
        static SM * sm = SM::instance();
        static TknDB * tdb = TknDB::instance();

TOUT << "\n" << *tdb << "\n";

/*
        switch( smState_g )
            {
            case 0:
                SIGNAL( "offline" );
                break;
                
            case 1:
                SIGNAL( "online" );
                break;
            }
*/

        }

    void load( 
            const char * & aWP 
            )
        {
        }

    void print(
            std::ostream & aStream
            ) const
        {
        aStream << "EvalSMState";
        }

  private:
    };

MOM( EvalSMState );

class Noop
        : public SP
    {
  public:
    Noop(
            )
        {
        }

    const char * className( 
            ) const
        {
        return "Noop";
        }

    void exec( 
            )
        {
        TOUT << "Noop::exec\n";
        }

    void load( 
            const char * & aWP 
            )
        {
        }

    void print(
            std::ostream & aStream
            ) const
        {
        aStream << "Noop";
        }

  private:
    };

MOM( Noop );

class SaveIMEI
        : public SP
    {
  public:
    SaveIMEI(
            )
        {
        }

    const char * className( 
            ) const
        {
        return "SaveIMEI";
        }

    void exec( 
            )
        {
        static CS * cs = CS::instance();
        static SM * sm = SM::instance();
        static TknDB * tdb = TknDB::instance();

        std::string imei;

        imei = cs->pop();

        tdb->put( std::string( "IMEI" ), imei );

        SIGNAL( "ok" );
        }

    void load( 
            const char * & aWP 
            )
        {
        }

    void print(
            std::ostream & aStream
            ) const
        {
        aStream << "SaveIMEI";
        }

  private:
    };

MOM( SaveIMEI );

class SaveIMI
        : public SP
    {
  public:
    SaveIMI(
            )
        {
        }

    const char * className( 
            ) const
        {
        return "SaveIMI";
        }

    void exec( 
            )
        {
        static CS * cs = CS::instance();
        static SM * sm = SM::instance();
        static TknDB * tdb = TknDB::instance();

        std::string imi;

        imi = cs->pop();
        tdb->put( std::string( "IMI" ), imi );

        SIGNAL( "ok" );
        }

    void load( 
            const char * & aWP 
            )
        {
        }

    void print(
            std::ostream & aStream
            ) const
        {
        aStream << "SaveIMI";
        }

  private:
    };

MOM( SaveIMI );

class SaveIMSI
        : public SP
    {
  public:
    SaveIMSI(
            )
        {
        }

    const char * className( 
            ) const
        {
        return "SaveIMSI";
        }

    void exec( 
            )
        {
        static TknDB * tdb = TknDB::instance();
        static CS * cs = CS::instance();
        static SM * sm = SM::instance();

        std::string imsi;

        imsi = cs->pop();
        tdb->put( std::string( "IMSI" ), imsi );

        SIGNAL( "ok" );
        }

    void load( 
            const char * & aWP 
            )
        {
        }

    void print(
            std::ostream & aStream
            ) const
        {
        aStream << "SaveIMSI";
        }

  private:
    };

MOM( SaveIMSI );

class Tmo
        : public SP
    {
  public:
    Tmo(
            )
            : ivTmo( 1 )
        {
        }

    const char * className( 
            ) const
        {
        return "Tmo";
        }

    void exec( 
            )
        {
        static SM * sm = SM::instance();

        if ( ivTmo >= 0 )
            {
            sm->setInv( ivTmo );
            }
        }

    void load( 
            const char * & aWP 
            )
        {
        skipWS( aWP );

        /*
        ** if the parser is at the end of line then just default
        ** the ivTmo Value.
        */
        if ( strchr( "\r\n", *aWP ) == NULL )
            {
            ivTmo = atoi( aWP );
            }
        }

    void print(
            std::ostream & aStream
            ) const
        {
        aStream << "Tmo " << ivTmo;
        }

  private:
    int ivTmo;
    };

MOM( Tmo );

#ifdef __LOCAL_SP

class SMPUB
        : public SP
    {
  public:
    SMPUB(
            )
            : ivTmo( -1 )
        {
        }

    const char * className( 
            ) const
        {
        return "SMPUB";
        }

    void exec( 
            )
        {
        static TknDB * tdb = TknDB::instance();
        
        static int emptyCnt = 0;
        static size_t bufHW = 0;

        static SM * sm = SM::instance();
        static SQ * sq = SQ::instance();

        std::string imsi;

        bool haveMsg;

        char buf[ 100 ];

        size_t ln;

        Msg * m = NULL;

        TickType_t tmo = ivTmo * 1000;

        haveMsg = xQueueReceive( SMPUB_g, &m, tmo );
    
        if ( haveMsg )
            {
            emptyCnt = 0;

            if ( m != NULL )
                {
                switch( m->ivMT )
                    {
                    case Msg::t_pub:
                        imsi = tdb->get( std::string( "IMSI" ) );

                        sq->push( m->ivPayload );

                        sprintf( buf, "AT+SMPUB=\"/SP/RPT/%s\",%u,0,0", imsi.c_str(), m->ivPayload.length() );
                        if ( sm->getVerbose() > 1 )
                            {
                            TOUT << "(SMPUB) S: '" << buf << "'\n";
                            }

                        ln = strlen( buf );

                        Serial2.printf( "%*.*s\r\n", (int) ln, (int) ln, buf );

                        // (void) uart_write_bytes( UART_NUM_2, buf, ln );
                        // (void) uart_write_bytes( UART_NUM_2, "\r\n", 2 );
            
                        delete m;
                        break;

                    case Msg::t_smpub:
                        SIGNAL( "smsub" );
                    }
                }
            }
        else
            {
            SIGNAL( "tmo" );
            }
        }

    void load( 
            const char * & aWP 
            )
        {
        if ( SMPUB_g == NULL )
            {
            SMPUB_g = xQueueCreate( 30, sizeof( Msg * ) );
            assert( SMPUB_g != NULL );
            }

        skipWS( aWP );

        if ( strchr( "\r\n", *aWP ) == NULL )
            {
            ivTmo = atoi( aWP );
            }
        }

    void print(
            std::ostream & aStream
            ) const
        {
        aStream << "SMPUB " << ivTmo;
        }

  private:
    int ivTmo;
    };

MOM( SMPUB );
#endif

SQ * SQ::instance(
        )
    {
    static SQ * inst = new SQ();

    assert( inst != NULL );

    return inst;
    }

SQ::SQ(
        )
    {
    }

class Cnt
        : public SP
    {
  public:
    Cnt(
            )
            : ivCnt( 0 )
            , ivLimit( 0 )
        {
        }

    const char * className( 
            ) const
        {
        return "Cnt";
        }

    void exec( 
            )
        {
        static SM * sm = SM::instance();

        ivCnt ++;

        if ( ivCnt < ivLimit )
            {
            SIGNAL( "less" );
            }
        else if ( ivCnt == ivLimit )
            {
            SIGNAL( "equal" );
            }
        else
            {
            SIGNAL( "greater" );
            }
        }

    void load( 
            const char * & aWP 
            )
        {
        skipWS( aWP );

        /*
        ** if the parser is at the end of line then just default
        ** the ivCnt Value.
        */
        if ( strchr( "\r\n", *aWP ) == NULL )
            {
            ivLimit = atoi( aWP );
            }

        ivCnt = 0;
        }

    void print(
            std::ostream & aStream
            ) const
        {
        aStream << "Cnt " << ivLimit;
        }

  private:
    int ivCnt;
    int ivLimit;
    };

MOM( Cnt );

class DumpTDB
        : public SP
    {
  public:
    DumpTDB(
            )
        {
        }

    const char * className( 
            ) const
        {
        return "DumpTDB";
        }

    void exec( 
            )
        {
        static SM * sm = SM::instance();
        static TknDB * tdb = TknDB::instance();

        TOUT << "DumpTDB - tdb:\n" << *tdb << "\n\n";

        SIGNAL( "ok" );
        }

    void load( 
            const char * & aWP 
            )
        {
        }

    void print(
            std::ostream & aStream
            ) const
        {
        aStream << "DumpTDB";
        }

  private:
    };

MOM( DumpTDB );

class SigTkn
        : public SP
    {
  public:
    SigTkn(
            )
        {
        }

    const char * className( 
            ) const
        {
        return "SigTkn";
        }

    void exec( 
            )
        {
        static SM * sm = SM::instance();
        static TknDB * tdb = TknDB::instance();

        std::string sig = tdb->get( ivTknName );

        SIGNAL( sig );
        }

    void load( 
            const char * & aWP 
            )
        {
        skipWS( aWP );

        ivTknName = aWP;
        }

    void print(
            std::ostream & aStream
            ) const
        {
        aStream << "SigTkn " << ivTknName;
        }

  private:
    std::string ivTknName;
    };

MOM( SigTkn );

static void _csv(
        const char * const aTknName,
        const char * & aWP
        )
    {
    static TknDB * tdb = TknDB::instance();

    std::string v;

    getTkn( v, aWP, "," );
    if ( v.length() != 0 )
        {
        tdb->put( aTknName, v );
        }
    }

class ParseNMEA
        : public SP
    {
  public:
    ParseNMEA(
            )
        {
        }

    const char * className( 
            ) const
        {
        return "ParseNMEA";
        }

    void exec( 
            )
        {
        static SM * sm = SM::instance();
        static TknDB * tdb = TknDB::instance();

        const char * wp;

        std::string nmea = tdb->get( "CGNSINF" );

        wp = nmea.c_str();

        _csv( "GPSPWR", wp );
        _csv( "GPSFIX", wp );
        _csv( "GPSDT", wp );
        _csv( "LAT", wp );
        _csv( "LNG", wp );
        _csv( "ALT", wp );
        _csv( "SPDOG", wp );
        _csv( "COURSEOG", wp );
        _csv( "FIXMODE", wp );

        SIGNAL( "ok" );
        }

    void load( 
            const char * & aWP 
            )
        {
        }

    void print(
            std::ostream & aStream
            ) const
        {
        aStream << "ParseNMEA";
        }

  private:
    };

MOM( ParseNMEA );

class DumpTokens
        : public SP
    {
  public:
    DumpTokens(
            )
        {
        }

    const char * className( 
            ) const
        {
        return "DumpTokens";
        }

    void exec( 
            )
        {
        static SM * sm = SM::instance();
        static TknDB * tdb = TknDB::instance();

	    tdb->display( std::cout );

        SIGNAL( "ok" );
        }

    void load( 
            const char * & aWP 
            )
        {
        }

    void print(
            std::ostream & aStream
            ) const
        {
        aStream << "DumpTokens";
        }

  private:
    };

MOM( DumpTokens );

std::string _escapeQuotes(
        const std::string & aStr
        )
    {
    const char * wp;

    std::string ret;

    for( wp = aStr.c_str(); *wp != '\0'; wp ++ )
        {
        if ( *wp == '"' )
            {
            ret += '\'';
            continue;
            }

        ret += *wp;
        }

    return ret;
    }

static void _loadMac(
        )
    {
    uint8_t rm[ 6 ];
    
    WiFi.macAddress( rm );
    sprintf( _mac, "%02X:%02X:%02X:%02X:%02X:%02X",
            rm[ 0 ], rm[ 1 ], rm[ 2 ],  
            rm[ 3 ], rm[ 4 ], rm[ 5 ] );
    }

class LogTokens
        : public SP
    {
  public:
    LogTokens(
            )
        {
        if ( !_macLoaded )
            {
            _macLoaded = true;
            _loadMac();
            }
        }

    const char * className( 
            ) const
        {
        return "LogTokens";
        }

    void exec( 
            )
        {
        size_t idx;

        static SM * sm = SM::instance();
        static TknDB * tdb = TknDB::instance();

        Msg * m = new Msg( Msg::t_pub );

        m->ivPayload = "{\"MAC\":\"";
        m->ivPayload += _mac;
        m->ivPayload += "\"";

        for( idx = 0; idx < ivTkn.size(); idx ++ )
            {
            m->ivPayload += ',';
            m->ivPayload += "\"";
            m->ivPayload += ivTkn[ idx ];
            m->ivPayload += "\":\"";
            m->ivPayload += _escapeQuotes( tdb->get( ivTkn[ idx ] ) );
            m->ivPayload += "\"";
            }

        m->ivPayload += "}";
        
        xQueueGenericSend( SMPUB_g, &m, 1000, queueSEND_TO_BACK );

        SIGNAL( "ok" );
        TOUT << "\nivPayload: " << m->ivPayload << std::endl;
        }

    void load( 
            const char * & aWP 
            )
        {
        std::string tkn;

	    for ( ; ; )
            {
            getTkn( tkn, aWP, " ,\t" );

            if ( tkn.length() == 0 )
                {
                break;
                }

            ivTkn.push_back( tkn );
            }
        }

    void print(
            std::ostream & aStream
            ) const
        {
        aStream << "LogTokens";
        }

  private:
    std::vector< std::string > ivTkn;
    };

MOM( LogTokens );

static void _getTkn(
        std::string & aRet,
        const char * const aSepList,
        const char * & aWP
        )
    {
    aRet.clear();

    for( ; *aWP != '\0'; aWP ++ )
        {
        if ( strchr( aSepList, *aWP ) != NULL )
            {
            aWP ++;
            return;
            }

        aRet += *aWP;
        }
    }

Filo< std::string > operList_g;

class QueueOperators
        : public SP
    {
  public:
    QueueOperators(
            )
        {
        }

    const char * className( 
            ) const
        {
        return "QueueOperators";
        }

    void exec( 
            )
        {
        const char * iwp;
        const char * wp;

        size_t idx;

        std::string ent;
        std::string longAlpha;
        std::string operId;
        std::string shortAlpha;
        std::string stat;
        std::string v;

        static SM * sm = SM::instance();
        static TknDB * tdb = TknDB::instance();

        if ( operList_g.depth() != 0 )
            {
            SIGNAL( "alreadyLoaded" );
            return;
            }

        v = tdb->get( "COPS" );
        TOUT << "\nv: " << v << "\n\n";

        for( wp = v.c_str(); *wp != '\0'; wp ++ )
            {
            if ( *wp == ',' )
                {
                break;
                }

            if ( *wp != '(' )
                {
                continue;
                }

            wp ++;

            _getTkn( ent, ")", wp );

            if ( ent.length() == 0 )
                {
                break;
                }

            iwp = ent.c_str();
            _getTkn( stat, ",", iwp );
            int st = atoi( stat.c_str() );

            if ( (st != 1) && (st != 2) )
                {
                TOUT << "ent: " << ent << " (discard(state))\n";
                continue;
                }

            _getTkn( longAlpha, ",", iwp );
            if ( (strcmp( longAlpha.c_str(), "\"Verizon\"" ) != 0) && 
                    (strcmp( longAlpha.c_str(), "\"AT&T\"" ) != 0) 
                    )
                {
                TOUT << "ent: " << ent << ", longAlpha: '" << longAlpha << "',  (discard(Name))\n";
                continue;
                }

            _getTkn( shortAlpha, ",", iwp );
            _getTkn( operId, ",", iwp );

            operList_g.push( operId );
            }

        SIGNAL( "ok" );
        }

    void load( 
            const char * & aWP 
            )
        {
        }

    void print(
            std::ostream & aStream
            ) const
        {
        aStream << "QueueOperators";
        }

  private:
    };

MOM( QueueOperators );

#ifdef __LOCAL_SP

class SetNetwork
        : public SP
    {
  public:
    SetNetwork(
            )
        {
        }

    const char * className( 
            ) const
        {
        return "SetNetwork";
        }

    void exec( 
            )
        {
        static SM * sm = SM::instance();

        std::string msg;

        if ( operList_g.depth() == 0 )
            {
            SIGNAL( "empty" );
            return;
            }

        msg = "AT+COPS=4,2,";
        msg += operList_g.pop();

#ifdef USE_PUBSUB
        if ( mqtt_g.connected() )
            {
            std::string m;

            m = "S: '";
            m += msg;
            m += "'";

            mqtt_g.publish( topic_g.c_str(), m.c_str() );
            }
        else
#endif
            {
            std::cout << "S: '" << msg << "'" << std::endl;
            }

        Serial2.print( msg.c_str() ); Serial2.print( "\r\n" );
        // (void) uart_write_bytes( UART_NUM_2, msg.c_str(), msg.length() );
        // (void) uart_write_bytes( UART_NUM_2, "\r\n", 2 );

        if ( ivTmo >= 0 )
            {
            sm->setInv( ivTmo );
            }
        }

    void load( 
            const char * & aWP 
            )
        {
        skipWS( aWP );

        ivTmo = atoi( aWP );
        }

    void print(
            std::ostream & aStream
            ) const
        {
        aStream << "SetNetwork " << ivTmo;
        }

  private:
    int ivTmo;
    };

MOM( SetNetwork );
#endif

class Branch
        : public SP
    {
  public:
    Branch(
            )
        {
        }

    const char * className( 
            ) const
        {
        return "Branch";
        }

    void exec( 
            )
        {
        static SM * sm = SM::instance();
        static TknDB * tdb = TknDB::instance();

        std::string sig = tdb->get( ivVar );

        if ( sig.length() == 0 )
            {
            SIGNAL( "noValue" );
            }

        SIGNAL( sig );
        }

    void load( 
            const char * & aWP 
            )
        {
        getTkn( ivVar, aWP, " \r\n\t" );

        assert( *aWP == '\0' );
        }

    void print(
            std::ostream & aStream
            ) const
        {
        aStream << "Branch " << ivVar;
        }

  private:
    std::string ivVar;
    };

MOM( Branch );

class ParseCSV
        : public SP
    {
  public:
    ParseCSV(
            )
        {
        }

    const char * className( 
            ) const
        {
        return "ParseCSV";
        }

    void exec( 
            )
        {
        static SM * sm = SM::instance();
        static TknDB * tdb = TknDB::instance();

        char tknName[ 100 ];

        int cnt;

        std::string csv = tdb->get( ivInVar );
        std::string tkn;

        const char * wp = csv.c_str();

        for( cnt = 0; *wp != '\0'; cnt ++ )
            {
            getTkn( tkn, wp, "," );

            if ( tkn.length() != 0 )
                {
                sprintf( tknName, "%s-%d", ivOutVarPrefix.c_str(), cnt );
                
                tdb->put( std::string( tknName ), tkn );
                }
            }

        SIGNAL( "ok" );
        }

    void load( 
            const char * & aWP 
            )
        {
        getTkn( ivInVar, aWP, " \r\n\t" );
        getTkn( ivOutVarPrefix, aWP, " \r\n\t" );

        assert( *aWP == '\0' );
        }

    void print(
            std::ostream & aStream
            ) const
        {
        aStream << "ParseCSV " << ivInVar << " " << ivOutVarPrefix;
        }

  private:
    std::string ivInVar;
    std::string ivOutVarPrefix;
    };

MOM( ParseCSV );

class Set
        : public SP
    {
  public:
    Set(
            )
        {
        }

    const char * className( 
            ) const
        {
        return "Set";
        }

    void exec( 
            )
        {
        static SM * sm = SM::instance();
        static TknDB * tdb = TknDB::instance();

        tdb->put( ivVar, ivVal );

        SIGNAL( "ok" );
        }

    void load( 
            const char * & aWP 
            )
        {
        getTkn( ivVar, aWP, " \r\n\t" );
        ivVal = aWP;
        }

    void print(
            std::ostream & aStream
            ) const
        {
        aStream << "Set " << ivVar << " " << ivVal;
        }

  private:
    std::string ivVar;
    std::string ivVal;
    };

MOM( Set );

class Interval
        : public SP
    {
  public:
    Interval(
            )
            : ivNxt( 0 )
        {
        }

    const char * className( 
            ) const
        {
        return "Interval";
        }

    void exec( 
            )
        {
        static SM * sm = SM::instance();

        unsigned long now = millis();

        if ( now > ivNxt )
            {
            ivNxt = millis() + (ivInterval * 1000);

            SIGNAL( "expired" );
            }

        SIGNAL( "ok" );
        }

    void load( 
            const char * & aWP 
            )
        {
        getTkn( ivIntervalStr, aWP, " \r\n\t" );

        ivInterval = atoi( ivIntervalStr.c_str() );
        }

    void print(
            std::ostream & aStream
            ) const
        {
        aStream << "Interval " << ivIntervalStr;
        }

  private:
    unsigned long ivNxt;

    int ivInterval;

    std::string ivIntervalStr;
    };

MOM( Interval );
