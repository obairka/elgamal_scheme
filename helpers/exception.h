#ifndef _EXCEPTION_H
#define _EXCEPTION_H
#include <exception>
#include <string>

struct Exception : public std::exception
{
   std::string s;
   Exception(std::string ss) : s(ss) {}
   ~Exception() throw () {} // Updated
   const char* what() const throw() { return s.c_str(); }
};

#endif