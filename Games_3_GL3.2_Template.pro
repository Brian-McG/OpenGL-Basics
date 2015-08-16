HEADERS       = glheaders.hpp \
                glwidget.hpp \
                window.hpp \
                model.hpp

SOURCES       = glwidget.cpp \
                main.cpp \
                window.cpp \
                model.cpp
QT += core gui opengl widgets
LIBS += -lGLEW
RESOURCES += resources.qrc
#QMAKE_CXXFLAGS += -DSHOULD_LOAD_SHADERS -std=c++11
QMAKE_CXXFLAGS += -std=c++11
# install
target.path = boom
INSTALLS += target
win32 {
# Unsupported, do it yourself.
}
macx {
copydata.commands = cp -r $$PWD/models $$OUT_PWD
}
unix {
copydata.commands = cp -r $$PWD/models $$OUT_PWD
}
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata
