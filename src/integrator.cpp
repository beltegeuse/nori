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

#include <nori/integrator.h>
#include <nori/scene.h>
#include <nori/camera.h>
#include <nori/bitmap.h>

NORI_NAMESPACE_BEGIN

//// Reimplementation of the spiraling block generator by Adam Arbree
class BlockGenerator {
public:
	enum EDirection { ERight = 0, EDown, ELeft, EUp };

	BlockGenerator(const Vector2i &size, int blockSize) {
		m_numBlocks = Vector2i(
			(int) std::ceil(size.x() / (float) blockSize),
			(int) std::ceil(size.y() / (float) blockSize));
		m_blocksLeft = m_numBlocks.x() * m_numBlocks.y();
		m_blockSize = blockSize;
		m_direction = ERight;
		m_block = Point2i(m_numBlocks / 2);
		m_size = size;
		m_stepsLeft = 1;
		m_numSteps = 1;
	}

	bool next() {
		if (m_blocksLeft == 0)
			return false;

		Point2i pos = m_block * m_blockSize;
//		rect.setOffset(pos);
//		rect.setSize((m_size - pos).cwiseMin(Vector2i::Constant(m_blockSize)));

		if (--m_blocksLeft == 0)
			return false;

		do {
			switch (m_direction) {
				case ERight: ++m_block.x(); break;
				case EDown:  ++m_block.y(); break;
				case ELeft:  --m_block.x(); break;
				case EUp:    --m_block.y(); break;
			}

			if (--m_stepsLeft == 0) {
				m_direction = (m_direction + 1) % 4;
				if (m_direction == ELeft || m_direction == ERight) 
					++m_numSteps;
				m_stepsLeft = m_numSteps;
			}
		} while ((m_block.array() < 0).any() ||
		         (m_block.array() >= m_numBlocks.array()).any());

		return true;
	}

protected:
	Point2i m_block;
	Vector2i m_numBlocks;
	Vector2i m_size;
	int m_blockSize;
	int m_numSteps;
	int m_blocksLeft;
	int m_stepsLeft;
	int m_direction;
};



void Integrator::render(const Scene *scene) {
	const Camera *camera = scene->getCamera();
	Vector2i size = camera->getSize();
	
	BlockGenerator blockgen(size, NORI_BLOCK_SIZE);
	Bitmap bitmap(size);

	while (blockgen.next()) {
		cout << "Rendering a block" << endl;
	}
}

NORI_NAMESPACE_END
