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

#include <nori/bsdf.h>
#include <nori/random.h>
#include <boost/math/distributions/students_t.hpp>
#include <QStringList>

NORI_NAMESPACE_BEGIN

class StudentsTTest : public NoriObject {
public:
	StudentsTTest(const PropertyList &propList) {
		/* The null hypothesis will be rejected when the associated 
		   p-value is below the significance level specified here. */
		m_significanceLevel = propList.getFloat("significanceLevel", 0.01f);

		/* This parameter specifies a list of incidence angles that will be tested */
		QString angleString = propList.getString("angles");
		QStringList angles = angleString.split(", ");
		bool success;
	
		for (int i=0; i<angles.size(); ++i) {
			m_angles.push_back(angles[i].toFloat(&success));
			if (!success)
				throw NoriException(QString("Could not parse '%1'").arg(angleString));
		}

		/* This parameter specifies a list of reference values, one for each angle */
		QString referenceString = propList.getString("references");
		QStringList references = referenceString.split(", ");
		for (int i=0; i<references.size(); ++i) {
			m_references.push_back(references[i].toFloat(&success));
			if (!success)
				throw NoriException(QString("Could not parse '%1'").arg(referenceString));
		}

		if (references.size() != angles.size())
			throw NoriException("Specified a different number of angles and reference values!");

		/* Number of BSDF samples that should be generated (default: 100K) */
		m_sampleCount = propList.getInteger("sampleCount", 100000);
	}

	virtual ~StudentsTTest() {
		for (size_t i=0; i<m_bsdfs.size(); ++i)
			delete m_bsdfs[i];
	}

	void addChild(NoriObject *obj) {
		switch (obj->getClassType()) {
			case EBSDF:
				m_bsdfs.push_back(static_cast<BSDF *>(obj));
				break;

			default:
				throw NoriException(QString("StudentsTTest::addChild(<%1>) is not supported!").arg(
					classTypeName(obj->getClassType())));
		}
	}
 
	/// Run the t-test
	void activate() {
		Random *random = new Random();
		int total = 0, passed = 0;

		/* Test each registered BSDF */
		for (size_t k=0; k<m_bsdfs.size(); ++k) {
			const BSDF *bsdf = m_bsdfs[k];
			for (size_t l=0; l<m_angles.size(); ++l) {
				float angle = m_angles[l], reference = m_references[l];

				cout << "------------------------------------------------------" << endl;
				cout << "Testing (angle=" << angle << "): " << qPrintable(bsdf->toString()) << endl;
				++total;

				BSDFQueryRecord bRec(
					sphericalDirection(degToRad(angle), 0)
				);

				cout << "Drawing " << m_sampleCount << " samples .. " << endl;
				double mean=0, variance = 0;
				for (int k=0; k<m_sampleCount; ++k) {
					Point2f sample(random->nextFloat(), random->nextFloat());
					double result = (double) bsdf->sample(bRec, sample).getLuminance();

					/* Numerically robust online variance estimation using an
					   algorithm proposed by Donald Knuth (TAOCP vol.2, 3rd ed., p.232) */
					double delta = result - mean;
					mean += delta / (double) (k+1);
					variance += delta * (result - mean);
				}
				variance /= m_sampleCount - 1;

				/* Compute the t statistic */
				float t = std::abs(mean - reference) * std::sqrt(m_sampleCount / std::max(variance, (double) Epsilon));

				/* Determine the degrees of freedom, and instantiate a matching distribution object */
				int dof = m_sampleCount - 1;
				boost::math::students_t distr(dof);

				cout << "Sample mean = " << mean << " (reference value = " << reference << ")" << endl;
				cout << "Sample variance = " << variance << endl;
				cout << "t-statistic = " << t << " (d.o.f. = " << dof << ")" << endl;

				/* Compute the p-value */
				float pval = (float) (2*boost::math::cdf(boost::math::complement(distr, t)));

				/* Apply the Sidak correction term, since we'll be conducting multiple independent 
				   hypothesis tests. This accounts for the fact that the probability of a failure
				   increases quickly when several hypothesis tests are run in sequence. */
				float alpha = 1.0f - std::pow(1.0f - m_significanceLevel, 1.0f / m_angles.size());
		
				if (pval < alpha) {
					cout << "Rejected the null hypothesis (p-value = " << pval << ", "
						"significance level = " << alpha << ")" << endl;
				} else {
					cout << "Accepted the null hypothesis (p-value = " << pval << ", "
						"significance level = " << alpha << ")" << endl;
					++passed;
				}
				cout << endl;
			}
		}
		cout << "Passed " << passed << "/" << total << " tests." << endl;

		delete random;
	}

	QString toString() const {
		return QString("StudentsTTest[\n"
			"  significanceLevel = %1,\n"
			"  sampleCount= %2\n"
			"]")
			.arg(m_significanceLevel)
			.arg(m_sampleCount);
	}

	EClassType getClassType() const { return ETest; }
private:
	std::vector<BSDF *> m_bsdfs;
	std::vector<float> m_angles;
	std::vector<float> m_references;
	float m_significanceLevel;
	int m_sampleCount;
};

NORI_REGISTER_CLASS(StudentsTTest, "ttest");
NORI_NAMESPACE_END
