// Brian Mc George
// MCGBRI004
// 26-04-2015

#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include <memory>
#include <vector>

//Forward declare class
class GLWidget;
class QAction;
class QMenu;

class window : public QMainWindow
{
    Q_OBJECT
public:
    virtual ~window();
    explicit window(QWidget *parent = 0);

private slots:
    void open();
    void newWindow();
    void reset();

private:
    void addActions();
    void addMenus();
    void addConections();
    void setUpWidget();
    QMenu *fileMenu;
    QAction *newAction;
    QAction *openAction;
    QAction *resetAction;
    GLWidget *glWidget;
    std::vector<window *> windows;
};

#endif // WINDOW_H
