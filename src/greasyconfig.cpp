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

#include "greasyconfig.h"
#include "greasyregex.h"

#include <fstream>
#include <iostream>
#include <unistd.h>

using namespace std;

const string GreasyConfig::entryPattern = "^[[:space:]]*([A-Za-z0-9_%/() -]+)[[:space:]]*=[[:space:]]*([A-Za-z0-9_%/().,:-]+)[[:space:]]*[#]*.*$";


GreasyConfig::GreasyConfig() {

  ready = false;

}


GreasyConfig::~GreasyConfig() { }


GreasyConfig* GreasyConfig::getInstance() {

  static GreasyConfig instance;
  return &instance;
}


bool GreasyConfig::readConfig(string filename) {

  ifstream myfile(filename.c_str());
  GreasyRegex entryReg = GreasyRegex(entryPattern);
  string line;
  vector<string> matches;
  
  if (myfile.is_open()) {
    // Read the configuration file
    while (!myfile.eof()){
      getline (myfile,line);
      if (entryReg.multipleMatch(line,matches,6) > 0) {
	insert(matches[1],matches.back());
      }
      matches.clear();
    }
    myfile.close();
    // If available, load configuration from the environment.
    // This may override options config parameters!
    loadEnvConfig();
    ready = true;
  } else ready = false;
  return ready;
}


void GreasyConfig::loadEnvConfig() {

  extern char **environ;
  string envEntry;
  vector<string> pair;
  int i;
    
  // Fill the map with each pair of key=value...
  for (i=0; environ[i]; i++) {
    	envEntry = environ[i];
	pair = split(envEntry,'=');
	string key = GreasyRegex::match(pair[0],"GREASY_([A-Za-z0-9_]+)");
	if (!key.empty()) {
	  if (pair.size() > 1) insert(key,pair[1]);
	  else insert(key,"");
	}
  }
  
}



bool GreasyConfig::isReady() {

  return ready;
	
}


bool GreasyConfig::keyExists(const string& key) {

  string ukey = key;
  return contents.find(toUpper(ukey))!=contents.end();

}


bool GreasyConfig::keyPatternExists(const string& pattern) {

  map<string,string>::iterator it;
  GreasyRegex keyReg = GreasyRegex(pattern);
  vector<string> matches;

  for ( it=contents.begin() ; it != contents.end(); it++ ) {
    if( keyReg.multipleMatch(it->first,matches,6)>0) return true;  
  }
  return false;
  

}


string GreasyConfig::getValue(const string& key) {

  string ukey = key;
  map<string,string>::iterator it = contents.find(toUpper(ukey));
  if (it==contents.end()) return "";
  else return it->second;

}


void GreasyConfig::insert(string key, string value){
  
  string ukey = key;
  contents[toUpper(ukey)]=value;
    
}


string GreasyConfig::printContents() {
  
  string result="";
  map<string,string>::iterator it;

  for ( it=contents.begin() ; it != contents.end(); it++ )
    result +=it->first+"="+it->second+"\n";

  return result;
  
}


map<string,string>::iterator GreasyConfig::begin() {
  
  return contents.begin();

}


map<string,string>::iterator GreasyConfig::end() {
  
  return contents.end();
  
}
