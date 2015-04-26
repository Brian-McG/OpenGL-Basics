#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>

//Declare class
class GLWidget;
class QAction;
class QMenu;

class window : public QMainWindow
{
    Q_OBJECT
public:

    explicit window(QWidget *parent = 0);
private slots:
    void open();
private:

    QMenu *fileMenu;
    QAction *openAction;
    GLWidget *glWidget;
};

#endif // WINDOW_H
