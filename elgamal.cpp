#include "elgamal.h"
#include "helpers/exception.h"

#include <sstream>
#include <ctime>
#include <cstdlib>


static const int MAX_ITERS_GEN_RAND = 500;

static mpz_class modular_power(mpz_class base, mpz_class exp, mpz_class mod) 
{
	mpz_class result;
	mpz_powm (result.get_mpz_t(), base.get_mpz_t(), exp.get_mpz_t(), mod.get_mpz_t());
	return result;
}
 
static mpz_class random_kbit_prime(gmp_randclass& rand, int k)
{
	
	mpz_class min;
	mpz_ui_pow_ui(min.get_mpz_t(), 2, k-1);
	mpz_class max;
	mpz_ui_pow_ui(max.get_mpz_t(), 2, k+1);

	//std::cout << "min = " << min << std::endl;
	//std::cout << "max = " << max << std::endl;

	for (int i = 0; i < MAX_ITERS_GEN_RAND; ++i)
	{
		mpz_class number = rand.get_z_bits(mpz_class(k));
		
		mpz_class prime;
		mpz_nextprime(prime.get_mpz_t(), number.get_mpz_t());

		if (prime < min) 
			continue;
		if (prime < max) {	

			return prime;
		}
	}

	throw Exception("Can't generate random k-bit prime");
}


// Finding a generator of a cyclic group
// INPUT: 
//	rand - random generator
//	a cyclic group G of order group_order, and the prime factorization group_order = p1^e1 ... pk ^ ek  = 2 * q;
// OUTPUT: a generator g of G 
static mpz_class cyclic_group_generator(gmp_randclass& rand, mpz_class group_order )
{
	// p = 2*q+1
	// n = p-1 = 2 * q;
	mpz_class g;

	for (int i = 0; i < MAX_ITERS_GEN_RAND; ++i)
	{
		// get random element of group
		g =  rand.get_z_range(group_order-1) + 1;
		
		// b1 = g ^2 % group_order
		mpz_class b1 = modular_power(g, 2, group_order);
				
		if (b1 == 1) {
			// g is not generator
			continue;
		}

		// p1 = 2, p2 = q 
		mpz_class q = group_order / 2;
		
		// b2 = g^q % group_order
		mpz_class b2 = modular_power(g, q, group_order);
		
		if (b2 == 1) {
			// g is not generator
			continue;
		}
		return g;
	}
	throw Exception("Can't find generator of group G!");
} 

// Generate safe prime
// safe prime p is a prime of the form p = 2q + 1 where q is  prime
// input: 	rand - seeded random generator
//			k - the required bitlength of the prime
static mpz_class safe_prime(gmp_randclass& rand, int k)
{
	mpz_class q, p;
	
	for (int i = 0; i < MAX_ITERS_GEN_RAND; ++i)
	//while (true)
	{
		// select a random (k-1)-bit random prime
		q = random_kbit_prime(rand, k-1);

		p = 2*q + 1;	

		// test if p is prime

		if (mpz_probab_prime_p(p.get_mpz_t(), 10) )
		{
			return p;
		}
	}

	throw Exception("Can't generate safe prime");	
}


namespace elgamal {

	std::string public_key::to_str() const
	{
		std::stringstream ss;
		ss << "PUBLIC KEY:\n";
		ss << "\tp = "<< p <<"\n";
		ss << "\tg = "<< g <<"\n";
		ss << "\ty = "<< y <<"\n";
		return ss.str();
	}

	std::string private_key::to_str() const
	{
		std::stringstream ss;
		ss << "PRIVATE KEY:\n";
		ss << "\tx = "<< x <<"\n";
		return ss.str();
	}

	std::string public_key::to_binstr() const
	{
		std::stringstream ss;
		ss << "PUBLIC KEY:\n";
		ss << "\tp = "<< p.get_str(2) <<"\n";
		ss << "\tk = "<< p.get_str(2).size() <<"\n";
		ss << "\tg = "<< g.get_str(2) <<"\n";
		ss << "\ty = "<< y.get_str(2) <<"\n";
		return ss.str();
	}

	std::string private_key::to_binstr() const
	{
		std::stringstream ss;
		ss << "PRIVATE KEY:\n";
		ss << "\tx = "<< x.get_str(2) <<"\n";
		return ss.str();
	}

	std::string key::to_str() const
	{
		std::stringstream ss;
		ss <<  pub.to_str() << "\n";
		ss << priv.to_str() << "\n";
		return ss.str();
	}
	
	key generate_key(int kbit, long seed)
	{
		gmp_randclass rand(gmp_randinit_default);
		rand.seed(seed);

		public_key pub;

		pub.kbit = kbit;
		pub.p = safe_prime(rand, kbit);	
		pub.g = cyclic_group_generator(rand, pub.p - 1);

		private_key priv;

		priv.x = rand.get_z_range(pub.p-2) + 1;

		pub.y = modular_power(pub.g, priv.x, pub.p);

		key elgamal_key;
		elgamal_key.pub = pub;
		elgamal_key.priv = priv;

		return elgamal_key;
	}

	ciphertext encrypt_message(public_key pub, mpz_class message, long __seed)
	{
		long seed = __seed;
		if (0 == __seed)
			seed = time(0);

		gmp_randclass rand(gmp_randinit_default);
		rand.seed(seed);

		// 1 < session_key < p - 1
		mpz_class session_key = rand.get_z_range(pub.p-1) + 1;

		ciphertext cipher;
		cipher.gamma = modular_power(pub.g, session_key, pub.p);
		cipher.delta = (message * modular_power(pub.y, session_key, pub.p) ) % pub.p;

		return cipher;
	}

	mpz_class decrypt_message(key my_key, ciphertext cipher)
	{
		// gamma_a = gamma^(p-1-x ) % p = gamma ^ (-x) % p = g^(-x* session_key)
		mpz_class g_exp = my_key.pub.p - 1 - my_key.priv.x;
		mpz_class gamma_a = modular_power(cipher.gamma, g_exp, my_key.pub.p);
		
		// Magic:
		// M = delta * (gamma ^-x) = delta * gamma ^ (p - 1 -x)
		//   = g^x*session_key * M * (g^ (p*session_key - sessoin_key -x * session_key)
		//   = M* g^ (p*session_key) = M
		mpz_class M = (gamma_a * cipher.delta)	% my_key.pub.p;
		
		return M;
	}
	
}