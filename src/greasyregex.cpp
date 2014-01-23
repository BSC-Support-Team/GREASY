#include "greasyregex.h"
#include "greasyutils.h"
#include <iostream>
#include <vector>

GreasyRegex::GreasyRegex(string pattern) {

  regcomp(&regexp, pattern.c_str(), REG_EXTENDED);

}

GreasyRegex::~GreasyRegex() {

  regfree(&regexp);

}


string GreasyRegex::match(string str, string pattern) {

  regex_t regexp;
  regmatch_t match[2];
  int error,dstbuflen;
  string result;
  const char * ptr = str.c_str();
  
  if (regcomp(&regexp,pattern.c_str(),REG_ICASE|REG_EXTENDED)!=0) return "";
  
  error=regexec(&regexp,ptr,2,match,0);
  if (error != 0) return "";
  
  dstbuflen=( match[1].rm_eo - match[1].rm_so <= 0 ? 0 : match[1].rm_eo - match[1].rm_so);
  if (match[1].rm_so < 0) return "";
  else result = string(ptr + match[1].rm_so,dstbuflen);
  
  return result;

}


int GreasyRegex::multipleMatch(string str, vector<string>& matches, int nmatches) {

  regmatch_t regmatch[nmatches];
  int size;
  int matchesFound = -1;
  int i = 0;
  const char * ptr = str.c_str();
  
  if (regexec(&regexp, ptr, nmatches, regmatch, 0) == 0) {
    matchesFound=0;
    for (i=0; (regmatch[i].rm_so >=0)&&(i<=nmatches); i++) {
      size = (regmatch[i].rm_eo - regmatch[i].rm_so <= 0 ? 0 : regmatch[i].rm_eo - regmatch[i].rm_so);
      if (size > 0) {
	matches.push_back(string(ptr + regmatch[i].rm_so,size));
	matchesFound++;
      }
    }
  }

  return matchesFound;

}
