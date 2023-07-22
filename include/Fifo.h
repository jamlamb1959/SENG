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

        // ivMut = xSemaphoreCreateMutex();
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

    Item pop(
            )
        {
        // Locker l( ivMut );

        static Item null = NULL;

        if ( depth() == 0 )
            {
            TOUT << "empty\n";

            return null;
            }

        size_t offset = ivOutIdx % ivCapacity;

        ivOutIdx ++;

        return ivQ[ offset ];
        }

    void push(
            const Item & anItem
            )
        {
        // Locker l( ivMut );

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
        // Locker l( ivMut );

        ivInIdx = ivOutIdx = 0;
        }

  private:
    Item * ivQ;
    
    size_t ivCapacity;
    size_t ivInIdx;
    size_t ivOutIdx;

    // SemaphoreHandle_t ivMut;
    };
#endif
