#include "glwidget.h"
#include <QtGui>
#include "window.h"
#include <QGLFormat>
#include <QAction>
#include <QMenuBar>
#include <QFileDialog>
#include <string>

window::window(QWidget *parent) :
    QMainWindow(parent)
{
    QGLFormat glFormat;
    glFormat.setVersion( 3, 2 );
    glFormat.setProfile( QGLFormat::CoreProfile );
    glFormat.setSampleBuffers( true );
    glWidget = new GLWidget(glFormat);
    setCentralWidget(glWidget);
    glWidget->setFocus();
    openAction = new QAction(tr("O&pen"), this);
    openAction->setStatusTip(tr("Opens a stl file"));
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAction);
    connect(openAction, SIGNAL(triggered()), this, SLOT(open()));
}

void window::open() {
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open stl file"), "", tr("*.stl"));
    glWidget->loadSTLFile(fileName.toStdString());
}
