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

#if !defined(__MESH_H)
#define __MESH_H

#include <nori/object.h>

NORI_NAMESPACE_BEGIN

/**
 * \brief 
 *
 * Nori Superclass of all bidirectional scattering distribution functions
 */
class Mesh : public NoriObject {
public:
	/**
	 * \brief Return the type of object (i.e. Mesh/BSDF/etc.) 
	 * provided by this instance
	 * */
	EClassType getClassType() const { return EMesh; }

private:
	Point3f  *m_vertexPositions;
	Normal3f *m_vertexNormals;
	Point2f  *m_vertexTexCoords;
	uint32_t *m_indices;
	size_t m_vertexCount;
	size_t m_triangleCount;
};

NORI_NAMESPACE_END

#endif /* __MESH_H */
