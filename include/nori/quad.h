/*
    This file is part of Mitsuba, a physically based rendering system.

    Copyright (c) 2007-2011 by Wenzel Jakob and others.

    Mitsuba is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License Version 3
    as published by the Free Software Foundation.

    Mitsuba is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#if !defined(__QUADRATURE_H)
#define __QUADRATURE_H

#include <nori/common.h>
#include <boost/function.hpp>

NORI_NAMESPACE_BEGIN

/**
 * \brief Adaptively computes the integral of a multidimensional function using 
 * either a Gauss-Kronod (1D) or a Genz-Malik (>1D) cubature rule.
 *
 * This class is a C++ wrapper around the \c cubature code by Steven G. Johnson
 * (http://ab-initio.mit.edu/wiki/index.php/Cubature)
 *
 * The original implementation is based on algorithms proposed in
 *
 * A. C. Genz and A. A. Malik, "An adaptive algorithm for numeric integration
 * over an N-dimensional rectangular region," J. Comput. Appl. Math. 6 (4),
 * 295–302 (1980).
 *
 * and
 *
 * J. Berntsen, T. O. Espelid, and A. Genz, "An adaptive algorithm for the
 * approximate calculation of multiple integrals," ACM Trans. Math. Soft. 17
 * (4), 437–451 (1991).
 *
 * \ingroup libcore
 */
class NDIntegrator {
public:
	typedef boost::function<void (const double *, double *)>         Integrand;
	typedef boost::function<void (size_t, const double *, double *)> VectorizedIntegrand;

	enum EResult {
		ESuccess = 0,
		EFailure = 1
	};

	/**
	 * Initialize the Cubature integration scheme
	 *
	 * \param fDim Number of integrands (i.e. dimensions of the image space)
	 * \param nDim Number of integration dimensions (i.e. dimensions of the
	 *      function domain)
	 * \param maxEvals Maximum number of function evaluationn (0 means no 
	 *      limit). The error bounds will likely be exceeded when the
	 *      integration is forced to stop prematurely. Note: the actual 
	 *      number of evaluations may somewhat exceed this value.
	 * \param absError Absolute error requirement (0 to disable)
	 * \param relError Relative error requirement (0 to disable)
	 */
	NDIntegrator(size_t fDim, size_t dim,
			size_t maxEvals, double absError = 0, double relError = 0);

	/**
	 * \brief Integrate the function \c f over the rectangular domain 
	 * bounded by \c min and \c max.
	 *
	 * The supplied function should have the interface
	 *
	 * <code>
	 * void integrand(const double *in, double *out);
	 * </code>
	 *
	 * The input array \c in consists of one set of input parameters
	 * having \c dim entries. The function is expected to store the
	 * results of the evaluation into the \c out array using \c fDim entries.
	 */
	EResult integrate(const Integrand &f, const double *min, const double *max,
			double *result, double *error, size_t *evals = NULL) const;

	/**
	 * \brief Integrate the function \c f over the rectangular domain 
	 * bounded by \c min and \c max.
	 *
	 * This function implements a vectorized version of the above
	 * integration function, which is more efficient by evaluating
	 * the integrant in `batches'. The supplied function should 
	 * have the interface
	 *
	 * <code>
	 * void integrand(int numPoints, const double *in, double *out);
	 * </code>
	 *
	 * Note that \c in in is not a single point, but an array of \c numPoints points
	 * (length \c numPoints x \c dim), and upon return the values of all \c fDim
	 * integrands at all \c numPoints points should be stored in \c out 
	 * (length \c fDim x \c numPoints). In particular, out[i*dim + j] is the j-th
	 * coordinate of the i-th point, and the k-th function evaluation (k<fDim)
	 * for the i-th point is returned in out[k*npt + i].
	 * The size of \c numPoints will vary with the dimensionality of the problem;
	 * higher-dimensional problems will have (exponentially) larger numbers,
	 * allowing for the possibility of more parallelism. Currently, \c numPoints
	 * starts at 15 in 1d, 17 in 2d, and 33 in 3d, but as the integrator
	 * calls your integrand more and more times the value will grow. e.g. if you end
	 * up requiring several thousand points in total, \c numPoints may grow to
	 * several hundred.
	 */
	EResult integrateVectorized(const VectorizedIntegrand &f, const double *min, 
		const double *max, double *result, double *error, size_t *evals = NULL) const;
protected:
	size_t m_fdim, m_dim, m_maxEvals;
	double m_absError, m_relError;
};

NORI_NAMESPACE_END

#endif /* __QUADRATURE_H */
