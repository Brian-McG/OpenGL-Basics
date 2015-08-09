// Copyright[2015] <Brian Mc George>
// MCGBRI004

#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <string>
#include <vector>
#include "glm/glm.hpp"
#include "./glheaders.h"  // Must be included before QT opengl headers
#include <QGLWidget>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QTimer>




#include <memory>
class GLWidget : public QGLWidget
{
    Q_OBJECT
public:
    GLWidget(const QGLFormat& format, QWidget* parent = 0);
    struct stlData {
        int numTriangles = 0;
        std::unique_ptr<glm::vec4[]> vertices;
        std::unique_ptr<glm::vec3[]> normals;
    };
    void loadSTLFile(const std::string & fileName);
    void loadObjImage(const std::string & fileName);
    void reset();

protected:
    virtual void initializeGL();
    virtual void resizeGL(int w, int h);
    virtual void paintGL();
    virtual void mouseMoveEvent(QMouseEvent* m);
    virtual void keyPressEvent(QKeyEvent* e);
    virtual void wheelEvent(QWheelEvent* e);

private:
    void rotX(const int & change);
    void rotY(const int & change);
    void rotZ(const int & change);
    void transX(const int & change);
    void transY(const int & change);
    void transZ(const int & change);
    void scaleBy(const int & change);
    QGLFormat desiredFormat();
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
    float translateX = 0.0f;
    float translateY = 0.0f;
    float translateZ = 3.0f;
    float translateSpeed = 0.005f;
    float lightRotation = 0.0f;
    QTimer redrawTimer;
    glm::vec3 light_position;
    glm::mat4 xRotMat;
    glm::mat4 yRotMat;
    glm::mat4 zRotMat;
    glm::mat4 scaleMat;
    glm::mat4 translateMat;
    glm::vec4 color;
    char mode = -1;
    char rotMode = -1;
    stlData data;
    bool prepareShaderProgram(const QString& vertexShaderPath,
                               const QString& fragmentShaderPath);

    QOpenGLShaderProgram m_shader;
    QOpenGLBuffer m_vertexBuffer;
    QOpenGLBuffer m_normalBuffer;

    //Obj related
    std::vector<glm::vec3> out_verticies;
    std::vector<glm::vec2> out_uvs;
    std::vector<glm::vec3> out_normals;
};
#endif // GLWIDGET_H
