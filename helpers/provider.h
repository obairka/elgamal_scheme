#ifndef _PROVIDER_H
#define _PROVIDER_H

#include <gmp.h>
#include <gmpxx.h>
#include <string>
#include <vector>
#include <fstream>

#include "exception.h"

class Provider {
public:
	Provider(const std::string& src, const std::string& dest);

	bool read(std::vector<unsigned char>& emptybuf, int kbyte);
	bool write(const std::vector<unsigned char>& buffer);
	
	void open();
	void close();

	bool is_empty() const; 
private:	
	std::string src;
	std::string dest;

	std::vector<unsigned char> input_data;
	std::vector<unsigned char> output_data;

	std::vector<unsigned char>::iterator current_pos;
};

#endif //_PROVIDER_H