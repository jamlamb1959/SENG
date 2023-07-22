#ifndef __SENG_SERIAL
#define __SENG_SERIAL

#include <iostream>

#include <assert.h>

#include <String.h>

class Console
    {
  public:
    static Console * instance(
            )
        {
        static Console * _inst = new Console();
    
        assert( _inst != NULL );

        return _inst;
        }

    void flush(
            )
        {
        }

    void print( 
            const char * const aStr
            )
        {
        std::cout << aStr;
        }

    void printf( 
            const char * const aFrmt,
            ...
            )
        {
        (void) aFrmt;

        assert( false );
        }

    void println(
            const Printable & anObj
            )
        {
        assert( false );
        (void) anObj;
        }

    void println( 
            const char * aStr 
            )
        {
        std::cout << aStr << "\n";
        }

  private:
    Console(
            )
        {
        }
    
    Console(
            const Console & anObj
            )
        {
        (void) anObj;
        }
    
    ~Console(
            )
        {
        }

    Console & operator = (
            const Console & anObj
            )
        {
        if ( this != &anObj )
            {
            assert( false );
            }

        return *this;
        }

    };

#define Serial (*Console::instance())

#endif

