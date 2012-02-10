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
DESTDIR = .

QT += xml xmlpatterns opengl

macx | unix {
	QMAKE_CXXFLAGS += -O3 -march=nocona -msse2 -mfpmath=sse
}

win32 {
	QMAKE_CXXFLAGS += /O2 /fp:fast /GS- /GL
	QMAKE_LDFLAGS += /LTCG
}

TARGET = nori
CONFIG += console
CONFIG -= app_bundle
