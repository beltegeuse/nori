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

#if !defined(__PROPLIST_H)
#define __PROPLIST_H

#include <nori/color.h>
#include <nori/transform.h>
#include <boost/variant.hpp>
#include <map>

NORI_NAMESPACE_BEGIN

/**
 * \brief This is a sssociative container used to supply the constructors
 * of \ref NoriObject subclasses with parameter information.
 */
class PropertyList {
public:
	PropertyList() { }

	/// Set a boolean property
	void setBoolean(const QString &name, const bool &value);
	
	/// Get a boolean property, and throw an exception if it does not exist
	bool getBoolean(const QString &name) const;

	/// Get a boolean property, and use a default value if it does not exist
	bool getBoolean(const QString &name, const bool &defaultValue) const;

	/// Set an integer property
	void setInteger(const QString &name, const int &value);
	
	/// Get an integer property, and throw an exception if it does not exist
	int getInteger(const QString &name) const;

	/// Get am integer property, and use a default value if it does not exist
	int getInteger(const QString &name, const int &defaultValue) const;

	/// Set a float property
	void setFloat(const QString &name, const float &value);
	
	/// Get a float property, and throw an exception if it does not exist
	float getFloat(const QString &name) const;

	/// Get a float property, and use a default value if it does not exist
	float getFloat(const QString &name, const float &defaultValue) const;

	/// Set a string property
	void setString(const QString &name, const QString &value);

	/// Get a string property, and throw an exception if it does not exist
	QString getString(const QString &name) const;

	/// Get a string property, and use a default value if it does not exist
	QString getString(const QString &name, const QString &defaultValue) const;

	/// Set a color property
	void setColor(const QString &name, const Color3f &value);

	/// Get a color property, and throw an exception if it does not exist
	Color3f getColor(const QString &name) const;

	/// Get a color property, and use a default value if it does not exist
	Color3f getColor(const QString &name, const Color3f &defaultValue) const;

	/// Set a point property
	void setPoint(const QString &name, const Point3f &value);

	/// Get a point property, and throw an exception if it does not exist
	Point3f getPoint(const QString &name) const;

	/// Get a point property, and use a default value if it does not exist
	Point3f getPoint(const QString &name, const Point3f &defaultValue) const;

	/// Set a vector property
	void setVector(const QString &name, const Vector3f &value);

	/// Get a vector property, and throw an exception if it does not exist
	Vector3f getVector(const QString &name) const;

	/// Get a vector property, and use a default value if it does not exist
	Vector3f getVector(const QString &name, const Vector3f &defaultValue) const;

	/// Set a transform property
	void setTransform(const QString &name, const Transform &value);

	/// Get a transform property, and throw an exception if it does not exist
	Transform getTransform(const QString &name) const;

	/// Get a transform property, and use a default value if it does not exist
	Transform getTransform(const QString &name, const Transform &defaultValue) const;
private:
	typedef boost::variant<bool, int, float, QString, 
		Color3f, Point3f, Transform> Property;

	std::map<QString, Property> m_properties;
};

NORI_NAMESPACE_END

#endif /* __PROPLIST_H */
