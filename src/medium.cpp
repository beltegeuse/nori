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
#include <nori/phase.h>

NORI_NAMESPACE_BEGIN

Medium::Medium() : m_phaseFunction(NULL) { }

Medium::~Medium() {
	if (m_phaseFunction)
		delete m_phaseFunction;
}
	
void Medium::addChild(NoriObject *child) {
	switch (child->getClassType()) {
		case EPhaseFunction:
			if (m_phaseFunction)
				throw NoriException("Medium: tried to register multiple phase function instances!");
			m_phaseFunction = static_cast<PhaseFunction *>(child);
			break;

		default:
			throw NoriException(QString("Medium::addChild(<%1>) is not supported!").arg(
				classTypeName(child->getClassType())));
	}
}

void Medium::activate() {
	if (!m_phaseFunction)
		m_phaseFunction = static_cast<PhaseFunction *>(
			NoriObjectFactory::createInstance("isotropic", PropertyList()));
}

NORI_NAMESPACE_END
