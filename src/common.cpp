#include <nori/object.h>
#include <Eigen/Geometry>
#include <Eigen/LU>

NORI_NAMESPACE_BEGIN

Color3f Color3f::toSRGB() const {
	Color3f result;

	for (int i=0; i<3; ++i) {
		float value = coeff(i);

		if (value <= 0.0031308f)
			result[i] = 12.92f * value;
		else
			result[i] = (1.0f + 0.055f) 
				* std::pow(value, 1.0f/2.4f) -  0.055f;
	}

	return result;
}

Color3f Color3f::toLinearRGB() const {
	Color3f result;

	for (int i=0; i<3; ++i) {
		float value = coeff(i);

		if (value <= 0.04045f)
			result[i] = value * (1.0f / 12.92f);
		else
			result[i] = std::pow((value + 0.055f)
				* (1.0f / 1.055f), 2.4f);
	}

	return result;
}

float Color3f::getLuminance() const {
	return coeff(0) * 0.212671f + coeff(1) * 0.715160f + coeff(2) * 0.072169f;
}

Transform::Transform(const Eigen::Matrix4f &trafo) 
	: m_trafo(trafo), m_inverse(trafo.inverse()) { }

Vector3f squareToUniformSphere(const Point2f &sample) {
	float z = 1.0f - 2.0f * sample.y();
	float r = std::sqrt(std::max((float) 0.0f, 1.0f - z*z));
	float sinPhi, cosPhi;
	sincos(2.0f * M_PI * sample.x(), &sinPhi, &cosPhi);
	return Vector3f(r * cosPhi, r * sinPhi, z);
}

Vector3f squareToUniformHemisphere(const Point2f &sample) {
	float z = sample.x();
	float tmp = std::sqrt(std::min((float) 0, 1-z*z));

	float sinPhi, cosPhi;
	sincos(2.0f * M_PI * sample.y(), &sinPhi, &cosPhi);

	return Vector3f(cosPhi * tmp, sinPhi * tmp, z);
}

Vector3f squareToCosineHemisphere(const Point2f &sample) {
	Point2f p = squareToUniformDiskConcentric(sample);
	float z = std::sqrt(std::max((float) 0, 
		1.0f - p.x()*p.x() - p.y()*p.y()));

	return Vector3f(p.x(), p.y(), z);
}

Point2f squareToUniformDisk(const Point2f &sample) {
	float r = std::sqrt(sample.x());
	float sinPhi, cosPhi;
	sincos(2.0f * M_PI * sample.y(), &sinPhi, &cosPhi);

	return Point2f(
		cosPhi * r,
		sinPhi * r
	);
}

Point2f squareToUniformTriangle(const Point2f &sample) {
	float a = std::sqrt(1.0f - sample.x());
	return Point2f(1 - a, a * sample.y());
}

Point2f squareToUniformDiskConcentric(const Point2f &sample) {
	float r1 = 2.0f*sample.x() - 1.0f;
	float r2 = 2.0f*sample.y() - 1.0f;

	Point2f coords;
	if (r1 == 0 && r2 == 0) {
		coords = Point2f(0, 0);
	} else if (r1 > -r2) { /* Regions 1/2 */
		if (r1 > r2)
			coords = Point2f(r1, (M_PI/4.0f) * r2/r1);
		else
			coords = Point2f(r2, (M_PI/4.0f) * (2.0f - r1/r2));
	} else { /* Regions 3/4 */
		if (r1<r2)
			coords = Point2f(-r1, (M_PI/4.0f) * (4.0f + r2/r1));
		else 
			coords = Point2f(-r2, (M_PI/4.0f) * (6.0f - r1/r2));
	}

	Point2f result;
	sincos(coords.y(), &result[1], &result[0]);
	return result*coords.x();
}

Point2f squareToTriangle(const Point2f &sample) {
	float a = std::sqrt(1.0f - sample.x());
	return Point2f(1 - a, a * sample.y());
}

Vector3f sphericalDirection(float theta, float phi) {
	float sinTheta, cosTheta, sinPhi, cosPhi;

	sincos(theta, &sinTheta, &cosTheta);
	sincos(phi, &sinPhi, &cosPhi);

	return Vector3f(
		sinTheta * cosPhi,
		sinTheta * sinPhi,
		cosTheta
	);
}

Point2f sphericalCoordinates(const Vector3f &v) {
	Point2f result(
		std::acos(v.z()),
		std::atan2(v.y(), v.x())
	);
	if (result.y() < 0)
		result.y() += 2*M_PI;
	return result;
}

void coordinateSystem(const Vector3f &a, Vector3f &b, Vector3f &c) {
	if (std::abs(a.x()) > std::abs(a.y())) {
		float invLen = 1.0f / std::sqrt(a.x() * a.x() + a.z() * a.z());
		c = Vector3f(a.z() * invLen, 0.0f, -a.x() * invLen);
	} else {
		float invLen = 1.0f / std::sqrt(a.y() * a.y() + a.z() * a.z());
		c = Vector3f(0.0f, a.z() * invLen, -a.y() * invLen);
	}
	b = c.cross(a);
}

NORI_NAMESPACE_END
