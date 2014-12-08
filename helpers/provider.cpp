#include "provider.h"

Provider::Provider(const std::string& src, const std::string& dest)
	: src(src), dest(dest)
{	
}

void Provider::open() // throws Exception
{
	std::ifstream input(src.c_str(), std::ios::binary|std::ios::ate);	
	if (!input.is_open())
	{
		throw Exception("Cannot open input file : [" + src + "]");
	}

	std::streampos size = input.tellg();
	input_data = std::vector<unsigned char>(size);
	input.seekg (0, std::ios::beg);
	input.read ((char*) &input_data[0], size);	
	input.close();
	current_pos = input_data.begin();
}

void Provider::close()
{
	std::ofstream output(dest.c_str(),  std::ios::binary);
	
	if (!output.is_open())
	{
		throw Exception("Cannot open output file : [" + dest + "]");
	}
	output.write((char*) &output_data[0], output_data.size());
	output.close();
	current_pos = input_data.begin();
}

bool Provider::is_empty() const
{
	return current_pos == input_data.end();
}

bool Provider::read(std::vector<unsigned char>& emptybuf, int kbyte)
{
	if (is_empty())
	{
		return false;
	}
	int ins_count = input_data.end() - current_pos;
	if (ins_count < kbyte)
	{
		kbyte = ins_count;
	}

	emptybuf.insert(emptybuf.end(), current_pos, current_pos+kbyte);

	current_pos += kbyte;

	return true;
}


bool Provider::write(const std::vector<unsigned char>& buf)
{
	output_data.insert(output_data.end(), buf.begin(), buf.end());
	return true;
}


