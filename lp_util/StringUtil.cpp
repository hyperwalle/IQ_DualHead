
#include "StringUtil.h"
#include <sstream>

using namespace std;

void SplitString( const string& str,
                  vector<string>& tokens,
                  const string& delimiters )
{
    tokens.clear();
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of( delimiters, 0 );
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of( delimiters, lastPos );

    while( string::npos != pos || string::npos != lastPos )
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

string Trim( string &str, 
             const string& delims, 
             bool left, 
             bool right )
{
    if(left)
    {
        TrimLeft( str, delims );	
    }
    if(right)
    {
        TrimRight( str, delims );	
    }
    return str;
}

void TrimLeft( string &str, 
               const string& delims )
{
    str.erase( 0, str.find_first_not_of( delims ));
}

void TrimRight( string &str, const string& delims )
{
    str.erase( str.find_last_not_of( delims ) + 1 );
}

string IntToString( int val )
{
    stringstream out;
    out << val;
    return out.str();
}

string DoubleToString( double val )
{
    stringstream out;
    out << val;
    return out.str();
}
