#include <Arduino.h>

#include <assert.h>

#include <Seq.h>

Seq * Seq::instance(
        )
    {
    static Seq * _inst = new Seq();

    return _inst;
    }

Seq::Task::Task(
        )
    {
    ivNxt = ivPrev = this;
    }

Seq::Task::Task( 
        const Task & anObj 
        )
    {
    ivNxt = ivPrev = this;

    (void) anObj;
    }
    
Seq::Task & Seq::Task::operator = ( 
        const Task & anObj 
        )
    {
    if ( this != &anObj )
        {
        /*
        ** need to store the object content.
        */
        }
    return *this;
    }

Seq::Task::~Task(
        )
    {
    unlink();
    }

void Seq::Task::link( 
        Task & aNxt 
        )
    {
    if ( aNxt.ivNxt != &aNxt )
        {
        aNxt.unlink();
        }

    aNxt.ivNxt = this;
    aNxt.ivPrev = ivPrev;
    
    ivPrev->ivNxt = &aNxt;
    ivPrev = &aNxt;
    }
    
Seq::Task * Seq::Task::next(
        )
    {
    return ivNxt;
    }

void Seq::Task::unlink(
        )
    {
    if ( ivNxt == this )
        {
        return;
        }

    ivNxt->ivPrev = ivPrev;
    ivPrev->ivNxt = ivNxt;

    ivNxt = ivPrev = this;
    }

void Seq::Task::lp(
        )
    {
    Serial.printf( "Seq::Task::lp(Not overloaded)\r\n" );
    }

void Seq::Task::stp(
        )
    {
    Serial.printf( "Seq::Task::stp(Not overloaded)\r\n" );
    }

void Seq::lp(
        )
    {
    Task * wp;

    for( wp = ivHead.next(); wp != &ivHead; wp = wp->next() )
        {
        wp->lp();
        }
    }

void Seq::reg( 
        Task & anObj 
        )
    {
    ivHead.link( anObj );
    }

void Seq::stp(
        )
    {
    Task * wp;

    for( wp = ivHead.next(); wp != &ivHead; wp = wp->next() )
        {
        wp->stp();
        }
    }

void Seq::unreg( 
        Task & anObj 
        )
    {
    anObj.unlink();
    }

Seq::Seq(
        )
    {
    }

Seq::Seq( 
        const Seq & anObj 
        )
    {
    (void) anObj;
    }

Seq & Seq::operator = ( 
        const Seq & anObj 
        )
    {
    if ( this != &anObj )
        {
        assert( false );
        }

    return *this;
    }
    
Seq::~Seq(
        )
    {
    Task * nxt;
    Task * wp;

    for( wp = ivHead.next(); wp != &ivHead; )
        {
        nxt = wp->next();
        wp->unlink();
        wp = nxt;
        }
    }

