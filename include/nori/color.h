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

#if !defined(__COLOR_H)
#define __COLOR_H

#include <nori/common.h>

NORI_NAMESPACE_BEGIN

/**
 * \brief Represents a linear RGB color value
 */
struct Color3f : public Eigen::Array3f {
public:
	typedef Eigen::Array3f Base;

	/// Initialize the color vector with a uniform value
	inline Color3f(float value = 0) : Base(value, value, value) { }

	/// Initialize the color vector with specific per-channel values
	inline Color3f(float r, float g, float b) : Base(r, g, b) { }

	/// Construct a color vector from ArrayBase (needed to play nice with Eigen)
	template <typename Derived> inline Color3f(const Eigen::ArrayBase<Derived>& p) 
		: Base(p) { }

	/// Assign a color vector from ArrayBase (needed to play nice with Eigen)
    template <typename Derived> Color3f &operator=(const Eigen::ArrayBase<Derived>& p) {
		this->Base::operator=(p);
		return *this;
    }

	/// Return a reference to the red channel
	inline float &r() { return x(); }
	/// Return a reference to the red channel (const version)
	inline const float &r() const { return x(); }
	/// Return a reference to the green channel
	inline float &g() { return y(); }
	/// Return a reference to the green channel (const version)
	inline const float &g() const { return y(); }
	/// Return a reference to the blue channel
	inline float &b() { return z(); }
	/// Return a reference to the blue channel (const version)
	inline const float &b() const { return z(); }

	/// Clamp to the positive range
	inline Color3f clamp() const { return Color3f(std::max(r(), 0.0f),
		std::max(g(), 0.0f), std::max(b(), 0.0f)); }

	/// Check if the color vector contains a NaN/Inf/negative value
	bool isValid() const;

	/// Convert from sRGB to linear RGB
	Color3f toLinearRGB() const;

	/// Convert from linear RGB to sRGB
	Color3f toSRGB() const;

	/// Return the associated luminance
	float getLuminance() const;

	/// Return a human-readable string summary
	inline QString toString() const {
		return QString("[%1, %2, %3]").arg(coeff(0)).arg(coeff(1)).arg(coeff(2));
	}
};

/**
 * \brief Represents a linear RGB color and a weight
 *
 * This is used by Nori's image reconstruction filter code
 */
struct Color4f : public Eigen::Array4f {
public:
	typedef Eigen::Array4f Base;

	/// Create an zero value
	inline Color4f() : Base(0.0f, 0.0f, 0.0f, 0.0f) { }

	/// Create from a 3-channel color
	inline Color4f(const Color3f &c) : Base(c.r(), c.g(), c.b(), 1.0f) { }

	/// Initialize the color vector with specific per-channel values
	inline Color4f(float r, float g, float b, float w) : Base(r, g, b, w) { }

	/// Construct a color vector from ArrayBase (needed to play nice with Eigen)
	template <typename Derived> inline Color4f(const Eigen::ArrayBase<Derived>& p) 
		: Base(p) { }

	/// Assign a color vector from ArrayBase (needed to play nice with Eigen)
    template <typename Derived> Color4f &operator=(const Eigen::ArrayBase<Derived>& p) {
		this->Base::operator=(p);
		return *this;
    }

	/// Normalize and convert into a \ref Color3f value 
	inline Color3f normalized() const {
		if (EXPECT_TAKEN(w() != 0))
			return head<3>() / w();
		else
			return Color3f(0.0f);
	}

	/// Return a human-readable string summary
	inline QString toString() const {
		return QString("[%1, %2, %3, %4]").arg(coeff(0)).arg(coeff(1)).arg(coeff(2)).arg(coeff(4));
	}
};

NORI_NAMESPACE_END

#endif /* __COLOR_H */

