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

#include <nori/medium.h>
#include <QFile>
#include <QDataStream>

#if defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS)
#include <sys/mman.h>
#include <fcntl.h>
#endif

#if defined(PLATFORM_WINDOWS)
#include <windows.h>
#endif

NORI_NAMESPACE_BEGIN

/**
 * \brief Heterogeneous participating medium class. The implementation 
 * fetches density values from an external file that is mapped into memory.
 *
 * The density values in the file are interpreted as the extinction
 * coefficient sigma_t. The scattering albedo is assumed to be constant
 * throughout the volume, and is given as a parameter to this class.
 */
class HeterogeneousMedium : public Medium {
public:
	HeterogeneousMedium(const PropertyList &propList) {
		// Denotes the scattering albedo
		m_albedo = propList.getColor("albedo");

		// An (optional) transformation that converts between medium and world coordinates
		m_worldToMedium = propList.getTransform("toWorld", Transform()).inverse();

		// Optional multiplicative factor that will be applied to all density values in the file
		m_densityMultiplier = propList.getFloat("densityMultiplier", 1.0f);

		m_filename = propList.getString("filename");
		QByteArray filename = m_filename.toLocal8Bit();
		QFile file(m_filename);

		if (!file.exists())
			throw NoriException(QString("The file \"%1\" does not exist!").arg(m_filename));

		/* Parse the file header */
		file.open(QIODevice::ReadOnly);
		QDataStream stream(&file);
		stream.setByteOrder(QDataStream::LittleEndian);

		qint8 header[3], version; qint32 type;
		stream >> header[0] >> header[1] >> header[2] >> version >> type;

		if (memcmp(header, "VOL", 3) != 0 || version != 3)
			throw NoriException("This is not a valid volume data file!");

		stream >> m_resolution.x() >> m_resolution.y() >> m_resolution.z();
		file.close();

		cout << "Mapping \"" << filename.data() << "\" (" << m_resolution.x()
			<< "x" << m_resolution.y() << "x" << m_resolution.z() << ") into memory .." << endl;

		m_fileSize = (size_t) file.size();
		#if defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS)
			int fd = open(filename.data(), O_RDONLY);
			if (fd == -1)
				throw NoriException(QString("Could not open \"%1\"!").arg(m_filename));
			m_data = (float *) mmap(NULL, m_fileSize, PROT_READ, MAP_SHARED, fd, 0);
			if (m_data == NULL)
				throw NoriException("mmap(): failed.");
			if (close(fd) != 0)
				throw NoriException("close(): unable to close file descriptor!");
		#elif defined(PLATFORM_WINDOWS)
			m_file = CreateFileA(filename.data(), GENERIC_READ, 
				FILE_SHARE_READ, NULL, OPEN_EXISTING, 
				FILE_ATTRIBUTE_NORMAL, NULL);
			if (m_file == INVALID_HANDLE_VALUE)
				throw NoriException(QString("Could not open \"%1\"!").arg(m_filename));
			m_fileMapping = CreateFileMapping(m_file, NULL, PAGE_READONLY, 0, 0, NULL);
			if (m_fileMapping == NULL)
				throw NoriException("CreateFileMapping(): failed.");
			m_data = (float *) MapViewOfFile(m_fileMapping, FILE_MAP_READ, 0, 0, 0);
			if (m_data == NULL)
				throw NoriException("MapViewOfFile(): failed.");
		#endif

		m_data += 12; // Shift past the header
	}

	virtual ~HeterogeneousMedium() {
		if (m_data) {
			m_data -= 12;

			cout << "Unmapping \"" << qPrintable(m_filename) << "\" from memory.." << endl;
			#if defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS)
				int retval = munmap(m_data, m_fileSize);
				if (retval != 0)
					throw NoriException("munmap(): unable to unmap memory!");
			#elif defined(PLATFORM_WINDOWS)
				if (!UnmapViewOfFile(m_data))
					throw NoriException("UnmapViewOfFile(): unable to unmap memory region");
				if (!CloseHandle(m_fileMapping))
					throw NoriException("CloseHandle(): unable to close file mapping!");
				if (!CloseHandle(m_file))
					throw NoriException("CloseHandle(): unable to close file");
			#endif
		}
	}

	/**
	 * \brief Evaluate sigma_t(p), where 'p' is given in local coordinates
	 *
	 * You may assume that the maximum value returned by this function is
	 * equal to 'm_densityMultiplier'
	 */
	float lookupSigmaT(const Point3f &_p) const {
		Point3f p  = _p.cwiseProduct(m_resolution.cast<float>()),
				pf = Point3f(std::floor(p.x()), std::floor(p.y()), std::floor(p.z()));
		Point3i p0 = pf.cast<int>();

		if ((p0.array() < 0).any() || (p0.array() >= m_resolution.array() - 1).any())
			return 0.0f;

		size_t row    = m_resolution.x(),
		       slab   = row * m_resolution.y(),
		       offset = p0.z()*slab + p0.y()*row + p0.x();

		const float
			d000 = m_data[offset],
			d001 = m_data[offset + 1],
			d010 = m_data[offset + row],
			d011 = m_data[offset + row + 1],
			d100 = m_data[offset + slab],
			d101 = m_data[offset + slab + 1],
			d110 = m_data[offset + slab + row],
			d111 = m_data[offset + slab + row + 1];

		Vector3f w1 = p-pf, w0 = (1 - w1.array()).matrix();

		/* Trilinearly interpolate */
		return (((d000 * w0.x() + d001 * w1.x()) * w0.y() +
		         (d010 * w0.x() + d011 * w1.x()) * w1.y()) * w0.z() +
		        ((d100 * w0.x() + d101 * w1.x()) * w0.y() +
		         (d110 * w0.x() + d111 * w1.x()) * w1.y()) * w1.z()) * m_densityMultiplier;
	}

	bool sampleDistance(const Ray3f &_ray, Sampler *sampler, float &t, Color3f &weight) const {
		/* Transform the ray into the local coordinate system */
		Ray3f ray = m_worldToMedium * _ray;

		throw NoriException("HeterogeneousMedium::sampleDistance(): not implemented!");
	}

	Color3f evalTransmittance(const Ray3f &_ray, Sampler *sampler) const {
		/* Transform the ray into the local coordinate system */
		Ray3f ray = m_worldToMedium * _ray;

		throw NoriException("HeterogeneousMedium::evalTransmittance(): not implemented!");
	}

	/// Return a human-readable summary
	QString toString() const {
		return QString(
			"HeterogeneousMedium[\n"
			"  filename = \"%1\",\n"
			"  densityMultiplier = %2,\n"
			"  albedo = %3\n"
			"]")
		.arg(m_filename)
		.arg(m_densityMultiplier)
		.arg(m_albedo.toString());
	}
private:
	Transform m_worldToMedium;

	/* Memory map-related attributes */
#if defined(PLATFORM_WINDOWS)
	HANDLE m_file;
	HANDLE m_fileMapping;
#endif
	QString m_filename;
	size_t m_fileSize;
	float *m_data;

	/* Heterogeneous medium attributes */
	Color3f m_albedo;
	Vector3i m_resolution;
	float m_densityMultiplier;
};

NORI_REGISTER_CLASS(HeterogeneousMedium, "heterogeneous");
NORI_NAMESPACE_END
