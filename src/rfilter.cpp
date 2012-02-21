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

#include <nori/rfilter.h>

NORI_NAMESPACE_BEGIN

/**
 * Windowed Gaussian filter with configurable extent
 * and standard deviation. Often produces pleasing 
 * results, but may introduce too much blurring.
 */
class GaussianFilter : public ReconstructionFilter {
public:
	GaussianFilter(const PropertyList &propList) {
		/* Half filter size */
		m_radius = propList.getFloat("radius", 2.0f);
		/* Standard deviation of the Gaussian */
		m_stddev = propList.getFloat("stddev", 0.5f);
	}

	float eval(float r) const {
		float alpha = -1.0f / (2.0f * m_stddev*m_stddev);
		return std::max(0.0f, 
			std::exp(alpha * r * r) - 
			std::exp(alpha * m_radius * m_radius));
	}

	QString toString() const {
		return QString("GaussianFilter[radius=%i, stddev=%f]")
			.arg(m_radius).arg(m_stddev);
	}
protected:
	float m_stddev;
};

/**
 * Separable reconstruction filter by Mitchell and Netravali
 * 
 * D. Mitchell, A. Netravali, Reconstruction filters for computer graphics, 
 * Proceedings of SIGGRAPH 88, Computer Graphics 22(4), pp. 221-228, 1988.
 */
class MitchellNetravaliFilter : public ReconstructionFilter {
public:
	MitchellNetravaliFilter(const PropertyList &propList) {
		/* Filter size in pixels */
		m_radius = propList.getFloat("radius", 2.0f);
		/* B parameter from the paper */
		m_B = propList.getFloat("B", 1.0f / 3.0f);
		/* C parameter from the paper */
		m_C = propList.getFloat("C", 1.0f / 3.0f);
	}

	float eval(float r) const {
		r = std::abs(2.0f * r / m_radius);
		float r2 = r*r, r3 = r2*r;

		if (r < 1) {
			return 1.0f/6.0f * ((12-9*m_B-6*m_C)*r3 
					+ (-18+12*m_B+6*m_C) * r2 + (6-2*m_B));
		} else if (r < 2) {
			return 1.0f/6.0f * ((-m_B-6*m_C)*r3 + (6*m_B+30*m_C) * r2
					+ (-12*m_B-48*m_C) * r + (8*m_B + 24*m_C));
		} else {
			return 0.0f;
		}
	}

	QString toString() const {
		return QString("MitchellNetravaliFilter[radius=%i, B=%i, C=%i]")
			.arg(m_radius).arg(m_B).arg(m_C);
	}
protected:
	float m_B, m_C;
};

NORI_REGISTER_CLASS(GaussianFilter, "gaussian");
NORI_REGISTER_CLASS(MitchellNetravaliFilter, "mitchell");

NORI_NAMESPACE_END
