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

#if !defined(__MEDIUM_H)
#define __MEDIUM_H

#include <nori/object.h>

NORI_NAMESPACE_BEGIN

/**
 * \brief Generic participating medium interface
 * 
 * This is the base class of all participating media implementations in Nori
 * (e.g. homogeneous/heterogeneous). It only contains two method, since that's 
 * all that is required to interface with path tracers: sampling a distance 
 * and evaluating the transmittance along a ray segment.
 */
class Medium : public NoriObject {
public:
 	/**
	 * \brief Importance sample the distance to the next medium 
	 * interaction along the specified ray
	 *
	 * \param ray
	 *    A ray data structure 
	 *
	 * \param[out] t
	 *    This parameter is used to return the sampled distance, when
	 *    medium sampling succeeded. Otherwise, it is not used.
	 *
	 * \param[out] weight
	 *    This parameter records the importance weight associated with 
	 *    the sample. When a position inside the medium is sampled, it 
	 *    records the product of sigma_s and the transmittance, divided
	 *    by the probability per unit length. When medium sampling fails,
	 *    it returns the transmittance along the ray, divided by the
	 *    discrete probability of failure.
	 *
	 * \return
	 *    \c true if medium sampling succeeded, and \c false otherwise.
	 */
	virtual bool sampleDistance(const Ray3f &ray, float &t, Color3f &weight) const = 0;

	/**
	 * \brief Evaluate the transmittance along the path segment [mint, maxt]
	 *
	 * The transmittance is defined as 
	 * \f[
	 *      \exp(-\int_{mint}^{maxt} \sigma_t(t) dt)
	 * \f]
	 */
	virtual bool evalTransmittance(const Ray3f &ray) const = 0;

	/**
	 * \brief Return the type of object (i.e. Mesh/Camera/etc.) 
	 * provided by this instance
	 * */
	EClassType getClassType() const { return EMedium; }
};

NORI_NAMESPACE_END

#endif /* __MEDIUM_H */
