/*
    This class contains a implementation of a KD-Tree to compute ray-triangle
	intersections in three dimensions. It was originally part of Mitsuba and 
	slightly adapted for

    Copyright (c) 2007-2012 by Wenzel Jakob

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

#if !defined(__KDTREE_H)
#define __KDTREE_H

#include <nori/gkdtree.h>
#include <nori/mesh.h>

NORI_NAMESPACE_BEGIN

/**
 * \brief Specializes \ref GenericKDTree to a three-dimensional
 * tree that can be used to intersect rays against triangles meshes.
 *
 * The internal tree construction algorithm generates a surface area
 * heuristic (SAH) kd-tree using the "perfect splits" optimization.
 *
 * This class also implements an robust adapted version of the optimized 
 * ray traversal algorithm (TA^B_{rec}), which is explained in Vlastimil 
 * Havran's PhD thesis "Heuristic Ray Shooting Algorithms". 
 *
 * \author Wenzel Jakob
 */
class KDTree : public GenericKDTree<BoundingBox3f, SurfaceAreaHeuristic3, KDTree> {
protected:
	typedef GenericKDTree<BoundingBox3f, SurfaceAreaHeuristic3, KDTree>  Parent;
	typedef Parent::SizeType                                             SizeType;
	typedef Parent::IndexType                                            IndexType;
	typedef Parent::KDNode                                               KDNode;

	using Parent::m_nodes;
	using Parent::m_bbox;
	using Parent::m_indices;

public:
	/// Create a new and empty kd-tree
	KDTree();

	/// Release all memory
	virtual ~KDTree();

	/**
	 * \brief Register a triangle mesh for inclusion in the kd-tree.
	 *
	 * This function can only be used before \ref build() is called
	 */
	void addMesh(Mesh *mesh);

	/// Build the kd-tree
	void build();

	/**
	 * \brief Intersect a ray against all triangle meshes registered
	 * with the kd-tree
	 *
	 * Detailed information about the intersection, if any, will be
	 * stored in the provided \ref Intersection data record. 
	 *
	 * The <tt>shadowRay</tt> parameter specifies whether this detailed
	 * information is really needed. When set to \c true, the 
	 * function just checks whether or not there is occlusion, but without
	 * providing any more detail (i.e. \c its will not be filled with
	 * contents). This is usually much faster.
	 *
	 * \return \c true If an intersection was found
	 */
	bool rayIntersect(const Ray3f &ray, Intersection &its, 
		bool shadowRay = false) const;

	/// Return the total number of internally represented triangles 
	inline SizeType getPrimitiveCount() const { return m_primitiveCount; }

	/// Return the total number of meshes registered with the kd-tree
	inline SizeType getMeshCount() const { return (SizeType) m_meshes.size(); }

	/// Return one of the registered meshes
	inline Mesh *getMesh(IndexType idx) { return m_meshes[idx]; }
	
	/// Return one of the registered meshes (const version)
	inline const Mesh *getMesh(IndexType idx) const { return m_meshes[idx]; }

	//// Return an axis-aligned bounding box containing the entire tree
	inline const BoundingBox3f &getBoundingBox() const {
		return m_bbox;
	}

	//// Return an axis-aligned bounding box containing the given triangle
	inline BoundingBox3f getBoundingBox(IndexType index) const {
		IndexType meshIdx = findMesh(index);
		return m_meshes[meshIdx]->getBoundingBox(index);
	}

	/**
	 * \brief Returns the axis-aligned bounding box of a triangle after it has 
	 * clipped to the extents of another given bounding box.
	 *
	 * See \ref Mesh::getClippedBoundingBox() for details
	 */
	inline BoundingBox3f getClippedBoundingBox(IndexType index, const BoundingBox3f &clip) const {
		IndexType meshIdx = findMesh(index);
		return m_meshes[meshIdx]->getClippedBoundingBox(index, clip);
	}
protected:
	/**
	 * \brief Compute the mesh and triangle indices corresponding to 
	 * a primitive index used by the underlying generic kd-tree implementation. 
	 */
	IndexType findMesh(IndexType &idx) const {
		std::vector<IndexType>::const_iterator it = std::lower_bound(
				m_sizeMap.begin(), m_sizeMap.end(), idx+1) - 1;
		idx -= *it;
		return (IndexType) (it - m_sizeMap.begin());
	}
private:
	std::vector<Mesh *> m_meshes;
	std::vector<SizeType> m_sizeMap;
	SizeType m_primitiveCount;
};

NORI_NAMESPACE_END

#endif /* __KDTREE_H */
