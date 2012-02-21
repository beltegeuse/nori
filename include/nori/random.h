/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2012 by Wenzel Jakob and Steve Marschner.

    Nori is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License Version 3
    as published by the Free Software Foundation.

    Nori is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#if !defined(__RANDOM_H)
#define __RANDOM_H

#include <nori/common.h>

NORI_NAMESPACE_BEGIN

/* Period parameters for the Mersenne Twister RNG */
#define MT_N 624
#define MT_M 397
#define MT_MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define MT_UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define MT_LOWER_MASK 0x7fffffffUL /* least significant r bits */

/**
 * \brief Mersenne Twister: pseudorandom number generator based on a
 * twisted generalized feedback shift register
 *
 * Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
 * All rights reserved.                          
 */
class Random {
public:
	/// Create an uninitialized instance
	Random();

	/// Seed the RNG with the specified seed value
	void seed(uint32_t value);
	
	/// Seed the RNG with an entire array 
	void seed(uint32_t *values, int length);
	
	/// Seed the RNG using an existing instance
	void seed(Random *random);

	/// Generate an uniformly distributed 32-bit integer
	uint32_t nextUInt();

	/// Generate an uniformly distributed single precision value on [0,1)
	float nextFloat();
private:
	uint32_t m_mt[MT_N];
	int m_mti;
};

NORI_NAMESPACE_END

#endif /* __RANDOM_H */
