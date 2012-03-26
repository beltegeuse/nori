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
#include <nori/phase.h>
#include <nori/quad.h>
#include <nori/random.h>
#include <boost/math/distributions/chi_squared.hpp>
#include <boost/bind.hpp>
#include <fstream>

NORI_NAMESPACE_BEGIN

class ChiSquareTest : public NoriObject {
public:
	ChiSquareTest(const PropertyList &propList) {
		/* The null hypothesis will be rejected when the associated 
		   p-value is below the significance level specified here. */
		m_significanceLevel = propList.getFloat("significanceLevel", 0.01f);

		/* Number of cells along the latitudinal axis. The azimuthal
		   resolution is twice this value. */
		m_thetaResolution = propList.getInteger("resolution", 10);

		/* Minimum expected bin frequency. The chi^2 test does not
		   work reliably when the expected frequency in a cell is
		   low (e.g. less than 5), because normality assumptions
		   break down in this case. Therefore, the implementation
		   will merge such low-frequency cells when they fall below
		   the threshold specified here. */
		m_minExpFrequency = propList.getInteger("minExpFrequency", 5);

		/* Number of samples that should be taken (-1: automatic) */
		m_sampleCount = propList.getInteger("sampleCount", -1);
		
		/* Each provided BSDF will be tested for a few different
		   incident directions. The value specified here determines
		   how many tests will be executed per BSDF */
		m_testCount = propList.getInteger("testCount", 5);

		m_phiResolution = 2 * m_thetaResolution;
		m_frequencies = new float[m_thetaResolution * m_phiResolution];
		m_expFrequencies = new float[m_thetaResolution * m_phiResolution];

		if (m_sampleCount < 0) // ~5K samples per bin
			m_sampleCount = m_thetaResolution * m_phiResolution * 5000;
	}

	virtual ~ChiSquareTest() {
		delete[] m_frequencies;
		delete[] m_expFrequencies;

		for (size_t i=0; i<m_bsdfs.size(); ++i)
			delete m_bsdfs[i];

		for (size_t i=0; i<m_phaseFunctions.size(); ++i)
			delete m_phaseFunctions[i];
	}

	void addChild(NoriObject *obj) {
		switch (obj->getClassType()) {
			case EBSDF:
				m_bsdfs.push_back(static_cast<BSDF *>(obj));
				break;
			
			case EPhaseFunction:
				m_phaseFunctions.push_back(static_cast<PhaseFunction *>(obj));
				break;

			default:
				throw NoriException(QString("ChiSquareTest::addChild(<%1>) is not supported!").arg(
					classTypeName(obj->getClassType())));
		}
	}
 

