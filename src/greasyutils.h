/* 
 * This file is part of GREASY software package
 * Copyright (C) by the BSC-Support Team, see www.bsc.es
 * 
 * GREASY is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 * 
 * GREASY is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GREASY. If not, see <http://www.gnu.org/licenses/>.
 *
*/

#ifndef GREASYUTILS_H
#define GREASYUTILS_H

#include <string>
#include <sstream>
#include <vector>

#include <unistd.h>

using namespace std;

/**
 * Greasyutils is a set of util methods and macros to make life easier to the developer.
 * 
*/

/**
 * MACRO aimed at debugging purposes. Usefull to identify the exact line where a call is crashing
 * supossing you know which call is failing (STR) but you don't know  exactly where in the code
 * is the failing call located.
*/
#define ERRCHECK( STR ) 						\
do { 									\
        try{								\
            STR	;							\
        }catch(exception& e){						\
            cerr << "exception caught in "<< __FILE__ << ":"<< __LINE__<< " => "  << e.what() << endl;	\
            exit (1);							\
        }								\
}while(0)


/**
  * Inline function to convert anything to a string.
  * @param t The variable to convert. 
  * @return The converted string.
  */
template <class T>
inline string toString (const T& t) {
  
  stringstream ss;
  ss << t;
  return ss.str();
  
}

/**
  * Inline function to convert a string to any kind of type.
  * @param t The variable where the converted value will reside.
  * @param s The string to convert.
  * @return The converted string.
  */
template <class T>
inline T fromString(T& t, const string& s) {
  
  istringstream iss(s);
  iss >> dec >> t;
  return t;
  
}

/**
  * Inline function to split a string using a given delimiter.
  * @param s The string to split.
  * @param delim The delimiter to be used.
  * @return A vector with all the splitted tokens.
  */
inline vector<string> split(const string &s, char delim) {
  
  vector<std::string> elems;
  stringstream ss(s);
  string item;
  
  while(getline(ss, item, delim))
    elems.push_back(item);
  
  return elems;
  
}

/**
  * Inline function to transform a string to uppercase
  * @param s The string to transform.
  * @return The string in uppercase.
  */
inline string toUpper(string& s) {
  
  for (size_t i = 0; i < s.length(); ++i)
    s[i]=toupper(s[i]);

  return s;
  
}

/**
  * Inline function to get the working dir.
  * @return The string containing the current working dir.
  */
inline string getWorkingDir() {
 
  char temp[4096];
  return ( getcwd(temp, 4096) ? std::string( temp ) + "/" : std::string("") );

}


#endif
