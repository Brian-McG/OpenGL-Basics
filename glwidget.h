#ifndef GLWIDGET_H
#define GLWIDGET_H

#include "glheaders.h" // Must be included before QT opengl headers
#include "glm/glm.hpp"
#include <QGLWidget>

#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

#include <memory>
class GLWidget : public QGLWidget
{
    Q_OBJECT
public:
    GLWidget( const QGLFormat& format, QWidget* parent = 0 );
    struct stlData {
        int numTriangles;
        std::unique_ptr<glm::vec4[]> vertices;
        std::unique_ptr<glm::vec3[]> normals;
    };

protected:
    virtual void initializeGL();
    virtual void resizeGL( int w, int h );
    virtual void paintGL();
    virtual stlData loadSTLFile(const std::string & fileName);

    virtual void keyPressEvent( QKeyEvent* e );

private:


    bool prepareShaderProgram( const QString& vertexShaderPath,
                               const QString& fragmentShaderPath );

    QOpenGLShaderProgram m_shader;
    QOpenGLBuffer m_vertexBuffer;
};

#endif // GLWIDGET_H