	/// Execute the chi-square test
	void activate() {
		/* Initialize the cubature code */
		NDIntegrator integrator(1, 2, 100000, 0, 1e-6f);

		/* Create a pseudorandom number generator */
		Random *random = new Random();

		int passed = 0, total = 0;

		/* Test each registered BSDF */
		for (size_t k=0; k<m_bsdfs.size(); ++k) {
			const BSDF *bsdf = m_bsdfs[k];

			/* Run several tests per BSDF to be on the safe side */
			for (int l = 0; l<m_testCount; ++l) {
				cout << "------------------------------------------------------" << endl;
				cout << "Testing: " << qPrintable(bsdf->toString()) << endl;
				++total;

				/* Randomly pick an incident direction on the hemisphere */
				Vector3f wi = squareToCosineHemisphere(
					Point2f(random->nextFloat(), random->nextFloat()));
	
				cout << "Accumulating " << m_sampleCount << " samples into a " << m_thetaResolution 
					 << "x" << m_phiResolution << " contingency table .." << endl;

				float factorTheta = m_thetaResolution / M_PI,
					  factorPhi   = m_phiResolution / (2 * M_PI);
	
				memset(m_frequencies, 0, m_thetaResolution*m_phiResolution*sizeof(float));

				/* Generate many samples from the BSDF and create 
				   a histogram / contingency table */
				BSDFQueryRecord bRec(wi);
				for (int i=0; i<m_sampleCount; ++i) {
					Point2f sample(random->nextFloat(), random->nextFloat());
					Color3f result = bsdf->sample(bRec, sample);

					if ((result.array() == 0).all())
						continue;
	
					Point2f coords = sphericalCoordinates(bRec.wo);
	
					int thetaBin = std::min(std::max(0,
						(int) std::floor(coords.x() * factorTheta)), m_thetaResolution-1);
					int phiBin = std::min(std::max(0,
						(int) std::floor(coords.y() * factorPhi)), m_phiResolution-1);
					m_frequencies[thetaBin * m_phiResolution + phiBin] += 1;
				}
	
				factorTheta = M_PI / m_thetaResolution;
				factorPhi   = 2 * M_PI / m_phiResolution;
	
				/* Numerically integrate the probability density
				   function over rectangles in spherical coordinates.
				   This is done using the 'cubature' library. */
				float *ptr = m_expFrequencies;
				cout << "Integrating expected frequencies .." << endl;
				for (int i=0; i<m_thetaResolution; ++i) {
					double min[2], max[2];
					min[0] = i * factorTheta;
					max[0] = (i+1) * factorTheta;
					for (int j=0; j<m_phiResolution; ++j) {
						min[1] = j * factorPhi;
						max[1] = (j+1) * factorPhi;
						double result, error;

						integrator.integrateVectorized(
							boost::bind(&ChiSquareTest::bsdfIntegrand, bsdf, wi, _1, _2, _3),
							min, max, &result, &error
						);
	
						*ptr++ = result * m_sampleCount;
					}
				}
	
				dump(QString("chi2test_%1.m").arg(total));

				if (runTest())
					++passed;

				cout << endl;
			}
		}

		/* Test each registered phase function */
		for (size_t k=0; k<m_phaseFunctions.size(); ++k) {
			const PhaseFunction *phaseFunction = m_phaseFunctions[k];

			/* Run several tests to be on the safe side */
			for (int l = 0; l<m_testCount; ++l) {
				cout << "------------------------------------------------------" << endl;
				cout << "Testing: " << qPrintable(phaseFunction->toString()) << endl;
				++total;

				/* Randomly pick an incident direction on the sphere */
				Vector3f wi = squareToUniformSphere(
					Point2f(random->nextFloat(), random->nextFloat()));
	
				cout << "Accumulating " << m_sampleCount << " samples into a " << m_thetaResolution 
					 << "x" << m_phiResolution << " contingency table .." << endl;

				float factorTheta = m_thetaResolution / M_PI,
					  factorPhi   = m_phiResolution / (2 * M_PI);
	
				memset(m_frequencies, 0, m_thetaResolution*m_phiResolution*sizeof(float));

				/* Generate many samples from the PhaseFunction and create 
				   a histogram / contingency table */
				PhaseFunctionQueryRecord bRec(wi);
				for (int i=0; i<m_sampleCount; ++i) {
					Point2f sample(random->nextFloat(), random->nextFloat());
					Color3f result = phaseFunction->sample(bRec, sample);

					if ((result.array() == 0).all())
						continue;
	
					Point2f coords = sphericalCoordinates(bRec.wo);
	
					int thetaBin = std::min(std::max(0,
						(int) std::floor(coords.x() * factorTheta)), m_thetaResolution-1);
					int phiBin = std::min(std::max(0,
						(int) std::floor(coords.y() * factorPhi)), m_phiResolution-1);
					m_frequencies[thetaBin * m_phiResolution + phiBin] += 1;
				}
	
				factorTheta = M_PI / m_thetaResolution;
				factorPhi   = 2 * M_PI / m_phiResolution;
	
				/* Numerically integrate the probability density
				   function over rectangles in spherical coordinates.
				   This is done using the 'cubature' library. */
				float *ptr = m_expFrequencies;
				cout << "Integrating expected frequencies .." << endl;
				for (int i=0; i<m_thetaResolution; ++i) {
					double min[2], max[2];
					min[0] = i * factorTheta;
					max[0] = (i+1) * factorTheta;
					for (int j=0; j<m_phiResolution; ++j) {
						min[1] = j * factorPhi;
						max[1] = (j+1) * factorPhi;
						double result, error;

						integrator.integrateVectorized(
							boost::bind(&ChiSquareTest::phaseFunctionIntegrand, phaseFunction, wi, _1, _2, _3),
							min, max, &result, &error
						);
	
						*ptr++ = result * m_sampleCount;
					}
				}
	
				dump(QString("chi2test_%1.m").arg(total));

				if (runTest())
					++passed;

				cout << endl;
			}
		}

		cout << "Passed " << passed << "/" << total << " tests." << endl;

		delete random;
	}

