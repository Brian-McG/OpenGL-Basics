// Copyright[2015] <Brian Mc George>
// MCGBRI004

#include <QApplication>
#include "./window.hpp"

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    window w;
    w.resize(640, 480);
    w.show();
    return a.exec();
}
