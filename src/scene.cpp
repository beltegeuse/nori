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

#include <nori/scene.h>
#include <nori/bitmap.h>

NORI_NAMESPACE_BEGIN

Scene::Scene(const PropertyList &) {
}

Scene::~Scene() {
	for (size_t i=0; i<m_meshes.size(); ++i)
		delete m_meshes[i];
}

void Scene::activate() {
	cout << "activating scene" << endl;

	Bitmap b(Vector2i(128, 128));
	b.setConstant(Color3f(0.2, 0.4, 0.3));
	b.save("out.exr");

	Bitmap b2("out.exr");
	cout << qPrintable(b2(3,4).toString()) << endl;
}

void Scene::addChild(NoriObject *obj) {
	switch (obj->getClassType()) {
		case EMesh:
			m_meshes.push_back(obj);
			break;

		default:
			throw NoriException(QString("Scene::addChild(<%1>) is not supported!").arg(
				classTypeName(obj->getClassType())));
	}
}

	
QString Scene::toString() const {
	return QString("Scene[]");
}

NORI_REGISTER_CLASS(Scene, "scene");
NORI_NAMESPACE_END
