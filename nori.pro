SOURCES += src/common.cpp \
	src/object.cpp \
	src/proplist.cpp \
	src/transform.cpp \
	src/diffuse.cpp \
	src/microfacet.cpp \
	src/scene.cpp \
	src/random.cpp \
	src/quad.cpp \
	src/chi2test.cpp \
	src/ttest.cpp \
	src/parser.cpp \
	src/main.cpp

HEADERS += include/nori/common.h
RESOURCES += data/resources.qrc

INCLUDEPATH += include
DEPENDPATH += include data
OBJECTS_DIR = build
RCC_DIR = build
MOC_DIR = build
UI_DIR = build

QT += xml xmlpatterns opengl
QMAKE_CXXFLAGS += -fopenmp -O3 -march=nocona -msse2 -mfpmath=sse
QMAKE_LFLAGS += -fopenmp

TARGET = nori
CONFIG += console
CONFIG -= app_bundle


