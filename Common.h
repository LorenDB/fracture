#ifndef COMMON_H
#define COMMON_H

#include <complex>
#include <boost/multiprecision/mpfr.hpp>

using big_float = boost::multiprecision::mpfr_float;
using complex = std::complex<big_float>;

#endif // COMMON_H
