#ifdef __TESTING

#include <Arduino.h>

#include <SeqLib.h>

static WiFiInfo_t _wifiInfo[] =
    {
    { "s1616", "4026892842" }
    , { NULL, NULL }
    };

static MyWiFi _wifi( _wifiInfo );
static Blink _blink;
static SENG _seng( "repo.sheepshed.tk", "/StateFlow/test.sf" );
static RTLIMIT _rtlimit;

void setup()
    {
    Seq * s = Seq::instance();

    Serial.begin( 115200 );
    delay( 4000 );

    Serial.printf( "setup(entered) %s(%d) - %s %s\r\n", 
            __FILE__, __LINE__, __DATE__, __TIME__ );

    s->stp();
    }

static bool _ledState = false;
void loop()
    {
    static Seq * _s = Seq::instance();

    _s->lp();
    }
#endif

