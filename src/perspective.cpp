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

#include <nori/camera.h>
#include <nori/rfilter.h>
#include <Eigen/Geometry>

NORI_NAMESPACE_BEGIN

/**
 * \brief Perspective camera with depth of field
 *
 * This class implements a simple perspective camera model. By default, 
 * it uses an infinitesimally small aperture, creating an infinite depth 
 * of field. Use the <tt>apertureRadius</tt> and <tt>focusDistance</tt>
 * parameters to change this behavior.
 */
class PerspectiveCamera : public Camera {
public:
	PerspectiveCamera(const PropertyList &propList) {
		/* Width and height in pixels. Default: 720p */
		m_size.x() = propList.getInteger("width", 1280);
		m_size.y() = propList.getInteger("height", 720);
		m_invSize = m_size.cast<float>().cwiseInverse();

		/* Specifies an optional camera-to-world transformation. Default: none */
		m_cameraToWorld = propList.getTransform("toWorld", Transform());

		/* Horizontal field of view in degrees */
		m_fov = propList.getFloat("fov", 30.0f);

		/* Denotes the world-space distance from the camera's aperture
		   to the focal plane */
		m_focusDistance = propList.getFloat("focusDistance", 0.0f);

		/* Denotes the radios of the camera in scene units.
		   Default: 0, i.e. no depth of field */
		m_apertureRadius = propList.getFloat("apertureRadius", 0.0f);

		/* Near and far clipping planes in world-space units */
		m_nearClip = propList.getFloat("nearClip", 1e-4f);
		m_farClip = propList.getFloat("nearClip", 1e4f);

		m_rfilter = NULL;
	}


	void activate() {
		float aspect = m_size.x() / (float) m_size.y();

		/* Project vectors in camera space onto a plane at z=1:
		 *
		 *  xProj = cot * x / z
		 *  yProj = cot * y / z
		 *  zProj = (far * (z - near)) / (z * (far-near))
		 *  The cotangent factor ensures that the field of view is 
		 *  mapped to the interval [-1, 1].
		 */
		float recip = 1.0f / (m_farClip - m_nearClip),
		      cot = 1.0f / std::tan(degToRad(m_fov / 2.0f));

		Eigen::Matrix4f perspective;
		perspective <<
			cot, 0,   0,   0,
			0, cot,   0,   0,
			0,   0,   m_farClip * recip, -m_nearClip * m_farClip * recip,
			0,   0,   1,   0;

		/**
		 * Translation and scaling to shift the clip coordinates into the
		 * range from zero to one. Also takes the aspect ratio into account.
		 */
		m_sampleToCamera = Transform( 
			Eigen::DiagonalMatrix<float, 3>(Vector3f(0.5f, -0.5f * aspect, 1.0f)) *
			Eigen::Translation<float, 3>(1.0f, -1.0f/aspect, 0.0f) * perspective).inverse();

		/* If no reconstruction filter was assigned, instantiate a Gaussian filter */
		if (!m_rfilter)
			m_rfilter = static_cast<ReconstructionFilter *>(
				NoriObjectFactory::createInstance("gaussian", PropertyList()));
	}

	Color3f sampleRay(Ray3f &ray,
			const Point2f &samplePosition,
			const Point2f &apertureSample) const {
		Point2f tmp = squareToUniformDiskConcentric(apertureSample)
			* m_apertureRadius;
	
		/* Compute the corresponding position on the 
		   near plane (in local camera space) */
		Point3f nearP = m_sampleToCamera * Point3f(
			samplePosition.x() * m_invSize.x(),
			samplePosition.y() * m_invSize.y(), 0.0f);

		/* Aperture position */
		Point3f apertureP(tmp.x(), tmp.y(), 0.0f);

		/* Sampled position on the focal plane */
		Point3f focusP = nearP * (m_focusDistance / nearP.z());

		/* Turn these into a normalized ray direction, and
		   adjust the ray interval accordingly */
		Vector3f d = (focusP - apertureP).normalized();
		float invZ = 1.0f / d.z();

		ray.o = m_cameraToWorld * apertureP;
		ray.d = m_cameraToWorld * d;
		ray.mint = m_nearClip * invZ;
		ray.maxt = m_farClip * invZ;
		ray.update();

		return Color3f(1.0f);
	}

	void addChild(NoriObject *obj) {
		switch (obj->getClassType()) {
			case EReconstructionFilter:
				if (m_rfilter)
					throw NoriException("Camera: tried to register multiple reconstruction filters!");
				m_rfilter = static_cast<ReconstructionFilter *>(obj);
				break;

			default:
				throw NoriException(QString("Camera::addChild(<%1>) is not supported!").arg(
					classTypeName(obj->getClassType())));
		}
	}

	/// Return a human-readable summary
	QString toString() const {
		return QString(
			"PerspectiveCamera[\n"
			"  cameraToWorld = %1,\n"
			"  size = %2,\n"
			"  fov = %3,\n"
			"  apertureRadius = %4,\n"
			"  focusDistance = %5,\n"
			"  clip = [%6, %7],\n"
			"  rfilter = %8\n"
			"]")
		.arg(indent(m_cameraToWorld.toString(), 18))
		.arg(m_size.toString())
		.arg(m_fov)
		.arg(m_apertureRadius)
		.arg(m_focusDistance)
		.arg(m_nearClip)
		.arg(m_farClip)
		.arg(indent(m_rfilter->toString()));
	}
private:
	Vector2f m_invSize;
	Transform m_sampleToCamera;
	Transform m_cameraToWorld;
	float m_fov;
	float m_apertureRadius;
	float m_focusDistance;
	float m_nearClip;
	float m_farClip;
};

NORI_REGISTER_CLASS(PerspectiveCamera, "perspective");
NORI_NAMESPACE_END
