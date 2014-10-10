#ifndef STRINGUTIL_H
#define STRINGUTIL_H

#include <string>
#include <vector>

using namespace std;

void SplitString( const string& str, vector<string>& tokens, const string& delimiters = " " );

string Trim( string &str, const string& delims = " \t\n\r", bool left = true, bool right = true );
void TrimLeft( string &str, const string& delims = " \t\n\r");
void TrimRight( string &str, const string& delims = " \t\n\r");
string IntToString( int val );
string DoubleToString( double val );

#endif // STRINGUTIL_H
