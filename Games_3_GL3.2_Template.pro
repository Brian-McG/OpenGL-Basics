HEADERS       = glheaders.h \
                glwidget.h \  
    window.h

SOURCES       = glwidget.cpp \
                main.cpp \
    window.cpp
QT += core gui opengl widgets
LIBS += -lGLEW
RESOURCES += resources.qrc
#QMAKE_CXXFLAGS += -DSHOULD_LOAD_SHADERS -std=c++11
QMAKE_CXXFLAGS += -std=c++11
# install
target.path = boom
INSTALLS += target
win32 {
copydata.commands = @call copy $$PWD/bunny.stl $$OUT_PWD
}
macx {
copydata.commands = cp $$PWD/bunny.stl $$OUT_PWD
}
unix {
copydata.commands = cp $$PWD/bunny.stl $$OUT_PWD
}
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata

