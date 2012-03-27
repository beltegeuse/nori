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

#include <nori/parser.h>
#include <Eigen/Geometry>
#include <QtGui>
#include <QtXml>
#include <QtXmlPatterns>
#include <stack>

NORI_NAMESPACE_BEGIN

class NoriParser : public QXmlDefaultHandler {
public:
	/// Set of supported XML tags
	enum ETag {
		/* Object classes */
		EScene                = NoriObject::EScene,
		EMesh                 = NoriObject::EMesh,
		EBSDF                 = NoriObject::EBSDF,
		ELuminaire            = NoriObject::ELuminaire,
		ECamera               = NoriObject::ECamera,
		EMedium               = NoriObject::EMedium,
		EPhaseFunction        = NoriObject::EPhaseFunction,
		EIntegrator           = NoriObject::EIntegrator,
		ESampler              = NoriObject::ESampler,
		ETest                 = NoriObject::ETest, 
		EReconstructionFilter = NoriObject::EReconstructionFilter,

		/* Properties */
		EBoolean = NoriObject::EClassTypeCount,
		EInteger,
		EFloat,
		EString,
		EPoint,
		EVector,
		EColor,
		ETransform,
		ETranslate,
		ERotate,
		EScale,
		ELookAt
	};

	NoriParser() : m_root(NULL) {
		/* Mapping from tag names to tag IDs */
		m_tags["scene"]      = EScene;
		m_tags["mesh"]       = EMesh;
		m_tags["bsdf"]       = EBSDF;
		m_tags["luminaire"]  = ELuminaire;
		m_tags["camera"]     = ECamera;
		m_tags["medium"]     = EMedium;
		m_tags["phase"]      = EPhaseFunction;
		m_tags["integrator"] = EIntegrator;
		m_tags["sampler"]    = ESampler;
		m_tags["rfilter"]    = EReconstructionFilter;
		m_tags["test"]       = ETest;
		m_tags["boolean"]    = EBoolean;
		m_tags["integer"]    = EInteger;
		m_tags["float"]      = EFloat;
		m_tags["string"]     = EString;
		m_tags["point"]      = EPoint;
		m_tags["vector"]     = EVector;
		m_tags["color"]      = EColor;
		m_tags["transform"]  = ETransform;
		m_tags["translate"]  = ETranslate;
		m_tags["rotate"]     = ERotate;
		m_tags["scale"]      = EScale;
		m_tags["lookat"]     = ELookAt;
	}

	struct ParserContext {
		QXmlAttributes attr;
		PropertyList propList;
		std::vector<NoriObject *> children;

		inline ParserContext(const QXmlAttributes &attr) : attr(attr) { }
	};

	float parseFloat(const QString &str) const {
		bool success;
		float result = str.toFloat(&success);
		if (!success)
			throw NoriException(QString("Unable to parse floating point value '%1'!").arg(str));
		return result;
	}

	Vector3f parseVector(const QString &str) const {
		QRegExp re("[\\s,]+");
		QStringList list = str.split(re);
		if (list.size() != 3)
			throw NoriException(QString("Cannot parse 3-vector '%1'!").arg(str));
						
		Vector3f result;
		for (int i=0; i<3; ++i)
			result[i] = parseFloat(list[i]);
		return result;
	}

	bool startElement(const QString & /* unused */, const QString &name,
		const QString& /* unused */, const QXmlAttributes &attr) {

		ParserContext ctx(attr);

		if (name == "transform")
			m_transform.setIdentity();
		else if (name == "scene")
			ctx.attr.append("type", "", "type", "scene");

		m_context.push_back(ctx);
		return true;
	}

