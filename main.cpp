// Brian Mc George
// MCGBRI004
// 25-04-2015

#include "window.h"
#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication a( argc, argv );
    window w;
    w.resize(640,480);
    w.show();
    return a.exec();
}

