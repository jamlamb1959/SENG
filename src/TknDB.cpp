
#include <iostream>

#include "TknDB.h"

#define TOUT std::cout << __FILE__ << "(" << __LINE__ << "): "

TknDB * TknDB::instance(
        )
    {
    static TknDB * inst = new TknDB();

    assert( inst != NULL );

    return inst;
    }

bool TknDB::contains( 
        const char * const aKeyStr 
        )
    {
    std::string key( aKeyStr );

    return contains( key );
    }

bool TknDB::contains( 
        const std::string & aKey 
        )
    {
    DB_t::iterator it;

    it = ivDB.find( aKey );
    return ( it != ivDB.end() );
    }


std::string TknDB::get( 
        const char * const aKeyStr 
        )
    {
    std::string key( aKeyStr );
    
    return get( key );
    }

std::string TknDB::get( 
        const std::string & aKey 
        )
    {
    static std::string _nil;

    if ( contains( aKey ) )
        {
        // TOUT << "get - " << aKey << " => " << ivDB[ aKey ] << "\n";
        return ivDB[ aKey ];
        }

    // TOUT << "get - " << aKey << " => _nil\n";
    return _nil;
    }


void TknDB::put( 
        const char * const aKeyStr, 
        const std::string & aVal 
        )
    {
    std::string key( aKeyStr );

    put( key, aVal );
    }

void TknDB::put( 
        const std::string & aKey, 
        const std::string & aVal 
        )
    {
    ivDB[ aKey ] = aVal;

    // TOUT << "put: " << aKey << " => " << aVal << "\n";
    }

void TknDB::remove( 
        const char * const aKey 
        )
    {
    std::string k( aKey );

    return remove( k );
    }

void TknDB::remove( 
        const std::string & aKey 
        )
    {
    ivDB.erase( aKey );
    }

TknDB::TknDB(
        )
    {
    }

void TknDB::display(
        std::ostream & aStream
        ) const
    {
    DB_t::const_iterator it;

    for( it = ivDB.begin(); it != ivDB.end(); it ++ )
        {
        aStream << "'" << it->first << "' => '" << it->second << "'\n";
        }
    }
    
std::ostream & operator << ( 
        std::ostream & aStream, 
        const TknDB & anObj 
        )
    {
    anObj.display( aStream );

    return aStream;
    }

