#ifndef _CONVERTER_H
#define _CONVERTER_H

#include "exception.h"
#include <gmp.h>
#include <gmpxx.h>
#include <sstream>
#include <bitset>
#include <string>

class Converter {
public:
	static mpz_class buf_to_mpz(const std::vector<unsigned char> buf)
	{
		std::string binary_string(buf_to_bin(buf));
		mpz_class long_number(binary_string,2);
		return long_number;
	}

	static std::vector<unsigned char> mpz_to_buf(mpz_class number, int kbit, bool param = false)
	{
		std::string binary_string = number.get_str(2);
		return write_kbit_num(binary_string, kbit, param);
	}


	static std::string buf_to_bin(const std::vector<unsigned char>& buf)
	{
		std::stringstream ss;	
		for (int i = 0; i < buf.size(); ++i) {
			ss << std::bitset<8>(buf[i]) ;
		}	
		return ss.str();
	}

	static std::vector<unsigned char> write_kbit_num(const std::string& binary_string, int k, bool param = false)
	{
		int len = binary_string.length();
		int buf_size = (param ? (len+7)/8 : k/8 );
		std::vector<unsigned char> buf(buf_size);
		
		if (len > k) {
			throw Exception("len > k");
		} 

		int zero_bits_cnt = (param? 0 : k - len);

		if (zero_bits_cnt < 0) {
			std::stringstream ss;	
			ss << "Error: msg bit = " << k << "bin string len = " << len << std::endl;
			throw Exception(ss.str());
		}

		int zero_bytes_cnt = zero_bits_cnt / 8;
		int zeros_chunk_cnt = zero_bits_cnt % 8;

		for (int i = 0; i < zero_bytes_cnt; ++i) {
			buf[i] = 0;
		} 

		if (zero_bytes_cnt >= k/8) {
			return buf;
		}

		for (int i = 0; i < zeros_chunk_cnt; ++i) {
			write_bit(buf[zero_bytes_cnt], i, false);
		}
		int sw = 0;
		for (int i = zeros_chunk_cnt; i < 8; ++i) {
			write_bit(buf[zero_bytes_cnt], i, (binary_string[sw] == '1'));	
			++sw;
		}
		for (int i = zero_bytes_cnt+1; (i < k/8) && (sw < len); ++i){
			for (int j = 0; j < 8; ++j)
			{
				write_bit(buf[i], j, (binary_string[sw] == '1'));
				++sw;
			}	
		}
		return buf;
	}
private:
	static void write_bit(unsigned char& b, int s, bool x) 
	{
		b |=  ((x ? 1 : 0) << (7-s));
	}
};


#endif // _CONVERTER_H