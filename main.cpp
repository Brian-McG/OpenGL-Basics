//#include "glwidget.h"
#include "window.h"
#include <QApplication>
//#include <QMainWindow>
//#include <QGLFormat>

int main(int argc, char* argv[])
{
    QApplication a( argc, argv );
    // Specify an OpenGL 3.2 format using the Core profile.
    // That is, no old-school fixed pipeline functionality
   // QGLFormat glFormat;
   // glFormat.setVersion( 3, 2 );
   // glFormat.setProfile( QGLFormat::CoreProfile );
   // glFormat.setSampleBuffers( true );

  //  GLWidget w( glFormat );
   // w.resize(640,480);
   // w.show();
    window w;
    w.resize(640,480);
    w.show();
    return a.exec();
}

