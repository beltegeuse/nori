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

#include <nori/random.h>
#include <nori/scene.h>
#include <nori/bsdf.h>
#include <nori/camera.h>
#include <nori/integrator.h>
#include <nori/sampler.h>
#include <boost/math/distributions/students_t.hpp>
#include <QStringList>

NORI_NAMESPACE_BEGIN

/**
 * Student's t-test for the equality of means
 *
 * This test analyzes whether the expected value of a random variable matches a 
 * certain pre-specified value. When there is significant statistical "evidence" 
 * against this hypothesis, the test fails.
 *
 * This is useful in checking whether a Monte Carlo method method converges 
 * against the right value. Because statistical tests are able to handle the
 * inherent noise of these methods, they can be used to construct statistical 
 * test suites not unlike the traditional unit tests used in software engineering.
 *
 * This implementation can be used to test two things:
 *
 * 1. that the illumination scattered by a BRDF model under uniform illumination
 *    into a certain direction matches a given value (modulo noise).
 * 
 * 2. that the average radiance received by a camera within some scene
 *    matches a given value (modulo noise).
 */
class StudentsTTest : public NoriObject {
public:
	StudentsTTest(const PropertyList &propList) {
		/* The null hypothesis will be rejected when the associated 
		   p-value is below the significance level specified here. */
		m_significanceLevel = propList.getFloat("significanceLevel", 0.01f);

		/* This parameter specifies a list of incidence angles that will be tested */
		QString angleString = propList.getString("angles", "");
		QRegExp re("[\\s,]+");
		QStringList angles = angleString.split(re);
		bool success;
	
		for (int i=0; i<angles.size(); ++i) {
			if (angles[i] == "")
				continue;
			m_angles.push_back(angles[i].toFloat(&success));
			if (!success)
				throw NoriException(QString("Could not parse '%1'").arg(angles[i]));
		}

		/* This parameter specifies a list of reference values, one for each angle */
		QString referenceString = propList.getString("references");
		QStringList references = referenceString.split(re);
		for (int i=0; i<references.size(); ++i) {
			if (references[i].isNull()) 
				continue;
			m_references.push_back(references[i].toFloat(&success));
			if (!success)
				throw NoriException(QString("Could not parse '%1'").arg(referenceString));
		}

		/* Number of BSDF samples that should be generated (default: 100K) */
		m_sampleCount = propList.getInteger("sampleCount", 100000);
	}

	virtual ~StudentsTTest() {
		for (size_t i=0; i<m_bsdfs.size(); ++i)
			delete m_bsdfs[i];
		for (size_t i=0; i<m_scenes.size(); ++i)
			delete m_scenes[i];
	}

	void addChild(NoriObject *obj) {
		switch (obj->getClassType()) {
			case EBSDF:
				m_bsdfs.push_back(static_cast<BSDF *>(obj));
				break;
			
			case EScene:
				m_scenes.push_back(static_cast<Scene *>(obj));
				break;

			default:
				throw NoriException(QString("StudentsTTest::addChild(<%1>) is not supported!").arg(
					classTypeName(obj->getClassType())));
		}
	}

	/// Conduct a two-sided t-test
	bool ttest(double mean, double variance, double reference) {
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
		float alpha = 1.0f - std::pow(1.0f - m_significanceLevel, 1.0f / m_references.size());

		if (pval < alpha) {
			cout << "Rejected the null hypothesis (p-value = " << pval << ", "
				"significance level = " << alpha << ")" << endl;
			return false;
		} else {
			cout << "Accepted the null hypothesis (p-value = " << pval << ", "
				"significance level = " << alpha << ")" << endl;
			return true;
		}
	}

	/// Invoke a series of t-tests on the provided input
	void activate() {
		Random *random = new Random();
		int total = 0, passed = 0;

		if (!m_bsdfs.empty()) {
			if (m_references.size() != m_angles.size())
				throw NoriException("Specified a different number of angles and reference values!");
			if (!m_scenes.empty())
				throw NoriException("Cannot test BSDFs and scenes at the same time!");

			/* Test each registered BSDF */
			for (size_t k=0; k<m_bsdfs.size(); ++k) {
				const BSDF *bsdf = m_bsdfs[k];
				for (size_t l=0; l<m_references.size(); ++l) {
					float angle = m_angles[l], reference = m_references[l];

					cout << "------------------------------------------------------" << endl;
					cout << "Testing (angle=" << angle << "): " << qPrintable(bsdf->toString()) << endl;
					++total;

					BSDFQueryRecord bRec(sphericalDirection(degToRad(angle), 0));

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
					if (ttest(mean, variance, reference))
						++passed;
					cout << endl;
				}
			}
		} else {
			if (m_references.size() != m_scenes.size())
				throw NoriException("Specified a different number of scenes and reference values!");

			Sampler *sampler = static_cast<Sampler *>(
				NoriObjectFactory::createInstance("independent", PropertyList()));
	
			for (size_t k=0; k<m_scenes.size(); ++k) {
				const Scene *scene = m_scenes[k];
				const Integrator *integrator = scene->getIntegrator();
				const Camera *camera = scene->getCamera();
				float reference = m_references[k];

				cout << "------------------------------------------------------" << endl;
				cout << "Testing scene: " << qPrintable(scene->toString()) << endl;
				++total;

				cout << "Generating " << m_sampleCount << " paths.. " << endl;

				double mean=0, variance = 0;
				for (int k=0; k<m_sampleCount; ++k) {
					/* Sample a ray from the camera */
					Ray3f ray;
					Point2f pixelSample = (sampler->next2D().array() 
						* camera->getOutputSize().cast<float>().array()).matrix();
					Color3f value = camera->sampleRay(ray, pixelSample, sampler->next2D());
					/* Compute the incident radiance */
					value *= integrator->Li(scene, sampler, ray);

					/* Numerically robust online variance estimation using an
					   algorithm proposed by Donald Knuth (TAOCP vol.2, 3rd ed., p.232) */
					double result = (double) value.getLuminance();
					double delta = result - mean;
					mean += delta / (double) (k+1);
					variance += delta * (result - mean);
				}
				variance /= m_sampleCount - 1;
				if (ttest(mean, variance, reference))
					++passed;
				cout << endl;
			}
		}
		cout << "Passed " << passed << "/" << total << " tests." << endl;

		delete random;
	}

	QString toString() const {
		return QString(
			"StudentsTTest[\n"
			"  significanceLevel = %1,\n"
			"  sampleCount= %2\n"
			"]")
			.arg(m_significanceLevel)
			.arg(m_sampleCount);
	}

	EClassType getClassType() const { return ETest; }
private:
	std::vector<BSDF *> m_bsdfs;
	std::vector<Scene *> m_scenes;
	std::vector<float> m_angles;
	std::vector<float> m_references;
	float m_significanceLevel;
	int m_sampleCount;
};

NORI_REGISTER_CLASS(StudentsTTest, "ttest");
NORI_NAMESPACE_END
