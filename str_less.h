///\file str_less.h
#pragma once

#include <functional>

namespace std
{

struct str_less : public less<string>
  {
  bool operator () (const char* s1, const char* s2) const
    { return strcmp(s1, s2) < 0; }
  bool operator () (const string& s1, const char* s2) const
    { return strcmp(s1.c_str(), s2) < 0; }
  bool operator () (const char* s1, const string& s2) const
    { return strcmp(s1, s2.c_str()) < 0; }
  };

}
