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