	/// Internally used data structure for sorting cells by exp. frequency
	struct Cell {
		float expFrequency;
		size_t index;
	};

	bool runTest() {
		/* Sort all cells by their expected frequencies */
		std::vector<Cell> cells(m_thetaResolution*m_phiResolution);
		for (size_t i=0; i<cells.size(); ++i) {
			cells[i].expFrequency = m_expFrequencies[i];
			cells[i].index = i;
		}

		/* Some boost lambda magic .. */
		std::sort(cells.begin(), cells.end(), 
			boost::bind(&Cell::expFrequency, _1) < boost::bind(&Cell::expFrequency, _2));

		/* Compute the Chi^2 statistic and pool cells as necessary */
		float pooledFrequencies = 0, pooledExpFrequencies = 0, chsq = 0;
		int pooledCells = 0, dof = 0;

		for (std::vector<Cell>::iterator it = cells.begin();
				it != cells.end(); ++it) {

			size_t index = it->index;
			if (m_expFrequencies[index] == 0) {
				if (m_frequencies[index] > m_sampleCount * 1e-5f) {
					/* Uh oh: samples in a cell that should be completely empty
					   according to the probability density function. Ordinarily,
					   even a single sample requires immediate rejection of the null 
					   hypothesis. But due to finite-precision computations and rounding 
					   errors, this can occasionally happen without there being an 
					   actual bug. Therefore, the criterion here is a bit more lenient. */

					cout << "Encountered " << m_frequencies[index] << " samples in a cell "
						 << "with expected frequency 0. Rejecting the null hypothesis!" << endl;
					return false;
				}
			} else if (m_expFrequencies[index] < m_minExpFrequency) {
				/* Pool cells with low expected frequencies */
				pooledFrequencies += m_frequencies[index];
				pooledExpFrequencies += m_expFrequencies[index];
				pooledCells++;
			} else if (pooledExpFrequencies > 0 && pooledExpFrequencies < m_minExpFrequency) {
				/* Keep on pooling cells until a sufficiently high 
				   expected frequency is achieved. */
				pooledFrequencies += m_frequencies[index];
				pooledExpFrequencies += m_expFrequencies[index];
				pooledCells++;
			} else {
				float diff = m_frequencies[index] - m_expFrequencies[index];
				chsq += (diff*diff) / m_expFrequencies[index];
				++dof;
			}
		}

		if (pooledExpFrequencies > 0 || pooledFrequencies > 0) {
			cout << "Pooled " << pooledCells << " to ensure sufficiently high expected "
				"cell frequencies (>" << m_minExpFrequency << ")" << endl;
			float diff = pooledFrequencies - pooledExpFrequencies;
			chsq += (diff*diff) / pooledExpFrequencies;
			++dof;
		}

		/* All parameters are assumed to be known, so there is no
		   additional DF reduction due to model parameters */
		dof -= 1;

		if (dof <= 0) {
			cout << "The number of degrees of freedom (" << dof << ") is too low!" << endl;
			return false;
		}

		cout << "Chi-square statistic = " << chsq << " (d.o.f. = " << dof << ")" << endl;
	
		/* Probability of obtaining a test statistic at least
		   as extreme as the one observed under the assumption
		   that the distributions match */
		float pval;

		try {
			boost::math::chi_squared chSqDist(dof);
			pval = 1 - (float) boost::math::cdf(chSqDist, chsq);
		} catch (const std::exception &ex) {
			cout << "Encountered an internal error during the p-value computation: " << ex.what() << endl;
			return false;
		}

		/* Apply the Sidak correction term, since we'll be conducting multiple independent 
		   hypothesis tests. This accounts for the fact that the probability of a failure
		   increases quickly when several hypothesis tests are run in sequence. */
		float alpha = 1.0f - std::pow(1.0f - m_significanceLevel, 1.0f / (m_testCount * 
			(m_bsdfs.size() + m_phaseFunctions.size())));

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

	/**
	 * \brief Dump the observed and expected frequencies 
	 * as a MATLAB source file for analysis
	 */
	void dump(const QString &filename) {
		std::ofstream out(filename.toStdString().c_str());

		cout << "Writing current state to " << qPrintable(filename) << endl;

		out << "frequencies = [ ";
		for (int i=0; i<m_thetaResolution; ++i) {
			for (int j=0; j<m_phiResolution; ++j) {
				out << m_frequencies[i*m_phiResolution+j];
				if (j+1 < m_phiResolution)
					out << ", ";
			}
			if (i+1 < m_thetaResolution)
				out << "; ";
		}
		out << " ];" << endl
			<< "expFrequencies = [ ";
		for (int i=0; i<m_thetaResolution; ++i) {
			for (int j=0; j<m_phiResolution; ++j) {
				out << m_expFrequencies[i*m_phiResolution+j];
				if (j+1 < m_phiResolution)
					out << ", ";
			}
			if (i+1 < m_thetaResolution)
				out << "; ";
		}
		out << " ];" << endl
			<< "colormap(jet);" << endl
			<< "clf; subplot(2,1,1);" << endl
			<< "imagesc(frequencies);" << endl
			<< "title('Observed frequencies');" << endl
			<< "axis equal;" << endl
			<< "subplot(2,1,2);" << endl
			<< "imagesc(expFrequencies);" << endl
			<< "axis equal;" << endl
			<< "title('Expected frequencies');" << endl;
		out.close();
	}

	QString toString() const {
		return QString("ChiSquareTest[\n"
			"  thetaResolution = %1,\n"
			"  phiResolution = %2,\n"
			"  minExpFrequency = %3,\n"
			"  sampleCount = %4,\n"
			"  testCount = %5,\n"
			"  significanceLevel = %6\n"
			"]")
			.arg(m_thetaResolution)
			.arg(m_phiResolution)
			.arg(m_minExpFrequency)
			.arg(m_sampleCount)
			.arg(m_testCount)
			.arg(m_significanceLevel);
	}

	EClassType getClassType() const { return ETest; }
private:
	/// Functor to evaluate the pdf values in parallel using OpenMP
	static void bsdfIntegrand(const BSDF *bsdf,
		const Vector3f &wi, size_t nPts, const double *in, double *out) {
		// Disabling for now, this is actually slower in some cases!
		// #pragma omp parallel for
		for (int i=0; i<(int) nPts; ++i) {
			/* The quadrature code runs in double precision, so some extra
			   conversions are required */
			Vector3f wo = sphericalDirection((float) in[2*i], (float) in[2*i+1]);
			BSDFQueryRecord bRec(wi, wo, ESolidAngle);
			out[i] = (double) (bsdf->pdf(bRec) * std::sin((float) in[2*i]));
		}
	}

	static void phaseFunctionIntegrand(const PhaseFunction *phase,
		const Vector3f &wi, size_t nPts, const double *in, double *out) {
		// Disabling for now, this is actually slower in some cases!
		for (int i=0; i<(int) nPts; ++i) {
			/* The quadrature code runs in double precision, so some extra
			   conversions are required */
			Vector3f wo = sphericalDirection((float) in[2*i], (float) in[2*i+1]);
			PhaseFunctionQueryRecord bRec(wi, wo);
			out[i] = (double) (phase->pdf(bRec) * std::sin((float) in[2*i]));
		}
	}
private:
	int m_thetaResolution, m_phiResolution;
	int m_minExpFrequency;
	int m_sampleCount;
	int m_testCount;
	float *m_frequencies;
	float *m_expFrequencies;
	float m_significanceLevel;
	std::vector<BSDF *> m_bsdfs;
	std::vector<PhaseFunction *> m_phaseFunctions;
};

NORI_REGISTER_CLASS(ChiSquareTest, "chi2test");
NORI_NAMESPACE_END
