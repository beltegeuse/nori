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

#include <nori/proplist.h>

NORI_NAMESPACE_BEGIN

#define DEFINE_PROPERTY_ACCESSOR(Type, TypeName, XmlName) \
	void PropertyList::set##TypeName(const QString &name, const Type &value) { \
		if (m_properties.find(name) != m_properties.end()) \
			cerr << "Property \"" << qPrintable(name) <<  "\" was specified multiple times!" << endl; \
		m_properties[name] = value; \
	} \
	\
	Type PropertyList::get##TypeName(const QString &name) const { \
		std::map<QString, Property>::const_iterator it = m_properties.find(name); \
		if (it == m_properties.end()) \
			throw NoriException(QString("Property '%1' is missing!").arg(name)); \
		const Type *result = boost::get<Type>(&it->second); \
		if (!result) \
			throw NoriException(QString("Property '%1' has the wrong type! " \
				"(expected <" #XmlName ">)!").arg(name)); \
		return (Type) *result; \
	} \
	\
	Type PropertyList::get##TypeName(const QString &name, const Type &defVal) const { \
		std::map<QString, Property>::const_iterator it = m_properties.find(name); \
		if (it == m_properties.end()) \
			return defVal; \
		const Type *result = boost::get<Type>(&it->second); \
		if (!result) \
			throw NoriException(QString("Property '%1' has the wrong type! " \
				"(expected <" #XmlName ">)!").arg(name)); \
		return (Type) *result; \
	}

DEFINE_PROPERTY_ACCESSOR(bool, Boolean, boolean)
DEFINE_PROPERTY_ACCESSOR(int, Integer, integer)
DEFINE_PROPERTY_ACCESSOR(float, Float, float)
DEFINE_PROPERTY_ACCESSOR(Color3f, Color, color)
DEFINE_PROPERTY_ACCESSOR(Point3f, Point, point)
DEFINE_PROPERTY_ACCESSOR(Vector3f, Vector, vector)
DEFINE_PROPERTY_ACCESSOR(QString, String, string)
DEFINE_PROPERTY_ACCESSOR(Transform, Transform, transform)

NORI_NAMESPACE_END

