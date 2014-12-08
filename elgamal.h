#ifndef _ELGAMAL_H
#define _ELGAMAL_H

#include <gmp.h>
#include <gmpxx.h>
#include <string>
#include <vector>


namespace elgamal {

	struct public_key {
		int kbit;
		mpz_class p;
		mpz_class g;
		mpz_class y;
		
		std::string to_str() const;
		std::string to_binstr() const;
	};

	struct private_key {
		mpz_class x;

		std::string to_str() const;
		std::string to_binstr() const;
	};

	struct key {
		public_key pub;
		private_key priv;

		std::string to_str() const;
	};

	struct ciphertext {
		mpz_class gamma;
		mpz_class delta;
	};

	key generate_key(int kbit, long seed);

	ciphertext encrypt_message(public_key pub, mpz_class message, long __seed = 0);

	mpz_class decrypt_message(key my_key, ciphertext cipher);

}

#endif // _ELGAMAL_H