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

#include <nori/sampler.h>
#include <nori/random.h>

NORI_NAMESPACE_BEGIN

/**
 * Independent sampling - returns independent uniformly distributed
 * random numbers on <tt>[0, 1)x[0, 1)</tt>.
 *
 * This class is essentially just a wrapper around the \ref Random
 * class. For more details on what sample generators do in general,
 * refer to the \ref Sampler class.
 */
class Independent : public Sampler {
public:
	Independent(const PropertyList &propList) {
		m_sampleCount = (size_t) propList.getInteger("sampleCount", 1);
		m_random = new Random();
	}

	virtual ~Independent() {
		delete m_random;
	}

	Sampler *clone() {
		Independent *cloned = new Independent();
		cloned->m_sampleCount = m_sampleCount;
		cloned->m_random = new Random();
		cloned->m_random->seed(m_random);
		return cloned;
	}

	void generate() { /* No-op for this sampler */ }
	void advance()  { /* No-op for this sampler */ }

	float next1D() {
		return m_random->nextFloat();
	}
	
	Point2f next2D() {
		return Point2f(
			m_random->nextFloat(),
			m_random->nextFloat()
		);
	}

	QString toString() const {
		return QString("Independent[sampleCount=%1]").arg(m_sampleCount);
	}
protected:
	Independent() { }
protected:
	Random *m_random;
};

NORI_REGISTER_CLASS(Independent, "independent");
NORI_NAMESPACE_END
