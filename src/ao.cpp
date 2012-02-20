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

NORI_NAMESPACE_BEGIN

class AmbientOcclusion : public Integrator {
public:
	AmbientOcclusion(const PropertyList &propList) {
		/* Ray "distance" of the ambient occlusion queries
		   relative to the scene size */
		m_distance = propList.getFloat("distance", 0.05f);
	}

	Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const {
		Intersection its;

		if (!scene->rayIntersect(ray, its))
			return Color3f(1.0f);

		Vector3f d = squareToCosineHemisphere(sampler->next2D());
		Ray ray(its.p, its.shFrame.toWorld(d), Epsilon, m_distance); /// XXX fix maxt

		return Color3f(scene->rayIntersect(ray) ? 1.0f : 0.0f);
	}
private:
	float m_distance;
};

NORI_REGISTER_CLASS(AmbientOcclusion, "ao");
NORI_NAMESPACE_END
