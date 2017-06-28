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

#include <string>
#include <vector>
#include <regex.h>

#define NMATCH 6

using namespace std;


/**
 * Class to make life easier when working with regular expressions.
 * The syntax for the regular expressions is the POSIX Extended Regular Expression syntax. 
 * 
*/
class GreasyRegex {

public:
  /**
    * Default constructor. It will compile the regular expression given in pattern.
    * @param pattern
    */
  GreasyRegex(string pattern);
  
  /**
    * Default destructor.
    */
  ~GreasyRegex();
  
  /**
    * Static function to easy match a simple regular expresion.
    * @param str The string to be processed.
    * @param pattern The search pattern.
    * @return The value of the first match.
    */
  static string match(string str,string pattern); 
  
  /**
    * Function to match a certain pattern into a string. It is able to handle multiple matches using a vector of results passed by reference.
    * @param str The string to be processed.
    * @param matches The reference of the vector where matches will be stored
    * @param nmatches Maximum number of matches to find.
    * @return The number of matches found.
    */
  int multipleMatch(string str, vector<string>& matches, int nmatches=5);

protected:
	regex_t regexp; /**< Regular expression handler */
	
};
