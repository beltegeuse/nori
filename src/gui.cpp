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

#include <nori/gui.h>
#include <QGLWidget>
#include <QGLShader>

using namespace nori;

#if defined(Q_WS_MACX)
	extern "C" {
		/* Bring the application to the front when starting it 
		   from the terminal (import from support_osx.m) */
		void nori_raise_osx();
	}
#endif

#if !defined(GL_RGBA32F_ARB)
	#define GL_RGBA32F_ARB 0x8814
#endif

class PreviewWidget : public QGLWidget {
public:
	PreviewWidget(QWidget *parent, const ImageBlock *output)
		: QGLWidget(parent), m_output(output), m_scale(1.0f) {
		const Vector2i &size = m_output->getSize();
		setMinimumSize(size.x(), size.y());
		setMaximumSize(size.x(), size.y());
	}

	QSize sizeHint() const {
		const Vector2i &size = m_output->getSize();
		return QSize(size.x(), size.y());
	}

	void refresh() {
		/* Reload the partially rendered image into a texture */
		m_output->lock();
		int borderSize = m_output->getBorderSize();
		const Vector2i &size = m_output->getSize();
		glPixelStorei(GL_UNPACK_ROW_LENGTH, m_output->cols());
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, size.x(), size.y(),
				0, GL_RGBA, GL_FLOAT, (uint8_t *) m_output->data() + 
				(borderSize * m_output->cols() + borderSize) * sizeof(Color4f));
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		m_output->unlock();
		if (m_program.isLinked()) 
			updateGL();
	}

	void setScale(float scale) {
		m_scale = scale;
		updateGL();
	}
protected:
	void initializeGL() {
		glClearColor(0.0, 0.0, 0.0, 0.0);
		/* Allocate texture memory for the rendered image */
		glGenTextures(1, &m_texture);
		glBindTexture(GL_TEXTURE_2D, m_texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		refresh();

		if (!m_program.addShaderFromSourceCode(QGLShader::Vertex,
				"void main() {\n"
				"	gl_Position = ftransform();\n"
				"   gl_TexCoord[0]  = gl_MultiTexCoord0;\n"
				"}\n"))
			throw NoriException("Could not compile vertex shader!");

		if (!m_program.addShaderFromSourceCode(QGLShader::Fragment,
				"#version 120\n"
				"uniform sampler2D source;\n"
				"uniform float scale;\n"
				"\n"
				"float toSRGB(float value) {\n"
				"	if (value < 0.0031308)\n"
				"		return 12.92 * value;\n"
				"	return 1.055 * pow(value, 0.41666) - 0.055;\n"
				"}\n"
				"\n"
				"void main() {\n"
				"	vec4 color = texture2D(source, gl_TexCoord[0].xy);\n"
				"	color *= scale / color.w;\n"
				"	gl_FragColor = vec4(toSRGB(color.r), toSRGB(color.g), toSRGB(color.b), 1);\n"
				"}\n"))
			throw NoriException("Could not compile fragment shader!");

		if (!m_program.link())
			throw NoriException("Could not link shader!");
	}

	void resizeGL(int w, int h) {
		glViewport(0, 0, w, h);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, 1, 1, 0, -1, 1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glEnable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}

	void paintGL() {
		glClear(GL_COLOR_BUFFER_BIT);
		m_program.bind();
		glBindTexture(GL_TEXTURE_2D, m_texture);
		m_program.setUniformValue("scale", m_scale);
		m_program.setUniformValue("source", 0);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(0.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(1.0f, 0.0f);
		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(1.0f, 1.0f);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(0.0f, 1.0f);
		glEnd();
	}
private:
	const ImageBlock *m_output;
	GLuint m_texture;
	float m_scale;
	QGLShaderProgram m_program;
};


NoriWindow::NoriWindow(const ImageBlock *output) : QWidget(NULL) {
	setWindowTitle("Nori");

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setSizeConstraint(QLayout::SetFixedSize);
	setLayout(layout);
	m_preview = new PreviewWidget(this, output);

	m_exposure = new QSlider(this);
	m_exposure->setMinimum(-1000);
	m_exposure->setMaximum(1000);
	m_exposure->setPageStep(1);
	m_exposure->setValue(0);
	m_exposure->setOrientation(Qt::Horizontal);
	m_exposure->setTickInterval(100);

	QHBoxLayout *layout2 = new QHBoxLayout();
	QLabel *label = new QLabel("Exposure : ", this);
	layout2->addWidget(label);
	layout2->addWidget(m_exposure);
	layout->addWidget(m_preview);
	layout->addLayout(layout2);

	#if defined(Q_WS_MACX)
		nori_raise_osx();
	#endif

	m_refreshTimer = new QTimer(this);
	m_refreshTimer->setInterval(500);

	connect(m_refreshTimer, SIGNAL(timeout()), this, SLOT(refresh()));
	connect(m_exposure, SIGNAL(valueChanged(int)), this, SLOT(setExposure(int)));
	show();
}

void NoriWindow::setExposure(int value) {
	m_preview->setScale(std::pow(2, value / 100.0f));
}

void NoriWindow::refresh() {
	m_preview->refresh();
}
