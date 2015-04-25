HEADERS       = glheaders.h \
                glwidget.h 

SOURCES       = glwidget.cpp \
                main.cpp
QT += core gui opengl widgets
LIBS += -lGLEW
RESOURCES += resources.qrc
#QMAKE_CXXFLAGS += -DSHOULD_LOAD_SHADERS
# install
target.path = boom
INSTALLS += target
