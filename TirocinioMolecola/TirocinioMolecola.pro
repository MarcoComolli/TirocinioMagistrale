TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt



VCGLIB_PATH = F:/Programmazione/librerie_tesi_cpp/vcglib
GLEW_PATH = C:/Users/Marco/Desktop/glew-1.13.0/glew-1.13.0/
FREEGLUT_PATH = F:/Programmazione/librerie_tesi_cpp/freeglut/
ANTTWEAKBAR_PATH = F:/Programmazione/librerie_tesi_cpp/AntTweakBar/

INCLUDEPATH += $$VCGLIB_PATH
INCLUDEPATH += $$GLEW_PATH/include
INCLUDEPATH += $$FREEGLUT_PATH/include
INCLUDEPATH += $$ANTTWEAKBAR_PATH/include

INCLUDEPATH += F:/Programmazione/librerie_tesi_cpp/cg_tut/
INCLUDEPATH += F:/Programmazione/librerie_tesi_cpp/cg_tut/Lib/
INCLUDEPATH += F:/Programmazione/librerie_tesi_cpp/glm-0.9.7.1/glm

LIBS += -L$$GLEW_PATH/lib/
LIBS +=  -lopengl32 -llibglew32 -lglu32
LIBS += libfreeglut
LIBS += -L$$FREEGLUT_PATH/lib/
LIBS += -L$$ANTTWEAKBAR_PATH/lib -lAntTweakBar




SOURCES += qutemolLib\atomTable.cpp \
    qutemolLib\molecule.cpp \
    qutemolLib\shape.cpp \
    qutemolLib\settings.cpp \
    qutemolLib\glProgram.cpp \
    qutemolLib\glSurface.cpp \
    atompair.cpp \
    pairstatistics.cpp \
    myrenderer.cpp \
    dynamicball.cpp \
    dynamicshape.cpp \
    spatialgrid.cpp \
    main_glut.cpp \
    common_stuff.cpp \
    quterenderer.cpp



HEADERS += \
    log.h \
    qutemolLib\atomTable.h \
    qutemolLib\molecule.h \
    qutemolLib\shape.h \
    qutemolLib\basetypes.h \
    qutemolLib\shapingOptions.h \
    qutemolLib\settings.h \
    qutemolLib\glProgram.h \
    qutemolLib\glSurface.h \
    atompair.h \
    myrenderer.h \
    ../../../../../../Programmazione/librerie_tesi_cpp/cg_tut/ogldev_pipeline.h \
    dynamicshape.h \
    spatialgrid.h \
    preferences.h \
    quterenderer.h




OTHER_FILES += \
    shader.fs.glsl \
    shader.vs.glsl



#DEFINES += FREEGLUT_STATIC
DEFINES += GLEW_STATIC
SOURCES += $$GLEW_PATH/src/glew.c \

DISTFILES += \
    ballFinal.frag.glsl \
    ballFinal.vert.glsl



