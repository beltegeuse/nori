SOURCES += src/common.cpp \
	src/object.cpp \
	src/proplist.cpp \
	src/diffuse.cpp \
	src/isotropic.cpp \
	src/microfacet.cpp \
	src/scene.cpp \
	src/random.cpp \
	src/quad.cpp \
	src/ao.cpp \
	src/chi2test.cpp \
	src/ttest.cpp \
	src/mesh.cpp \
	src/kdtree.cpp \
	src/obj.cpp \
	src/perspective.cpp \
	src/rfilter.cpp \
	src/block.cpp \
	src/bitmap.cpp \
	src/parser.cpp \
	src/mirror.cpp \
	src/medium.cpp \
	src/homogeneous.cpp \
	src/heterogeneous.cpp \
	src/independent.cpp \
	src/main.cpp \
	src/gui.cpp

HEADERS += include/nori/*.h

RESOURCES += data/resources.qrc

INCLUDEPATH += include
DEPENDPATH += include data
OBJECTS_DIR = build
RCC_DIR = build
MOC_DIR = build
UI_DIR = build
DESTDIR = .

QT += xml xmlpatterns opengl

macx {
	OBJECTIVE_SOURCES += src/support_osx.m
	QMAKE_LFLAGS += -framework Cocoa -lobjc
	# The following is (strangely) needed for correct exception handling on Mac OS 10.6+
	QMAKE_CXXFLAGS_X86_64 -= -mmacosx-version-min=10.5
	QMAKE_CXXFLAGS_X86_64 += -mmacosx-version-min=10.6
}

macx | unix {
	QMAKE_CXXFLAGS_RELEASE -= -O2 
	QMAKE_CXXFLAGS += -O3 -march=nocona -msse2 -mfpmath=sse -fstrict-aliasing
	QMAKE_LIBDIR += /usr/local/lib
	INCLUDEPATH += /usr/include/OpenEXR /usr/local/include/OpenEXR
	LIBS += -lIlmImf -lIex
}

win32 {
	# You will have to update the following two lines based on where you have installed OpenEXR
	QMAKE_LIBDIR += openexr/lib/x64
	INCLUDEPATH += openexr/include

	QMAKE_CXXFLAGS += /O2 /fp:fast /GS- /GL /D_SCL_SECURE_NO_WARNINGS /D_CRT_SECURE_NO_WARNINGS
	QMAKE_LDFLAGS += /LTCG
	SOURCES += src/support_win32.cpp
	LIBS += IlmImf.lib Iex.lib IlmThread.lib Imath.lib Half.lib
}

TARGET = nori
CONFIG += console 
CONFIG -= app_bundle
