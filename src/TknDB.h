#ifndef __TknDB_h__
#define __TknDB_h__

#include <iostream>
#include <map>
#include <string>

class TknDB
    {
  public:
    static TknDB * instance();

    bool contains( const char * const aKeyStr );
    bool contains( const std::string & aKey );

    void display( std::ostream & aStream ) const;

    std::string get( const char * const aKeyStr );
    std::string get( const std::string & aKey );

    void put( const char * const aKeyStr, const std::string & aVal );
    void put( const std::string & aKey, const std::string & aVal );

    void remove( const char * const aKey );
    void remove( const std::string & aKey );

  private:
    TknDB();

    typedef std::map< std::string, std::string > DB_t;

    DB_t ivDB;
    };

std::ostream & operator << ( std::ostream & aStream, const TknDB & anObj );

#endif

