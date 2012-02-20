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

#include <nori/kdtree.h>

NORI_NAMESPACE_BEGIN

KDTree::KDTree() : m_primitiveCount(0) {
	m_sizeMap.push_back(0);
}

KDTree::~KDTree() {
	for (size_t i=0; i<m_meshes.size(); ++i)
		delete m_meshes[i];
}

void KDTree::build() {
	SizeType primCount = getPrimitiveCount();
	cout << "Constructing a SAH kd-tree (" << primCount << " triangles, "
		 << getCoreCount() << " threads) .." << endl;
	Parent::buildInternal();
}

void KDTree::addMesh(Mesh *mesh) {
	m_primitiveCount += mesh->getTriangleCount();
	m_meshes.push_back(mesh);
	m_sizeMap.push_back(m_sizeMap.back() + mesh->getTriangleCount());
}

NORI_NAMESPACE_END
