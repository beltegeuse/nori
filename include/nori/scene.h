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

#if !defined(__SCENE_H)
#define __SCENE_H

#include <nori/kdtree.h>

NORI_NAMESPACE_BEGIN

/**
 * \brief Principal scene data structure
 *
 * This class holds information on scene objects and is responsible for
 * coordinating rendering jobs. It also provides useful query routines that 
 * are mostly used by the \ref Integrator implementations.
 */
class Scene : public NoriObject {
public:
	/// Construct a new scene object
	Scene(const PropertyList &);
	
	/// Release all memory
	virtual ~Scene();

	/**
	 * \brief Inherited from \ref NoriObject::activate()
	 *
	 * Initializes the internal data structures (kd-tree,
	 * luminaire sampling data structures, etc.)
	 */
	void activate();

	/// Add a child object to the scene (meshes, integrators etc.)
	void addChild(NoriObject *obj);

	/// Return a brief string summary of the instance (for debugging purposes)
	QString toString() const;

	EClassType getClassType() const { return EScene; }
private:
	std::vector<NoriObject *> m_meshes;
	KDTree m_kdtree;
};

NORI_NAMESPACE_END

#endif /* __SCENE_H */
