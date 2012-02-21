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

#if !defined(__SAMPLER_H)
#define __SAMPLER_H

#include <nori/object.h>

NORI_NAMESPACE_BEGIN

/**
 * \brief Abstract sample generator
 *
 * A sample generator is responsible for generating the random number stream
 * that will be passed an \ref Integrator implementation as it computes the
 * radiance incident along a specified ray.
 *
 * The most simple conceivable sample generator is just a wrapper around the
 * Mersenne-Twister random number generator and is implemented in
 * <tt>independent.cpp</tt> (it is named this way because it generates 
 * statistically independent random numbers).
 *
 * Fancier samplers might use stratification or low-discrepancy sequences
 * (e.g. Halton, Hammersley, or Sobol point sets) for improved convergence.
 * Another use of this class is in producing intentionally correlated
 * random numbers, e.g. as part of a Metropolis-Hastings integration scheme.
 *
 * The general interface between a sampler and a rendering algorithm is as 
 * follows: Before beginning to render a pixel, the rendering algorithm calls 
 * \ref generate(). The first pixel sample can now be computed, after which
 * \ref advance() needs to be invoked. This repeats until all pixel samples have
 * been exhausted.  While computing a pixel sample, the rendering 
 * algorithm requests (pseudo-) random numbers using the \ref next1D() and
 * \ref next2D() functions.
 *
 * Conceptually, the right way of thinking of this goes as follows:
 * For each sample in a pixel, a sample generator produces a (hypothetical)
 * point in an infinite dimensional random number hypercube. A rendering 
 * algorithm can then request subsequent 1D or 2D components of this point 
 * using the \ref next1D() and \ref next2D() functions. Fancy implementations
 * of this class make certain guarantees about the stratification of the 
 * first n components with respect to the other points that are sampled 
 * within a pixel.
 */
class Sampler : public NoriObject {
public:
	/// Release all memory
	virtual ~Sampler() { }

	/**
	 * \brief Clone the current instance
	 *
	 * This is used to create a separate sampler for each thread when rendering
	 * with multiple cores. The clone is expected to be different to some 
	 * extent, e.g. a pseudorandom generator should be based on a different random 
	 * seed compared to the original. Other parameters (e.g. those that 
	 * influence the stratification quality) should be copied exactly.
	 */
	virtual Sampler *clone() = 0;

	/**
	 * \brief Prepare to generate new samples
	 * 
	 * This function is called initially and every time the 
	 * integrator starts rendering a new pixel.
	 */
	virtual void generate() = 0;

	/// Advance to the next sample
	virtual void advance() = 0;

	/// Retrieve the next component value from the current sample
	virtual float next1D() = 0;

	/// Retrieve the next two component values from the current sample
	virtual Point2f next2D() = 0;

	/// Return the number of configured pixel samples
	virtual inline size_t getSampleCount() const { return m_sampleCount; }

	/**
	 * \brief Return the type of object (i.e. Mesh/Sampler/etc.) 
	 * provided by this instance
	 * */
	EClassType getClassType() const { return ESampler; }
protected:
	size_t m_sampleCount;
};

NORI_NAMESPACE_END

#endif /* __SAMPLER_H */
