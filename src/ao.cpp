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

#include <nori/integrator.h>
#include <nori/sampler.h>
#include <nori/scene.h>

NORI_NAMESPACE_BEGIN

/**
 * \brief Ambient occlusion: very simple rendering technique that adds
 * "depth" to renderings by accounting for local shadowing.
 */
class AmbientOcclusion : public Integrator {
public:
	AmbientOcclusion(const PropertyList &propList) {
		/* Ray length of the ambient occlusion queries;
		   expressed relative to the scene size */
		m_length = propList.getFloat("length", 0.05f);
	}

	Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const {
		/* Find the surface that is visible in the requested direction */
		Intersection its;
		cout << "AO:" << qPrintable(ray.toString()) << endl;
		if (!scene->rayIntersect(ray, its))
			return Color3f(1.0f);

		/* Sample a cosine-weighted direction from the hemisphere (local coordinates) */
		Vector3f d = squareToCosineHemisphere(sampler->next2D());

		/* Use the shading frame at "its" to convert it to world coordinates */
		d = its.shFrame.toWorld(d);

		/* Determine the length of the "shadow ray" based on the scene size
		   and the configuration options */
		float length = m_length * scene->getBoundingBox().getExtents().norm();

		/* Create a new outgoing ray having extents (epsilon, length) */
		Ray3f shadowRay(its.p, d, Epsilon, length);

		/* Perform an occlusion test and return one or zero depending on the result */
		return Color3f(scene->rayIntersect(shadowRay) ? 1.0f : 0.0f);
	}

	QString toString() const {
		return QString("AmbientOcclusion[length=%1]").arg(m_length);
	}
private:
	float m_length;
};

NORI_REGISTER_CLASS(AmbientOcclusion, "ao");
NORI_NAMESPACE_END
