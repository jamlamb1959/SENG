#ifndef __util_h__
#define __util_h__

void delay( const unsigned long int aNumMillis );

void pinMode( const int aGPIO, const int aDirecton );

/*
** Direction values
*/
#define INPUT   0
#define OUTPUT  1

void digitalWrite( const int aGPIO, const int aLevel );

/*
** Level
*/
#define LOW     0
#define HIGH    1

unsigned long int millis();

#endif
