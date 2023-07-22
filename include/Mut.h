/*
 * Mut.h
 */

#ifndef __SENG_Mut_h__
#define __SENG_Mut_h__

#ifdef NOARDUINO
#include <pthread.h>
#endif

/*
** Class Defines
*/

namespace STENGINE
    {

/**
 *     Mut
 *
 * This is an object that represents a mutex.  The object will initialize
 * the mutex as unlocked when it is created.  The lock and unlock methods
 * can be used to control access.
 *
 * The mutex will be created recursive.   The class does not support the
 * the ability to copy and perform equal operations.  But it is much
 * simpler then the Mutex implementation.
 */

class Mut 
    {
  public:
    /**
     * Method for constructing the mutex.
     */
    Mut();

    /**
     ** Copy constructor. Does nothing.
     */
    Mut( const Mut & anObj );

    /**
     * Private destructor to match the constructor.
     */
    ~Mut();

    /**
     ** This method will attempt to lock the mutex.
     ** If the lock attempt is not successful the method will assert.
     */
    virtual void lock( const char * aFile = "HUH", 
            const int aLineNo = 0, const int aTmo = -1 );

    /**
     ** This method will attempt to unlock the mutex.
     ** If the unlock attempt is not successful the method will assert.
     */
    virtual void unlock();

    /**
     ** Rhe equal operator will detach the existing mutex and then attach
     ** to the mutex of the operator that was passed in.
     */
    Mut & operator = ( const Mut & anObj );

  private:
    /*
    ** The pthread mutext block.
    */
    pthread_mutex_t ivMut;
    };

    } /* end namespace */
#endif
