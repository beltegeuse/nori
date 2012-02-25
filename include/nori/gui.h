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

#if !defined(__GUI_H)
#define __GUI_H

#include <nori/block.h>
#include <QtGui>

class PreviewWidget;

/**
 * \brief Implements a simple preview window for watching
 * renderings as they progress
 */
class NoriWindow : public QWidget {
	Q_OBJECT
public:
	NoriWindow(const nori::ImageBlock *output);

	inline void startRefresh() { m_refreshTimer->start(); }
	inline void stopRefresh() { m_refreshTimer->stop(); }

private slots:
	void setExposure(int value);
	void refresh();

private:
	PreviewWidget *m_preview;
	QSlider *m_exposure;
	QTimer *m_refreshTimer;
};

#endif /* __GUI_H */
