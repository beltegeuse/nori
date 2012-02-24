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
#include <nori/parallel.h>
#include <nori/scene.h>
#include <nori/camera.h>
#include <nori/bitmap.h>

NORI_NAMESPACE_BEGIN

void Integrator::render(const Scene *scene, Sampler *sampler) {
	const Camera *camera = scene->getCamera();
	Vector2i outputSize = camera->getOutputSize();

	/* Create a block generator (i.e. a work scheduler) */
	BlockGenerator blockGenerator(outputSize, NORI_BLOCK_SIZE);

	/* Allocate memory for the entire output image */
	ImageBlock result(outputSize, camera->getReconstructionFilter());

	int nCores = getCoreCount();
	std::vector<RenderThread *> threads;

	/* Launch render threads */
	for (int i=0; i<nCores; ++i) {
		RenderThread *thread = new RenderThread(scene, sampler, &blockGenerator, &result);
		thread->start();
		threads.push_back(thread);
	}

	/* Wait for them to finish */
	for (int i=0; i<nCores; ++i) {
		threads[i]->wait();
		delete threads[i];
	}

	/* Now turn the rendered image block into 
	   a properly normalized bitmap and save it */
	Bitmap *bitmap = result.toBitmap();
	bitmap->save("output.exr");

	delete bitmap;
}

NORI_NAMESPACE_END
