
#ifndef __Filo_h__
#define __Filo_h__

#include <iostream>

#define defCapacity_l   10
#ifndef TOUT
#define TOUT  std::cout << __FILE__ << "(" << __LINE__ << "): "
#endif

template < class Item >
class Filo
    {
  public:
    Filo(
            const size_t aCapacity = defCapacity_l
            )
            : ivCapacity( aCapacity )
            , ivInIdx( 0 )
        {
        ivQ = new Item[ ivCapacity ];
        }

    ~Filo(
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
        return ivInIdx;
        }

    Item pop(
            )
        {
        static Item null;

        if ( depth() == 0 )
            {
            TOUT << "empty\n";

            return null;
            }

        ivInIdx --;

        size_t offset = ivInIdx % ivCapacity;

        return ivQ[ offset ];
        }

    void push(
            const Item & anItem
            )
        {
        if ( depth() == ivCapacity )
            {
            TOUT << "full\n";

            return;
            }

        size_t offset = ivInIdx % ivCapacity;

        ivQ[ offset ] = anItem;

        ivInIdx ++;
        }

    void reset(
            )
        {
        ivInIdx = 0;
        }

  private:
    Item * ivQ;
    
    size_t ivCapacity;
    size_t ivInIdx;
    };
#endif
