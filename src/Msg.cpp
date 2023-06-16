
#include "Msg.h"

Msg::Msg( 
        const MsgType_t aMT
        )
        : ivMT( aMT )
    {
    }

std::ostream & operator << ( 
        std::ostream & aStream, 
        const Msg & anObj 
        )
    {
    switch( anObj.ivMT )
        {
        case Msg::t_null:
            aStream << "t_null";
            break;

        case Msg::t_pub:
            aStream << "t_pub: '" << anObj.ivPayload << "'";
            break;

        default:
            aStream << "ivMT(" << (int) anObj.ivMT << ")";
            break;
        }

    return aStream;
    }

