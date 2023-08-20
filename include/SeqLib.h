#ifndef __SeqLib_h__
#define __SeqLib_h__

#include <Seq.h>

class Blink
        : public Seq::Task
    {
  public:
    Blink( const int aLed = 2 );
    Blink( const Blink & anObj );
    ~Blink();

    Blink & operator = ( const Blink & anObj );

    void lp();
    void stp();

    void setInv( int aMilliSeconds = 500 );
    void swtch();

  private:
    bool ivState;

    int ivInv;
    int ivLed;

    unsigned long int ivNxtTime;
    };

class RTLIMIT
        : public Seq::Task
    {
  public:
    RTLIMIT( const int aRTLimitSeconds = 3600, const int anInv = 60 );
    RTLIMIT( const RTLIMIT & anObj );
    ~RTLIMIT();

    RTLIMIT & operator = ( const RTLIMIT & anObj );

    void lp();
    void stp();

  private:
    int ivRTLimit;
    int ivInv;

    unsigned long ivStTime;
    unsigned long ivRpt;
    unsigned long ivTmo;
    };

class SENG
        : public Seq::Task
    {
  public:
    SENG( const char * const aFlow, const int aVerbose );
    SENG( const char * const aHost, const char * const aURI, const int aVerbose = 0, const int aPort = 80 );
    SENG( const SENG & anObj );
    ~SENG();

    SENG & operator = ( const SENG & anObj );

    void lp();
    void stp();

    /*
    ** if set to 0 the time out is disabled.
    */
    unsigned long getTimeout() const;
    void setTimeout( const unsigned int aTmo = 0 );

  private:
    const char * ivFlow;

    int ivPort;
    int ivVerbose;

    std::string ivHost;
    std::string ivURI;

    unsigned long ivTmo;
    };

typedef struct
    {
    const char * const ssid;
    const char * const passwd;
    } WiFiInfo_t;

class MyWiFi
        : public Seq::Task
    {
  public:
    /*
    ** Pass in an array of WiFiInfo_t structures.  
    **
    ** the last entry has ssid == NULL
    */
    MyWiFi( const WiFiInfo_t * aWiFiSet = NULL );
    MyWiFi( const MyWiFi & anObj );
    ~MyWiFi();

    MyWiFi & operator = ( const MyWiFi & anObj );

    void lp();
    void stp();

  private:
    const WiFiInfo_t * ivCurrent;
    const WiFiInfo_t * ivInfo;
    };

#endif
