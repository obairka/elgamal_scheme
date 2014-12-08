#include "helpers/provider.h"
#include "helpers/converter.h"
#include "helpers/exception.h"
#include "elgamal.h"

#include <vector>
#include <ctime>
#include <iostream>

//getopt
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>


int main(int argc, char* argv[])
{
	int index;
	int c;
	int kbit = 0;
	std::string input;
	std::string output;
	opterr = 0;

	while ((c = getopt (argc, argv, "k:i:o:")) != -1)
	{
		switch (c)
		{
			case 'k':
				// k bits
				kbit = atoi(optarg);
			break;

			case 'i':
				// input file name
				input = std::string(optarg);
			break;

			case 'o':
				// input file name
				output = std::string(optarg);
			break;			
			
			default:
        		std::cerr << "Usage: " << argv[0] << "-k key -o output -i input"<<std::endl;
        		return 0;
		}
	}

	if (input.empty() || output.empty() || kbit == 0)
	{
		std::cerr << "Usage: " << argv[0] << "-k key -o output -i input"<<std::endl;
		return 0;
	}


	Provider input_cipher(input, "cipher.txt");

	elgamal::key key = elgamal::generate_key(kbit, time(0));

	std::cout << key.to_str();

	input_cipher.open();

	int mkbit = kbit / 2; // message read k bit

	while (true)
	{
		std::vector<unsigned char> buffer;
		if (!input_cipher.read(buffer, mkbit/8)) {
			// stop
			break;
		}
		
		mpz_class input_M = Converter::buf_to_mpz(buffer);
		// encryption
		elgamal::ciphertext cipher = elgamal::encrypt_message(key.pub, input_M);
		

		std::vector<unsigned char> gamma_buf( Converter::mpz_to_buf(cipher.gamma, kbit));
	 	std::vector<unsigned char> delta_buf( Converter::mpz_to_buf(cipher.delta, kbit));

	 	input_cipher.write(gamma_buf);
	 	input_cipher.write(delta_buf);
	}
	input_cipher.close();

	Provider cipher_output("cipher.txt", output); 
	
	cipher_output.open();

	while (true)
	{
		std::vector<unsigned char> buffer;
		if (!cipher_output.read(buffer, kbit/8)) {
			// stop
			break;
		}
		mpz_class gamma = Converter::buf_to_mpz(buffer);
		buffer.clear();
		if (!cipher_output.read(buffer, kbit/8)) {
			// gamma есть, а delta нет!
			throw Exception("Wrong ciphertext file!");
		}
		mpz_class delta = Converter::buf_to_mpz(buffer);
		elgamal::ciphertext cipher;
		cipher.gamma = gamma;
		cipher.delta = delta;

		//decryption
		mpz_class output_M = elgamal::decrypt_message(key, cipher);

		std::vector<unsigned char> out_msg_buf;

		bool last_zeros = false;
		if (cipher_output.is_empty())
		{
			// last bytes
			last_zeros = true;
		}
		out_msg_buf = Converter::mpz_to_buf(output_M,  mkbit, last_zeros);	
		//std::cout << output_M << std::endl;
		//std::cout << Converter::buf_to_bin(out_msg_buf) << std::endl;
		
		cipher_output.write(out_msg_buf);
	}	 
	cipher_output.close();
	
	std::cout << std::endl;


}