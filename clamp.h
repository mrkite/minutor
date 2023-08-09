// Copyright (c) 2016, EtlamGit
#ifndef CLAMP_H_
#define CLAMP_H_

namespace std {

// will be in STD with C++17
#if __cplusplus < 201703L
template < class T >
const T& clamp( const T& value, const T& lower, const T& upper )
{
  return (value<lower)? lower : (upper<value)? upper : value;
}
#endif

}
#endif  // CLAMP_H_
