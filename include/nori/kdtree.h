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
 * tree that can be used to intersect rays with triangles meshes.
 *
 * The internal tree construction algorithm generates a surface area
 * heuristic (SAH) kd-tree using the "perfect splits" optimization.
 *
 * This class also implements an epsilon-free version of the optimized 
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

	/// Return the total number of internally represented triangles 
	inline SizeType getPrimitiveCount() const { return m_primitiveCount; }

	/// Return the total number of meshes registered with the kd-tree
	inline SizeType getMeshCount() const { return m_meshes.size(); }

	/// Return one of the registered meshes
	inline Mesh *getMesh(IndexType idx) { return m_meshes[idx]; }
	
	/// Return one of the registered meshes (const version)
	inline const Mesh *getMesh(IndexType idx) const { return m_meshes[idx]; }

	//// Return an axis-aligned bounding box containing the given triangle
	inline BoundingBox3f getBoundingBox(uint32_t index) const {
		IndexType meshIdx = findMesh(index);
		return m_meshes[meshIdx]->getBoundingBox(index);
	}

	/**
	 * \brief Returns the axis-aligned bounding box of a triangle after it has 
	 * clipped to the extents of another given bounding box.
	 *
	 * See \ref Mesh::getClippedBoundingBox() for details
	 */
	inline BoundingBox3f getClippedBoundingBox(uint32_t index, const BoundingBox3f &clip) const {
		IndexType meshIdx = findMesh(index);
		return m_meshes[meshIdx]->getClippedBoundingBox(index, clip);
	}
protected:
	/// Ray traversal stack entry used by \ref rayIntersectInternal()
	struct KDStackEntryHavran {
		/* Pointer to the far child */
		const KDNode * __restrict node;
		/* Distance traveled along the ray (entry or exit) */
		float t;
		/* Previous stack item */
		uint32_t prev;
		/* Associated point */
		Point3f p;
	};
#if 0
	/// \brief Robust Havran-style kd-tree traversal loop
	template<bool shadowRay> inline
			bool rayIntersectInternal(const Ray3f &ray, float mint, float maxt, 
			float &t, void *temp) const {
		KDStackEntryHavran stack[NORI_KD_MAXDEPTH];
	
		/* Set up the entry point */
		uint32_t enPt = 0;
		stack[enPt].t = mint;
		stack[enPt].p = ray(mint);
	
		/* Set up the exit point */
		uint32_t exPt = 1;
		stack[exPt].t = maxt;
		stack[exPt].p = ray(maxt);
		stack[exPt].node = NULL;
	
		bool foundIntersection = false;
		const KDNode * __restrict currNode = m_nodes;
		while (currNode != NULL) {
			while (EXPECT_TAKEN(!currNode->isLeaf())) {
				const float splitVal = (float) currNode->getSplit();
				const int axis = currNode->getAxis();
				const KDNode * __restrict farChild;
	
				if (stack[enPt].p[axis] <= splitVal) {
					if (stack[exPt].p[axis] <= splitVal) {
						/* Cases N1, N2, N3, P5, Z2 and Z3 (see thesis) */
						currNode = currNode->getLeft();
						continue;
					}
	
					/* Typo in Havran's thesis:
					   (it specifies "stack[exPt].p == splitVal", which
					    is clearly incorrect) */
					if (stack[enPt].p[axis] == splitVal) {
						/* Case Z1 */
						currNode = currNode->getRight();
						continue;
					}
	
					/* Case N4 */
					currNode = currNode->getLeft();
					farChild = currNode + 1; // getRight()
				} else { /* stack[enPt].p[axis] > splitVal */
					if (splitVal < stack[exPt].p[axis]) {
						/* Cases P1, P2, P3 and N5 */
						currNode = currNode->getRight();
						continue;
					}
					/* Case P4 */
					farChild = currNode->getLeft();
					currNode = farChild + 1; // getRight()
				}
	
				/* Cases P4 and N4 -- calculate the distance to the split plane */
				float distToSplit = (splitVal - ray.o[axis]) * ray.dRcp[axis];
	
				/* Set up a new exit point */
				const uint32_t tmp = exPt++;
				if (exPt == enPt) /* Do not overwrite the entry point */
					++exPt;
	
				stack[exPt].prev = tmp;
				stack[exPt].t = distToSplit;
				stack[exPt].node = farChild;
				stack[exPt].p = ray(distToSplit);
				stack[exPt].p[axis] = splitVal;
			}
	
			/* Reached a leaf node */
			for (IndexType entry=currNode->getPrimStart(),
					last = currNode->getPrimEnd(); entry != last; entry++) {
				const IndexType primIdx = m_indices[entry];
	
				bool result;
				if (!shadowRay)
					result = intersect(ray, primIdx, mint, maxt, t, temp);
				else
					result = intersect(ray, primIdx, mint, maxt);
	
				if (result) {
					if (shadowRay)
						return true;
					maxt = t;
					foundIntersection = true;
				}
			}
	
			if (stack[exPt].t > maxt) 
				break;
	
			/* Pop from the stack and advance to the next node on the interval */
			enPt = exPt;
			currNode = stack[exPt].node;
			exPt = stack[enPt].prev;
		}
	
		return foundIntersection;
	}
#endif

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
