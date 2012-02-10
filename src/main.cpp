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

#include <nori/parser.h>
#include <boost/scoped_ptr.hpp>
#include <QApplication>

int main(int argc, char **argv) {
	using namespace nori;
	
	QApplication app(argc, argv);
	Q_INIT_RESOURCE(resources);

	try {
		if (argc != 2) {
			cerr << "Syntax: nori <scene.xml>" << endl;
			return -1;
		}

		boost::scoped_ptr<NoriObject> scene(loadScene(argv[1]));

	} catch (const NoriException &ex) {
		cerr << "Caught a critical exception: " << qPrintable(ex.getReason()) << endl;
		return -1;
	}

	return 0;
}
