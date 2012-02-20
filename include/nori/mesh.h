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
#include <nori/frame.h>

NORI_NAMESPACE_BEGIN

/**
 * \brief Intersection data structure
 *
 * This data structure records local information about a ray-triangle intersection.
 * This includes the position, traveled ray distance, uv coordinates, as well
 * as well as two local coordinate frames (one that corresponds to the true
 * geometry, and one that is used for shading computations).
 */
struct Intersection {
	/// Position of the surface intersection
	Point3f p;
	/// Unoccluded distance along the ray
	float t;
	/// UV coordinates, if any
	Point2f uv;
	/// Shading frame (based on the shading normal)
	Frame shFrame;
	/// Geometric frame (based on the true geometry)
	Frame geoFrame;
	/// Pointer to the associated mesh
	const Mesh *mesh;
};

/**
 * \brief Triangle mesh
 *
 * This class stores a triangle mesh object and provides numerous functions
 * for querying the individual triangles. Subclasses of \c Mesh implement 
 * the specifics of how to create its contents (e.g. by loading from an 
 * external file)
 */
class Mesh : public NoriObject {
public:
	/// Release all memory
	virtual ~Mesh();
	
	/// Initialize internal data structures (called once by the XML parser)
	virtual void activate();

	/// Return the total number of triangles in this hsape
	inline uint32_t getTriangleCount() const { return m_triangleCount; }
	
	/// Return the total number of vertices in this hsape
	inline uint32_t getVertexCount() const { return m_vertexCount; }

	/**
	 * \brief Uniformly sample a position on the mesh with 
	 * respect to surface area. Returns both position and normal
	 */
	void samplePosition(const Point2f &sample, Point3f &p, Normal3f &n) const;

	/// Return the surface area of the given triangle
	float surfaceArea(uint32_t index) const;

	//// Return an axis-aligned bounding box containing the given triangle
	BoundingBox3f getBoundingBox(uint32_t index) const;

	/**
	 * \brief Returns the axis-aligned bounding box of a triangle after it has 
	 * clipped to the extents of another given bounding box.
	 *
	 * This function uses the Sutherland-Hodgman algorithm to calculate the 
	 * convex polygon that is created when applying all 6 BoundingBox3f splitting 
	 * planes to the triangle. Afterwards, the BoundingBox3f of the newly created 
	 * convex polygon is returned. This function is an important component 
	 * for efficiently creating 'Perfect Split' KD-trees. For more detail, 
	 * see "On building fast kd-Trees for Ray Tracing, and on doing 
	 * that in O(N log N)" by Ingo Wald and Vlastimil Havran
	 */
	BoundingBox3f getClippedBoundingBox(uint32_t index, const BoundingBox3f &clip) const;

	/** \brief Ray-triangle intersection test
	 * 
	 * Uses the algorithm by Moeller and Trumbore discussed at
	 * <tt>http://www.acm.org/jgt/papers/MollerTrumbore97/code.html</tt>.
	 *
	 * \param index
	 *    Index of the triangle that should be intersected
	 * \param ray
	 *    The ray segment to be used for the intersection query
	 * \param t
	 *    Upon success, \a t contains the distance from the ray origin to the
	 *    intersection point,
	 * \param u
	 *   Upon success, \c u will contain the 'U' component of the intersection
	 *   in barycentric coordinates
	 * \param v
	 *   Upon success, \c v will contain the 'V' component of the intersection
	 *   in barycentric coordinates
	 * \return
	 *   \c true if an intersection has been detected
	 */
	bool rayIntersect(uint32_t index, const Ray3f &ray, float &u, float &v, float &t) const;

	/// Return the surface area of the entire mesh
	inline float surfaceArea() const { return m_distr.getSum(); }

	/// Return a pointer to the vertex positions
	inline const Point3f *getVertexPositions() const { return m_vertexPositions; }

	/// Return a pointer to the vertex normals (or \c NULL if there are none)
	inline const Normal3f *getVertexNormals() const { return m_vertexNormals; }

	/// Return a pointer to the texture coordinates (or \c NULL if there are none)
	inline const Point2f *getVertexTexCoords() const { return m_vertexTexCoords; }

	/// Return a pointer to the triangle vertex index list
	inline const uint32_t *getIndices() const { return m_indices; }

	/// Register a child object (e.g. a BSDF) with the mesh
	virtual void addChild(NoriObject *child);

	/// Return a human-readable summary of this instance
	QString toString() const;

	/**
	 * \brief Return the type of object (i.e. Mesh/BSDF/etc.) 
	 * provided by this instance
	 * */
	EClassType getClassType() const { return EMesh; }
protected:
	/// Create an empty mesh
	Mesh();
protected:
	Point3f  *m_vertexPositions;
	Normal3f *m_vertexNormals;
	Point2f  *m_vertexTexCoords;
	uint32_t *m_indices;
	uint32_t m_vertexCount;
	uint32_t m_triangleCount;
	DiscretePDF m_distr;
	BSDF    *m_bsdf;
};

NORI_NAMESPACE_END

#endif /* __MESH_H */
