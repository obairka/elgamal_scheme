# for run export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
g++ elgamal.cpp scheme.cpp helpers/*.cpp -lgmpxx -lgmp -o elgamal.exe