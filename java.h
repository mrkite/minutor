/** Copyright (c) 2022, EtlamGit */
#ifndef JAVA_H
#define JAVA_H

namespace Java {

class Random
{
 public:
  Random(long long seed);
  int nextInt();
  int nextInt(int n);

 private:
  int next(int bits);
  long long seed;
};

}

#endif // JAVA_H