	bool endElement(const QString & /* unused */, const QString &name,
			const QString& /* unused */) {
		std::map<QString, ETag>::const_iterator it = m_tags.find(name);
		ParserContext &context = m_context.back();

		if (it == m_tags.end())
			throw NoriException(QString("Encountered an unknown tag '%1'!").arg(name));

		int tag = (int) it->second;

		if (tag < NoriObject::EClassTypeCount) {
			/* This is an object, first instantiate it */
			NoriObject *obj = NoriObjectFactory::createInstance(
				context.attr.value("type"),
				context.propList
			);

			if (obj->getClassType() != (int) tag)
				throw NoriException(QString("Unexpectedly constructed an object "
					"of type <%1> (expected type <%2>): %3")
				.arg(NoriObject::classTypeName(obj->getClassType()))
				.arg(NoriObject::classTypeName((NoriObject::EClassType) tag))
				.arg(obj->toString()));

			/* Add all children */
			for (size_t i=0; i<context.children.size(); ++i) {
				obj->addChild(context.children[i]);
				context.children[i]->setParent(obj);
			}

			/* Activate / configure the object */
			obj->activate();

			/* Add it to its parent, if there is one */
			if (m_context.size() >= 2)
				m_context[m_context.size() - 2].children.push_back(obj);
			else
				m_root = obj;
		} else {
			/* This is a property */
			PropertyList &propList = m_context[m_context.size() - 2].propList;
			bool success;

			switch (tag) {
				case EString: {
						propList.setString(context.attr.value("name"),
							context.attr.value("value"));
					}
					break;

				case EInteger: {
						QString value = context.attr.value("value");
						int result = value.toInt(&success);
						if (!success)
							throw NoriException(QString("Unable to parse integer value '%1'!").arg(value));
						propList.setInteger(context.attr.value("name"), result);
					}
					break;

				case EFloat: {
						QString value = context.attr.value("value");
						float result = value.toFloat(&success);
						if (!success)
							throw NoriException(QString("Unable to parse float value '%1'!").arg(value));
						propList.setFloat(context.attr.value("name"), result);
					}
					break;

				case EBoolean: {
						QString value = context.attr.value("value").toLower();
						if (value != "true" && value != "false")
							throw NoriException(QString("Unable to parse boolean value '%1'!").arg(value));
						propList.setBoolean(context.attr.value("name"), value == QString("true"));
					}
					break;

				case EPoint: {
						Vector3f v = parseVector(context.attr.value("value"));
						propList.setPoint(context.attr.value("name"), Point3f(v));
					}
					break;

				case EVector: {
						Vector3f v = parseVector(context.attr.value("value"));
						propList.setVector(context.attr.value("name"), v);
					}
					break;

				case EColor: {
						Vector3f v = parseVector(context.attr.value("value"));
						propList.setColor(context.attr.value("name"), Color3f(v.x(), v.y(), v.z()));
					}
					break;

				case ETransform: {
						propList.setTransform(context.attr.value("name"), 
							Transform(m_transform.matrix()));
					}
					break;

				case ETranslate: {
						Vector3f v = parseVector(context.attr.value("value"));
						m_transform = Eigen::Translation<float, 3>(v.x(), v.y(), v.z()) * m_transform;
					}
					break;

				case EScale: {
						Vector3f v = parseVector(context.attr.value("value"));
						m_transform = Eigen::DiagonalMatrix<float, 3>(v) * m_transform;
					}
					break;

				case ERotate: {
						float angle = degToRad(parseFloat(context.attr.value("angle")));
						Vector3f axis = parseVector(context.attr.value("axis")).normalized();
						m_transform = Eigen::AngleAxis<float>(angle, axis) * m_transform;
					}
					break;

				case ELookAt: {
						Point3f origin = parseVector(context.attr.value("origin"));
						Point3f target = parseVector(context.attr.value("target"));
						Vector3f up     = parseVector(context.attr.value("up")).normalized();

						Vector3f dir = (target - origin).normalized();
						Vector3f left = up.normalized().cross(dir);
						Vector3f newUp = dir.cross(left);

						Eigen::Matrix4f trafo;
						trafo << left, newUp, dir, origin,
							      0, 0, 0, 1;

						m_transform = Eigen::Affine3f(trafo) * m_transform;
					}
					break;

			}
		}

		m_context.pop_back();
		return true;
	}

	inline NoriObject *getRoot() const {
		return m_root;
	}
private:
	std::map<QString, ETag> m_tags;
	std::vector<ParserContext> m_context;
	Eigen::Affine3f m_transform;
	NoriObject *m_root;
};

/// Handle XML schema verification errors
class NoriMessageHandler : public QAbstractMessageHandler {
public:
	void handleMessage(QtMsgType type, const QString &descr, 
			const QUrl &, const QSourceLocation &loc) {
		const char *typeName;
		switch (type) {
			case QtDebugMsg: typeName = "Debug"; break;
			case QtWarningMsg: typeName = "Warning"; break;
			case QtCriticalMsg: typeName = "Critical"; break;
			case QtFatalMsg: 
			default: typeName = "Fatal"; break;	
		}

		/* Convert the HTML error message to plain text */
		QXmlStreamReader xml(descr);
		QString text;
		while (!xml.atEnd())
			if (xml.readNext() == QXmlStreamReader::Characters)
				text += xml.text();

		cerr << typeName << ": " << qPrintable(text);
		if (!loc.isNull())
			cerr << " (line " << loc.line() << ", col " << loc.column() << ")";
		cerr << endl;
	}
};

NoriObject *loadScene(const QString &filename) {
	NoriParser parser;
	QFile schemaFile(":/schema.xsd");
	QXmlSchema schema;

	#if !defined(PLATFORM_WINDOWS)
		/* Fixes number parsing on some machines (notably those with locale ru_RU) */
		setlocale(LC_NUMERIC, "C");
	#endif

	NoriMessageHandler handler;
	schema.setMessageHandler(&handler);
	if (!schemaFile.open(QIODevice::ReadOnly))
		throw NoriException("Unable to open the XML schema!");
	if (!schema.load(schemaFile.readAll()))
		throw NoriException("Unable to parse the XML schema!");

	QXmlSchemaValidator validator(schema);
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly))
		throw NoriException(QString("Unable to open the file \"%1\"").arg(filename));
	if (!validator.validate(&file))
		throw NoriException(QString("Unable to validate the file \"%1\"").arg(filename));

	QXmlInputSource source(&file);
	file.seek(0);
	QXmlSimpleReader reader;
	reader.setContentHandler(&parser);
	if (!reader.parse(source)) 
		throw NoriException(QString("Unable to parse the file \"%1\"").arg(filename));

	return parser.getRoot();
}

NORI_NAMESPACE_END

