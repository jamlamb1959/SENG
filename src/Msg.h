
#ifndef __Msg_h__
#define __Msg_h__

#include <iostream>
#include <string>


class Msg
    {
  public:
    typedef enum
        {
        t_null,
        t_pub,
        t_smpub
        } MsgType_t;

    Msg( const MsgType_t aMT = t_null );

    MsgType_t ivMT;

    std::string ivPayload;
    };

std::ostream & operator << ( std::ostream & aStream, const Msg & anObj );

#endif
