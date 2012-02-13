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
#include <nori/dpdf.h>

NORI_NAMESPACE_BEGIN

/**
 * \brief Triangle mesh
 *
 * This class represents a triangle mesh object, and subclasses
 * implement the specifics of how to create its contents (e.g. 
 * by loading it from an extermal file)
 */
class Mesh : public NoriObject {
public:
	/// Release all memory
	virtual ~Mesh();

	/// Return the total number of triangles in this hsape
	inline uint32_t getTriangleCount() const { return m_triangleCount; }
	
	/// Return the total number of vertices in this hsape
	inline uint32_t getVertexCount() const { return m_vertexCount; }

	/// Return a pointer to the vertex positions
	inline const Point3f *getVertexPositions() const { return m_vertexPositions; }

	/// Return a pointer to the vertex normals (or \c NULL if there are none)
	inline const Normal3f *getVertexNormals() const { return m_vertexNormals; }

	/// Return a pointer to the texture coordinates (or \c NULL if there are none)
	inline const Point2f *getVertexTexCoords() const { return m_vertexTexCoords; }

	/// Return a human-readable summary of this instance
	QString toString() const;

	/**
	 * \brief Return the type of object (i.e. Mesh/BSDF/etc.) 
	 * provided by this instance
	 * */
	EClassType getClassType() const { return EMesh; }
protected:
	Point3f  *m_vertexPositions;
	Normal3f *m_vertexNormals;
	Point2f  *m_vertexTexCoords;
	uint32_t *m_indices;
	uint32_t m_vertexCount;
	uint32_t m_triangleCount;
};

NORI_NAMESPACE_END

#endif /* __MESH_H */
