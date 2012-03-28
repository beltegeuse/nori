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
#include <boost/unordered_map.hpp>
#include <QTextStream>
#include <QStringList>
#include <QFile>

NORI_NAMESPACE_BEGIN

/**
 * \brief Loader for Wavefront OBJ triangle meshes
 */
class WavefrontOBJ : public Mesh {
public:
	WavefrontOBJ(const PropertyList &propList) {
		typedef boost::unordered_map<OBJVertex, uint32_t, OBJVertexHash> VertexMap;

		/* Process the OBJ-file line by line */	
		QString filename = propList.getString("filename");
		QFile input(filename);
		if (!input.open(QIODevice::ReadOnly | QIODevice::Text))
			throw NoriException(QString("Cannot open \"%1\"").arg(filename));

		Transform trafo = propList.getTransform("toWorld", Transform());

		cout << "Loading \"" << qPrintable(filename) << "\" .." << endl;
		m_name = filename;

		QTextStream stream(&input);
		QTextStream line;
		QString temp, prefix;

		std::vector<Point3f>   positions;
		std::vector<Point2f>   texcoords;
		std::vector<Normal3f>  normals;
		std::vector<uint32_t>  indices;
		std::vector<OBJVertex> vertices;
		VertexMap vertexMap;

		while (!(temp = stream.readLine()).isNull()) {
			line.setString(&temp);

			line >> prefix;
			if (prefix == "v") {
				Point3f p;
				line >> p.x() >> p.y() >> p.z();
				p = trafo * p;
				positions.push_back(p);
			} else if (prefix == "vt") {
				Point2f tc;
				line >> tc.x() >> tc.y();
				texcoords.push_back(tc);
			} else if (prefix == "vn") {
				Normal3f n;
				line >> n.x() >> n.y() >> n.z();
				n = (trafo * n).normalized();
				normals.push_back(n);
			} else if (prefix == "f") {
				QString v1, v2, v3, v4;
				line >> v1 >> v2 >> v3 >> v4;
				OBJVertex tri[6];
				int nVertices = 3;

				tri[0] = OBJVertex(v1);
				tri[1] = OBJVertex(v2);
				tri[2] = OBJVertex(v3);

				if (!v4.isNull()) {
					/* This is a quad, split into two triangles */
					tri[3] = OBJVertex(v4);
					tri[4] = tri[0];
					tri[5] = tri[2];
					nVertices = 6;
				}

				/* Now convert from the Wavefront OBJ indexing scheme to a good
				   old indexed vertex list (i.e. just one index per vertex) */
				for (int i=0; i<nVertices; ++i) {
					const OBJVertex &v = tri[i];
					VertexMap::const_iterator it = vertexMap.find(v);
					if (it == vertexMap.end()) {
						vertexMap[v] = (uint32_t) vertices.size();
						indices.push_back((uint32_t) vertices.size());
						vertices.push_back(v);
					} else {
						indices.push_back(it->second);
					}
				}
			}
		}

		m_triangleCount = (uint32_t) (indices.size() / 3);
		m_vertexCount = (uint32_t) vertices.size();

		cout << "Read " << m_triangleCount << " triangles and "
			 << m_vertexCount << " vertices." << endl;

		/* Create the compact in-memory representation (i.e. without 
		   unused buffer space). This involves some copying and following
		   of indirections. */

		m_indices = new uint32_t[indices.size()];
		for (size_t i=0; i<indices.size(); ++i)
			m_indices[i] = indices[i];

		m_vertexPositions = new Point3f[m_vertexCount];
		for (size_t i=0; i<m_vertexCount; ++i)
			m_vertexPositions[i] = positions.at(vertices[i].p);

		if (!normals.empty()) {
			m_vertexNormals = new Normal3f[m_vertexCount];
			for (size_t i=0; i<m_vertexCount; ++i)
				m_vertexNormals[i] = normals.at(vertices[i].n);
		}

		if (!texcoords.empty()) {
			m_vertexTexCoords = new Point2f[m_vertexCount];
			for (size_t i=0; i<m_vertexCount; ++i)
				m_vertexTexCoords[i] = texcoords.at(vertices[i].uv);
		}
	}

protected:
	/// Vertex indices used by the OBJ format
	struct OBJVertex {
		uint32_t p, n, uv;

		inline OBJVertex() { }

		OBJVertex(const QString &string) 
			: n((uint32_t) -1), uv((uint32_t) -1) {
			QStringList tokens = string.split("/");

			if (tokens.size() != 1 && tokens.size() != 3)
				goto fail;

			bool ok;
			p  = (uint32_t) tokens[0].toInt(&ok) - 1; if (!ok) goto fail;

			if (tokens.size() == 3) {
				if (tokens[1].length() > 0) {
					uv = (uint32_t) tokens[1].toInt(&ok) - 1; if (!ok) goto fail;
				}
				if (tokens[2].length() > 0) {
					n  = (uint32_t) tokens[2].toInt(&ok) - 1; if (!ok) goto fail;
				}
			}

			return;
		fail:
			throw NoriException(QString("Could not parse vertex data: '%1'!").arg(string));
		}

		inline bool operator==(const OBJVertex &v) const {
			return v.p == p && v.n == n && v.uv == uv;
		}
	};

	/// Hash function for \ref OBJVertex
	struct OBJVertexHash : std::unary_function<OBJVertex, size_t> {
		std::size_t operator()(const OBJVertex &v) const {
			size_t hash = 0;
			boost::hash_combine(hash, v.p);
			boost::hash_combine(hash, v.n);
			boost::hash_combine(hash, v.uv);
			return hash;
		}
	};
};

NORI_REGISTER_CLASS(WavefrontOBJ, "obj");
NORI_NAMESPACE_END
