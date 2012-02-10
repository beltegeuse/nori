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

#if !defined(__TRANSFORM_H)
#define __TRANSFORM_H

#include <nori/common.h>
#include <nori/vector.h>

NORI_NAMESPACE_BEGIN

/**
 * \brief Homogeneous coordinate transformation
 *
 * This class stores a general homogeneous coordinate tranformation, such as
 * rotation, translation, uniform or non-uniform scaling, and perspective
 * transformations. The inverse of this transformation is also recorded
 * here, since it is required when transforming normal vectors.
 */
struct Transform {
public:
	/// Create a new transform instance for the given matrix 
	Transform(const Eigen::Matrix4f &trafo);

	/// Create a new transform instance for the given matrix and its inverse
	Transform(const Eigen::Matrix4f &trafo, const Eigen::Matrix4f &inv) 
		: m_trafo(trafo), m_inverse(inv) { }

	/// Return the underlying matrix
	inline const Eigen::Matrix4f &getMatrix() const {
		return m_trafo;
	}

	/// Return the inverse of the underlying matrix
	inline const Eigen::Matrix4f &getInverseMatrix() const {
		return m_inverse;
	}

private:
	Eigen::Matrix4f m_trafo;
	Eigen::Matrix4f m_inverse;
};

NORI_NAMESPACE_END

#endif /* __TRANSFORM_H */
