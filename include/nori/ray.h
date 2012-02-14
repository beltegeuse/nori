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

#if !defined(__RAY_H)
#define __RAY_H

#include <nori/vector.h>

NORI_NAMESPACE_BEGIN

/**
 * \brief Simple n-dimensional ray segment data structure
 * 
 * Along with the ray origin and direction, this data structure additionally
 * stores a ray segment [mint, maxt] (whose entries may include positive/negative
 * infinity), as well as the componentwise reciprocals of the ray direction.
 * That is just done for convenience, as these values are frequently required.
 */
template <typename _PointType, typename _VectorType> struct TRay {
	typedef _PointType                  PointType;
	typedef _VectorType                 VectorType;
	typedef typename PointType::Scalar  Scalar;

	PointType o;     ///< Ray origin
	VectorType d;    ///< Ray direction
	VectorType dRcp; ///< Componentwise reciprocals of the ray direction
	Scalar mint;     ///< Minimum position on the ray segment
	Scalar maxt;     ///< Maximum position on the ray segment

	/// Construct a new ray
	inline TRay() : mint(Epsilon), 
		maxt(std::numeric_limits<Scalar>::infinity()) { }

	/// Construct a new ray
	inline TRay(const PointType &o, const VectorType &d, 
		Scalar mint, Scalar maxt) : o(o), d(d), mint(mint), maxt(maxt) {
		update();
	}

	/// Copy constructor
	inline TRay(const TRay &ray) 
	 : o(ray.o), d(ray.d), dRcp(ray.dRcp),
	   mint(ray.mint), maxt(ray.maxt) { }

	/// Copy a ray, but change the covered segment of the copy
	inline TRay(const TRay &ray, Scalar mint, Scalar maxt) 
	 : o(ray.o), d(ray.d), dRcp(ray.dRcp), mint(mint), maxt(maxt) { }

	/// Update the reciprocal ray directions after changing 'd'
	inline void update() {
		dRcp = d.array().cwiseInverse().matrix();
	}

	/// Return the position of a point along the ray
	inline PointType operator() (Scalar t) const { return o + t * d; }

	/// Return a human-readable string summary of this ray
	inline QString toString() const {
		return QString(
				"Ray[\n"
				"  o = %1,\n"
				"  d = %2,\n"
				"  mint = %3,\n"
				"  maxt = %4\n"
				"]")
			.arg(o.toString())
			.arg(d.toString())
			.arg(mint)
			.arg(maxt);
	}
};

NORI_NAMESPACE_END

#endif /* __RAY_H */

