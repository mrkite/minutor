/** Copyright (c) 2022, EtlamGit */
/* This is basically a reimplementation of parts of JAVA Random class */

#include "java.h"
#include <stdexcept>


namespace Java {

Random::Random(long long seed)
{
  this->seed = (seed ^ 0x5DEECE66DLL) & ((1LL << 48) - 1);
}

int Random::nextInt()
{
  return next(32);
}

int Random::nextInt(int n)
{
  if (n <= 0)
    throw std::invalid_argument("n must be positive");

  if ((n & -n) == n) // i.e., n is a power of 2
    return (int) ((n * (long long) next(31)) >> 31);

  int bits, val;
  do {
    bits = next(31);
    val  = bits % n;
  } while (bits - val + (n - 1) < 0);
  return val;
}

int Random::next(int bits)
{
  seed = (seed * 0x5DEECE66DLL + 0xBLL) & ((1LL << 48) - 1);
  return (int) (static_cast<unsigned long long>(seed) >> (48 - bits));
}


}
