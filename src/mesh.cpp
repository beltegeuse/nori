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

#include <nori/mesh.h>

NORI_NAMESPACE_BEGIN

Mesh::Mesh() : m_vertexPositions(0), m_vertexNormals(0),
  m_vertexTexCoords(0), m_indices(0), m_vertexCount(0),
  m_triangleCount(0) { }

Mesh::~Mesh() {
	delete[] m_vertexPositions;
	if (m_vertexNormals)
		delete[] m_vertexNormals;
	if (m_vertexTexCoords)
		delete[] m_vertexTexCoords;
	delete[] m_indices;
}

QString Mesh::toString() const {
	return QString(
		"Mesh[\n"
		"  vertexCount = %1,\n"
		"  triangleCount = %2\n"
		"]")
	.arg(m_vertexCount)
	.arg(m_triangleCount);
}

NORI_NAMESPACE_END
