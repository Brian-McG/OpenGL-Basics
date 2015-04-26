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
        int numTriangles=0;
        std::unique_ptr<glm::vec4[]> vertices;
        std::unique_ptr<glm::vec3[]> normals;
    };

protected:
    virtual void initializeGL();
    virtual void resizeGL( int w, int h );
    virtual void paintGL();
    virtual stlData loadSTLFile(const std::string & fileName);
    virtual void mouseMoveEvent(QMouseEvent* m);
    virtual void keyPressEvent( QKeyEvent* e );

private:
    glm::mat4 Projection;

    // Camera Matrix
    glm::mat4 View;
    glm::mat4 Model;
    glm::mat4 MVP;
    int width;
    int height;
    float xAngle = 0.0f;
    float yAngle = 0.0f;
    float zAngle = 0.0f;
    float rotateSpeed = 0.02f;
    int lastX;
    int lastY;
    float scale = 1.0f;
    float scaleSpeed = 0.02f;

    // Do this in cpp rather
    glm::mat4 xRotMat;
    glm::mat4 yRotMat;
    glm::mat4 zRotMat;
    glm::mat4 scaleMat;
    glm::vec4 color;
    unsigned char mode;
    unsigned char rotMode;
    stlData data;
    bool prepareShaderProgram( const QString& vertexShaderPath,
                               const QString& fragmentShaderPath );

    QOpenGLShaderProgram m_shader;
    QOpenGLBuffer m_vertexBuffer;
};

#endif // GLWIDGET_H
