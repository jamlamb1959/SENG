
#ifndef __Locker_h__
#define __Locker_h__

#ifndef NOARDUINO
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

class Locker
    {
  public:
    Locker( 
            SemaphoreHandle_t & aMut,
            const char * aFN = "HUH",
            const int aLine = 0
            )
            : ivMut( &aMut )
        {
Serial.printf( "%s(%d) %p(take), %p \r\n", aFN, aLine, this, ivMut );
        xSemaphoreTake( *ivMut, portMAX_DELAY );
Serial.printf( "%s(%d) %p(aquired)\r\n", aFN, aLine, this );
        }

    ~Locker( 
            )
        {
        xSemaphoreGive( *ivMut );
        }

  private:
    Locker( const Locker & anObj )
        {
        (void) anObj;
        }

    Locker & operator = ( const Locker & anObj )
        {
        (void) anObj;
        return *this;
        }

    SemaphoreHandle_t * ivMut;
    };
#else

#include <Mut.h>

class Locker
    {
  public:
    Locker( STENGINE::Mut & aMut, const char * aFN = "HUH", const int aLine = 0 )
            : ivMut( &aMut )
            , ivLineNo( aLine )
            , ivFile( aFN )
        {
        ivMut->lock( ivFile, ivLineNo );
        }

    ~Locker(
            )
        {
        ivMut->unlock();
        }

  private:
    STENGINE::Mut * ivMut;

    int ivLineNo;
    const char * ivFile;
    };

#endif

#endif
