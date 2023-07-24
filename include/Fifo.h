#ifndef __Fifo_h__
#define __Fifo_h__

#include <iostream>
#include <Locker.h>

#define defCapacity_l   10

#ifndef TOUT
#define TOUT std::cout << __FILE__ << "(" << __LINE__ << "): "
#endif

template < class Item >
class Fifo
    {
  public:
    Fifo(
            const size_t aCapacity = defCapacity_l
            )
            : ivCapacity( aCapacity )
            , ivInIdx( 0 )
            , ivOutIdx( 0 )
        {
        ivQ = new Item[ ivCapacity ];
        }

    ~Fifo(
            )
        {
        delete [] ivQ;
        }

    size_t capacity(
            ) const
        {
        return ivCapacity;
        }

    size_t depth(
            ) const
        {
        return (ivInIdx - ivOutIdx);
        }

    bool pop(
            Item & anItem
            )
        {
        if ( depth() == 0 )
            {
            TOUT << "empty\n";

            return false;
            }

        size_t offset = ivOutIdx % ivCapacity;

        ivOutIdx ++;

        anItem = ivQ[ offset ];

        return true;
        }

    void push(
            const Item & anItem
            )
        {
        if ( depth() == ivCapacity )
            {
            TOUT << "Q full\n";

            return;
            }

        size_t offset = ivInIdx % ivCapacity;

        ivQ[ offset ] = anItem;

        ivInIdx ++;
        }

    void reset(
            )
        {
        ivInIdx = ivOutIdx = 0;
        }

  private:
    Item * ivQ;
    
    size_t ivCapacity;
    size_t ivInIdx;
    size_t ivOutIdx;
    };
#endif
