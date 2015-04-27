// Brian Mc George
// MCGBRI004
// 26-04-2015

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
    setUpWidget();
    addActions();
    addMenus();
    addConections();
}

window::~window() {
    delete glWidget;
    delete newAction;
    delete openAction;
    delete fileMenu;
    for(unsigned int i =0; i<windows.size();++i) {
        delete windows[i];
    }
}

void window::setUpWidget() {
    QGLFormat glFormat;
    glFormat.setVersion( 3, 2 );
    glFormat.setProfile( QGLFormat::CoreProfile );
    glFormat.setSampleBuffers( true );
    glWidget = new GLWidget(glFormat);
    setCentralWidget(glWidget);
    glWidget->setFocus();
}

void window::addActions() {
    openAction = new QAction(tr("O&pen"), this);
    openAction->setStatusTip(tr("Opens a stl file"));
    newAction = new QAction(tr("N&ew"), this);
    newAction->setStatusTip(tr("Open new window"));
    resetAction = new QAction(tr("R&eset"), this);
    resetAction->setStatusTip(tr("Resets the scene"));
}

void window::addMenus() {
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAction);
    fileMenu->addAction(openAction);
    fileMenu->addAction(resetAction);
}

void window::addConections() {
    connect(newAction, SIGNAL(triggered()), this, SLOT(newWindow()));
    connect(openAction, SIGNAL(triggered()), this, SLOT(open()));
    connect(resetAction, SIGNAL(triggered()), this, SLOT(reset()));
}

void window::open() {
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open stl file"), "", tr("*.stl"));
    if(fileName != "") {
        glWidget->loadSTLFile(fileName.toStdString());
    }
}

void window::newWindow() {
    windows.push_back(new window());
    windows.back()->resize(640,480);
    windows.back()->show();
}

void window::reset() {
    glWidget->reset();
}
