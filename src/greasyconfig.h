/* 
 * This file is part of GREASY software package
 * Copyright (C) by the BSC-Support Team, see www.bsc.es
 * 
 * GREASY is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
*/

#ifndef GREASYCONFIG_H
#define GREASYCONFIG_H

#include <string>
#include <map>

#include "greasyutils.h"

using namespace std;

/**
 * Class to load configuration parameters for the application.
 * This class will read the specified file that must be propperly formatted.
 * Information will be stored in a map containing the pairs key-value.
 * An example of a correct configuration file is shown below:
 * 
 * #
 * # Example of configuration file
 * #
 * 
 * # This is a comment.
 * # It will be ignored.
 * 
 * # An example of key-value pair is this:
 * 
 * key1=value1 # This comment will also be ignored
 * 
 * # Spaces are allowed between key, equal and value.
 * 
 * key2 = value2 
 * 
 * # End of example configuration file
 * 
 * It is also able to read parameters from the environment. It will treat all
 * the environment variables with prefix GREASY_ as parameters. For example,
 * defining the environment variable GREASY_X=2, X=2 would be stored in the
 * configuration. If the same parameter is defined in both places, the value
 * taken will be the one in the environment.
 * 
 * The keys will be always stored in uppercase, so all find operations are
 * case insensitive.
 * 
*/
class GreasyConfig {

public:

  /**
    * Get the unique GreasyConfig instance. Implementation of the Singleton
    * Pattern.
    * @return A pointer to the GreasyConfig instance.
    */
  static GreasyConfig* getInstance();
  
  /**
    * Read the config. It will read and parse the config file specified
    * and store it in the map. It is also able to overload file keys with 
    * environment variables.
    * @param fileName Path to file to be parsed.
    * @return True if all is ok, false otherwise.
    */
  bool readConfig(string fileName);
  
  /**
    * Boolean function to know if the config has been processed propperly.
    * @return True if the object is ready, false otherwise.
    */
  bool isReady();
  
  /**
    * Boolean function to know if a certain key is in the configuration.
    * @param key 
    * @return true if the key exists, false otherwise.
    */
  bool keyExists(const string& key);
  
  /**
    * Boolean function to know if a certain key pattern exists in the map.
    * @param pattern 
    * @return true if the pattern of the key exists, false otherwise.
    */	
  bool keyPatternExists(const string& pattern);
  
  /**
    * Get the value for a certain key. If the key is not in the configuration,
    * an empty string will be returned.
    * @param key 
    * @return The value associated to the key, or an empty string if not there.
    */
  string getValue(const string& key);
  
  /**
    * Insert a pair key-value to the configuration
    * @param key 
    * @param value 
    */	
  void insert(string key, string value);
  
  /**
    * Prints the contents of the configuration into a string for debugging
    * purposes.
    * @returns the string 
    */		
  string printContents();
  
  /**
    * Wrapper to the begin function of the map
    * @returns the iterator to the first entry of the configuration 
    */	
  map<string,string>::iterator begin();
  
  /**
    * Wrapper to the end function of the map
    * @returns the iterator to the last entry of the configuration 
    */		
  map<string,string>::iterator end();
	

protected:
  
  /**
    * Default Constructor.
    */
  GreasyConfig();
  
  /**
    * Default destructor.
    */
  ~GreasyConfig();
  
  /**
    * Load the config from the environment variables of the form GREASY_KEY where KEY is the key in uppercase found in the config file.
    * @param fileName Path to file to be parsed.
    * @return True if all is ok, false otherwise.
    */
  void loadEnvConfig();	

  const static string entryPattern; /**< Pattern of the lines containing the values in the configuration file. */
  static GreasyConfig instance; /**< Unique instance of the config file. */
  bool ready; /**< Boolean telling if the configuration values have been processed. */
  map<string,string> contents; /**< Map containing all the pairs key-value from the configuration file. */
  
};

#endif
