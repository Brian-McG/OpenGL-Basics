// Copyright[2015] <Brian Mc George>
// MCGBRI004

#include <QCoreApplication>
#include <QKeyEvent>
#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include <memory>
#include <stdexcept>
#include "./glwidget.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"

#define VERT_SHADER ":/simple.vert"
#define FRAG_SHADER ":/simple.frag"
#define BUNNY_LOCATION "bunny.stl"

GLWidget::GLWidget(const QGLFormat& format, QWidget* parent)
: QGLWidget(format, parent),
m_vertexBuffer(QOpenGLBuffer::VertexBuffer) {
}

void GLWidget::initializeGL() {
  // Resolve OpenGL functions
  glewExperimental = true;
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  GLenum GlewInitResult = glewInit();
  if (GlewInitResult != GLEW_OK) {
    const GLubyte* errorStr = glewGetErrorString(GlewInitResult);
    size_t size = strlen(reinterpret_cast<const char*>(errorStr));
    qDebug() << "Glew error "
    << QString::fromUtf8(
      reinterpret_cast<const char*>(errorStr), size);
    }

    // get context opengl-version
    qDebug() << "Widget OpenGl: " << format().majorVersion() << "." << format().minorVersion();
    qDebug() << "Context valid: " << context()->isValid();
    qDebug() << "Really used OpenGl: " << context()->format().majorVersion() << "." << context()->format().minorVersion();
    qDebug() << "OpenGl information: VENDOR:       " << (const char*)glGetString(GL_VENDOR);
    qDebug() << "                    RENDERDER:    " << (const char*)glGetString(GL_RENDERER);
    qDebug() << "                    VERSION:      " << (const char*)glGetString(GL_VERSION);
    qDebug() << "                    GLSL VERSION: " << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    QGLFormat glFormat = QGLWidget::format();
    if ( !glFormat.sampleBuffers() )
    qWarning() << "Could not enable sample buffers";

    // Set the clear color to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    // we need a VAO in core!
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    loadSTLFile(BUNNY_LOCATION);

    // If bunny fails to load, load a placeholder cube
    if (data.numTriangles == 0) {
      qDebug() << "The bunny failed to load, loading a placeholder cube instead.";
      data = GLWidget::stlData();
      data.numTriangles = 1;
      data.normals =  std::unique_ptr<glm::vec3[]>(new glm::vec3[data.numTriangles * 3]);
      data.vertices = std::unique_ptr<glm::vec4[]>(new glm::vec4[data.numTriangles*3]);
      data.vertices[0] = glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f);
      data.vertices[1] = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
      data.vertices[2] = glm::vec4(1.0f, -1.0f, 0.0f, 1.0f);
      data.normals[0] = glm::vec3(0.0f, 0.0f, 1.0f);
      data.normals[1] = glm::vec3(0.0f, 0.0f, 1.0f);
      data.normals[2] = glm::vec3(0.0f, 0.0f, 1.0f);
    }

    // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit to 100 units
    Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);

    // Camera Matrix
    View = glm::lookAt(glm::vec3(0, 0, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    Model = glm::translate(glm::mat4(1), glm::vec3(0, 0, 0));
    MVP = Projection * View * translateMat * zRotMat * yRotMat * xRotMat * Model;

    m_vertexBuffer.create();
    m_vertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (!m_vertexBuffer.bind()) {
      qWarning() << "Could not bind vertex buffer to the context";
      return;
    }
    m_vertexBuffer.allocate(data.vertices.get(), data.numTriangles * 3 * sizeof(glm::vec4));

    qDebug() << "Attempting vertex shader load from " << VERT_SHADER;
    qDebug() << "Attempting fragment shader load from " << FRAG_SHADER;

    // Prepare a complete shader program...
    if ( !prepareShaderProgram( VERT_SHADER, FRAG_SHADER) )
    std::runtime_error("Failed to load shader");
    // Bind the shader program so that we can associate variables from
    // our application to the shaders
    if (!m_shader.bind()) {
      qWarning() << "Could not bind shader program to context";
      return;
    }
    // Enable the "vertex" attribute to bind it to our currently bound
    // vertex buffer.

    m_shader.enableAttributeArray("vertex");
    m_shader.setAttributeBuffer("vertex", GL_FLOAT, 0, 4);


    m_normalBuffer.create();
    m_normalBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (!m_normalBuffer.bind()) {
      qWarning() << "Could not bind vertex buffer to the context";
      return;
    }
    m_normalBuffer.allocate(data.normals.get(), data.numTriangles * 3 * sizeof(glm::vec3));
    m_shader.bind();
    m_shader.enableAttributeArray("normal");
    m_shader.setAttributeBuffer("normal", GL_FLOAT, 0, 3);

    color = std::move(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    GLuint MatrixID = glGetUniformLocation(m_shader.programId(), "MVP");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

    xRotMat = std::move(glm::mat4(1.0f));
    yRotMat = std::move(glm::mat4(1.0f));
    zRotMat = std::move(glm::mat4(1.0f));
    scaleMat = std::move(glm::mat4(1.0f));
    light_position = glm::vec3(0, 0, -10);

    connect(&redrawTimer, SIGNAL(timeout()), this, SLOT(updateGL()));
    if (format().swapInterval() == -1) {  // V-sync unavailable
        redrawTimer.setInterval(17);
    } else {  // V-sync available
        redrawTimer.setInterval(0);
    }
    redrawTimer.start();
  }


  void GLWidget::reset() {
    lastX = 20000;
    lastY = 20000;
    xAngle = 0.0f;
    yAngle = 0.0f;
    zAngle = 0.0f;
    translateX = 0.0f;
    translateY = 0.0f;
    translateZ = 3.0f;
    scale = 1.0f;
    xRotMat = glm::mat4(1.0f);
    yRotMat = glm::mat4(1.0f);
    zRotMat = glm::mat4(1.0f);
    scaleMat = glm::mat4(1.0f);
    translateMat = glm::mat4(1.0f);
    color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    mode = -1;
    rotMode = -1;
    repaint();
  }

  void GLWidget::loadSTLFile(const std::string & file_name) {
    std::ifstream fileStream;
    fileStream.open(file_name, std::ios_base::in | std::ios::binary);
    if (fileStream.fail()) {
      qWarning() << "An error occurred accessing the file does the file exist?";
      fileStream.close();
      data = std::move(GLWidget::stlData());
    }

    // Ignore header
    fileStream.ignore(80, EOF);

    // Number of triangles
    unsigned int numTriangles;
    fileStream.read(reinterpret_cast<char *>(&numTriangles), 4);
    if (fileStream.fail()) {
      qWarning() << "An error occurred reading from file";
      fileStream.close();
      data = std::move(GLWidget::stlData());
      return;
    }

    GLWidget::stlData model;
    model.numTriangles = numTriangles;
    qDebug() << "Number of triangles:" << numTriangles;
    model.normals = std::unique_ptr<glm::vec3[]>(new glm::vec3[numTriangles * 3]);
    model.vertices = std::unique_ptr<glm::vec4[]>(new glm::vec4[numTriangles * 3]);
    unsigned int triCounter = 0;
    for (unsigned int i = 0; i < numTriangles; ++i) {
      float normX, normY, normZ, x, y, z;
      fileStream.read(reinterpret_cast<char *>(&normX), 4);
      fileStream.read(reinterpret_cast<char *>(&normY), 4);
      fileStream.read(reinterpret_cast<char *>(&normZ), 4);
      fileStream.read(reinterpret_cast<char *>(&x), 4);
      fileStream.read(reinterpret_cast<char *>(&y), 4);
      fileStream.read(reinterpret_cast<char *>(&z), 4);
      if (fileStream.fail()) {
        qWarning() << "An error occurred reading from file";
        fileStream.close();
        data = std::move(GLWidget::stlData());
        m_vertexBuffer.allocate( data.vertices.get(), data.numTriangles * 3 * sizeof(glm::vec4) );
        return;
      }

      // Normal Vector
      model.normals[triCounter] = glm::vec3(normX, normY, normZ);
      model.normals[triCounter+1] = glm::vec3(normX, normY, normZ);
      model.normals[triCounter+2] = glm::vec3(normX, normY, normZ);
      // Vertex 1
      model.vertices[triCounter] = glm::vec4(x, y, z, 1.0f);

      // Vertex 2
      x = 0.0f;
      y = 0.0f;
      z = 0.0f;
      fileStream.read(reinterpret_cast<char *>(&x), 4);
      fileStream.read(reinterpret_cast<char *>(&y), 4);
      fileStream.read(reinterpret_cast<char *>(&z), 4);
      if (fileStream.fail()) {
        qWarning() << "An error occurred reading from file";
        fileStream.close();
        data = std::move(GLWidget::stlData());
        m_vertexBuffer.allocate( data.vertices.get(), data.numTriangles * 3 * sizeof(glm::vec4) );
        return;
      }
      model.vertices[triCounter+1] = glm::vec4(x, y, z, 1.0f);

      // Vertex 3
      x = 0.0f;
      y = 0.0f;
      z = 0.0f;
      fileStream.read(reinterpret_cast<char *>(&x), 4);
      fileStream.read(reinterpret_cast<char *>(&y), 4);
      fileStream.read(reinterpret_cast<char *>(&z), 4);
      if (fileStream.fail()) {
        qWarning() << "An error occurred reading from file";
        fileStream.close();
        data = std::move(GLWidget::stlData());
        m_vertexBuffer.allocate( data.vertices.get(), data.numTriangles * 3 * sizeof(glm::vec4) );
        return;
      }
      model.vertices[triCounter+2] = glm::vec4(x, y, z, 1.0f);

      // Attribute byte count
      fileStream.ignore(2, EOF);

      triCounter+=3;
    }
    data = std::move(model);
    return;
  }

  bool loadObjImage(const std::string & file_name) {
    std::vector< unsigned int > vertex_indices, uv_indices, normal_indices;
    std::vector< glm::vec3 > temp_vertices;
    std::vector< glm::vec2 > temp_uvs;
    std::vector< glm::vec3 > temp_normals;
    FILE * file = fopen(file_name.c_str(), "r");
    if( file == NULL ){
      printf("Impossible to open the file !\n");
      return false;
    }
  }

  void GLWidget::resizeGL(int w, int h) {
    width = w;
    height = h;
    // Set the viewport to window dimensions
    glViewport(0, 0, w, qMax(h, 1));
  }

  void GLWidget::paintGL() {
    // (Re)calculate model view projection
    Model = glm::translate(glm::mat4(1), glm::vec3(-translateX, -translateY, 0));
    Model = glm::rotate(Model, xAngle, glm::vec3(1.0f, 0, 0));
    Model = glm::rotate(Model, yAngle, glm::vec3(0, 1.0f, 0));
    Model = glm::rotate(Model, zAngle, glm::vec3(0, 0, 1.0f));
    Model = glm::scale(Model, glm::vec3(scale, scale, scale));
    MVP = Projection  * View * Model;
    lightRotation += 0.2f;
    if (lightRotation > 360.0f) {
      lightRotation = 0.0f;
    }
    glm::mat4 MVN = glm::inverse(glm::transpose(glm::mat4(View * Model)));
    GLuint NormalID = glGetUniformLocation(m_shader.programId(), "MVN");
    glUniformMatrix4fv(NormalID, 1, GL_FALSE, &MVN[0][0]);

    GLuint ViewID = glGetUniformLocation(m_shader.programId(), "view");
    glUniformMatrix4fv(ViewID, 1, GL_FALSE, &View[0][0]);

    float radian_rotation = glm::radians(lightRotation);
    glm::vec3 rotated_light = glm::mat3x3(glm::vec3(glm::cos(radian_rotation), 0, glm::sin(radian_rotation)), glm::vec3(0, 1, 0), glm::vec3(-glm::sin(radian_rotation), 0, glm::cos(radian_rotation))) * light_position;
    // qDebug() << glm::sin(radian_rotation) << " " << rotated_light[0] << " " << rotated_light[1] << " " << rotated_light[2];
    glUniform3f(glGetUniformLocation(m_shader.programId(), "rotating_light"), rotated_light[0], rotated_light[1], rotated_light[2]);
    // glm::vec4 ambprod_rotating_light = glm::vec4(0.00f, 0.00f, 0.00f, 1.0f);
    // glm::vec4 diffprod_rotating_light = glm::vec4(0.0f, 0.3f, 0.0f, 1.0f);
    // glm::vec4 specprod_rotating_light = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    glm::vec4 ambprod_rotating_light = glm::vec4(0.00f, 0.00f, 0.00f, 1.0f);
    glm::vec4 diffprod_rotating_light = glm::vec4(0.0f, 0.3f, 0.0f, 1.0f);
    glm::vec4 specprod_rotating_light = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    glUniform4f(glGetUniformLocation(m_shader.programId(), "ambprod_rotating_light"), ambprod_rotating_light[0], ambprod_rotating_light[1], ambprod_rotating_light[2], ambprod_rotating_light[3]);
    glUniform4f(glGetUniformLocation(m_shader.programId(), "diffprod_rotating_light"), diffprod_rotating_light[0], diffprod_rotating_light[1], diffprod_rotating_light[2], diffprod_rotating_light[3]);
    glUniform4f(glGetUniformLocation(m_shader.programId(), "specprod_rotating_light"), specprod_rotating_light[0], specprod_rotating_light[1], specprod_rotating_light[2], specprod_rotating_light[3]);
    glUniform1f(glGetUniformLocation(m_shader.programId(), "shine_rotating_light"), 100.0);

    glm::vec3 static_light(0.0f, 0.0f, 10.0f);
    glUniform3f(glGetUniformLocation(m_shader.programId(), "static_light"), static_light[0], static_light[1], static_light[2]);
    glm::vec4 ambprod_static_light = glm::vec4(0.05, 0.05, 0.05, 1.0);
    glm::vec4 diffprod_static_light = glm::vec4(0.3, 0.3, 0.3, 1.0);
    glm::vec4 specprod_static_light = glm::vec4(0.6, 0.6, 0.6, 1.0);
    glUniform4f(glGetUniformLocation(m_shader.programId(), "ambprod_static_light"), ambprod_static_light[0], ambprod_static_light[1], ambprod_static_light[2], ambprod_static_light[3]);
    glUniform4f(glGetUniformLocation(m_shader.programId(), "diffprod_static_light"), diffprod_static_light[0], diffprod_static_light[1], diffprod_static_light[2], diffprod_static_light[3]);
    glUniform4f(glGetUniformLocation(m_shader.programId(), "specprod_static_light"), specprod_static_light[0], specprod_static_light[1], specprod_static_light[2], specprod_static_light[3]);
    glUniform1f(glGetUniformLocation(m_shader.programId(), "shine_static_light"), 100.0);

    GLuint MatrixID = glGetUniformLocation(m_shader.programId(), "MVP");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

    GLuint modelID = glGetUniformLocation(m_shader.programId(), "model");
    glUniformMatrix4fv(modelID, 1, GL_FALSE, &Model[0][0]);

    // Clear the buffer with the current clearing color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw triangles
    glDrawArrays(GL_TRIANGLES, 0, data.numTriangles * 3);
  }

  QGLFormat GLWidget::desiredFormat()
  {
    QGLFormat fmt;
    fmt.setSwapInterval(1);
    return fmt;
  }

  void GLWidget::rotX(const int & change) {
    if (change != 0) {
      xAngle += rotateSpeed * change;
      if (xAngle > 360.0f) {
        xAngle -= 360.0f;
      }
      if (xAngle < -360.0f) {
        xAngle += 360.0f;
      }
      // x rotation matrix
      glm::vec4 xRot(1.0f, 0.0f, 0.0f, 0.0f);
      glm::vec4 yRot(0.0f, cosf(xAngle), -sinf(xAngle), 0.0f);
      glm::vec4 zRot(0.0f, sinf(xAngle), cosf(xAngle), 0.0f);
      glm::vec4 wRot(0.0f, 0.0f, 0.0f, 1.0f);
      xRotMat = glm::mat4(xRot, yRot, zRot, wRot);
      repaint();
    }
  }

  void GLWidget::rotY(const int &change) {
    if (change != 0) {
      yAngle += rotateSpeed * change;
      if (yAngle > 360.0f) {
        yAngle -= 360.0f;
      }
      if (yAngle < -360.0f) {
        yAngle += 360.0f;
      }
      // y rotation matrix
      glm::vec4 xRot(cosf(yAngle), 0.0f, sinf(yAngle), 0.0f);
      glm::vec4 yRot(0.0f, 1.0f, 0.0f, 0.0f);
      glm::vec4 zRot(-sinf(yAngle), 0.0f, cosf(yAngle), 0.0f);
      glm::vec4 wRot(0.0f, 0.0f, 0.0f, 1.0f);
      yRotMat = glm::mat4(xRot, yRot, zRot, wRot);
      repaint();
    }
  }

  void GLWidget::rotZ(const int &change) {
    if (change != 0) {
      zAngle += rotateSpeed * change;
      if (zAngle > 360.0f) {
        zAngle -= 360.0f;
      }
      if (zAngle < -360.0f) {
        zAngle += 360.0f;
      }
      // z rotation matrix
      glm::vec4 xRot(cosf(zAngle), -sinf(zAngle), 0.0f, 0.0f);
      glm::vec4 yRot(sinf(zAngle), cosf(zAngle), 0.0f, 0.0f);
      glm::vec4 zRot(0.0f, 0.0f, 1.0f, 0.0f);
      glm::vec4 wRot(0.0f, 0.0f, 0.0f, 1.0f);
      zRotMat = glm::mat4(xRot, yRot, zRot, wRot);
      repaint();
    }
  }

  void GLWidget::scaleBy(const int &change) {
    if (change != 0) {
      scale -= change * scaleSpeed;
      if (scale < 0.0f) {
        scale = 0.0f;
      }
      glm::vec4 scaleX(scale, 0.0f, 0.0f, 0.0f);
      glm::vec4 scaleY(0.0f, scale, 0.0f, 0.0f);
      glm::vec4 scaleZ(0.0f, 0.0f, scale, 0.0f);
      glm::vec4 scaleW(0.0f, 0.0f, 0.0f, 1.0f);
      scaleMat = glm::mat4(scaleX, scaleY, scaleZ, scaleW);
      repaint();
    }
  }

  void GLWidget::mouseMoveEvent(QMouseEvent* e) {
    if (mode == 0 || mode == 1 || mode == 2) {
      // Update change in mouse position
      int x = (width/2) - e->x();
      int y = (height/2) - e->y();
      int dx = lastX - x;
      int dy = lastY - y;
      if (dx > 5 || dx < -5) {
        dx = 0;
      }
      if (dy > 5 || dy < -5) {
        dy = 0;
      }
      lastX = x;
      lastY = y;
      if (mode == 0) {
        // x-rotation
        if (rotMode == 0) {
          rotX(-dy);
        } else if (rotMode == 1) {  // y-rotation
          rotY(-dx);
        } else if (rotMode == 2) {  // z-rotation
          rotZ(dx);
        }
      } else if (mode == 1) {
        // Up -> increase scale
        // Down -> decrease scale
        scaleBy(dy);
      } else if (mode == 2) {
        translateX -= dx * translateSpeed;
        translateY += dy * translateSpeed;
        repaint();
      }
    }
  }

  void GLWidget::wheelEvent(QWheelEvent *e) {
    if (mode == 0 || mode == 1 || mode == 2) {
      // Update change in wheel
      int wheel = e->delta();
      int change = 0;
      if (wheel > 0) {
        change = 5;
      } else if (wheel < 0) {
        change = -5;
      }
      if (mode == 0) {
        if (rotMode == 0) {  // x-rotation
          rotX(change);
        } else if (rotMode == 1) {  // y-rotation
          rotY(change);
        } else if (rotMode == 2) {  // z-rotation
          rotZ(change);
        }
      } else if (mode == 1) {
        scaleBy(-change);
      } else if (mode == 2) {
        translateZ += change * translateSpeed;
        repaint();
      }
    }
  }

  void GLWidget::keyPressEvent(QKeyEvent* e) {
    switch ( e->key() ) {
      case Qt::Key_R:  // Rotation mode
      lastX = 20000;
      lastY = 20000;
      mode = 0;
      rotMode += 1;
      if (rotMode >= 3) {
        rotMode = 0;
      }
      if (rotMode == 0) {
        qDebug() << "x rotation mode";
      }
      if (rotMode == 1) {
        qDebug() << "y rotation mode";
      }
      if (rotMode == 2) {
        qDebug() << "z rotation mode";
      }
      break;
      case Qt::Key_S:  // Scale Mode
      qDebug() << "Scale mode";
      lastX = 20000;
      lastY = 20000;
      mode = 1;
      break;
      case Qt::Key_T:  // Translate Mode
      qDebug() << "Translate mode";
      lastX = 20000;
      lastY = 20000;
      mode = 2;
      break;
      case Qt::Key_1:  // Color 1
      color = std::move(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
      repaint();
      break;
      case Qt::Key_2:  // Color 2
      color = std::move(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
      repaint();
      break;
      case Qt::Key_3:  // Color 3
      color = std::move(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
      repaint();
      break;
      case Qt::Key_4:  // Color 4
      color = std::move(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
      repaint();
      break;
      case Qt::Key_5:  // Color 5
      color = std::move(glm::vec4(0.8f, 0.72f, 0.878f, 1.0f));
      repaint();
      break;
      case Qt::Key_Escape:
      QCoreApplication::instance()->quit();
      break;

      default:
      QGLWidget::keyPressEvent(e);
    }
  }

  bool GLWidget::prepareShaderProgram(const QString& vertexShaderPath,
    const QString& fragmentShaderPath) {
      // First we load and compile the vertex shader...
      bool result = m_shader.addShaderFromSourceFile(QOpenGLShader::Vertex, vertexShaderPath);
      if ( !result )
      qWarning() << m_shader.log();

      // ...now the fragment shader...
      result = m_shader.addShaderFromSourceFile(QOpenGLShader::Fragment, fragmentShaderPath);
      if ( !result )
      qWarning() << m_shader.log();

      // ...and finally we link them to resolve any references.
      result = m_shader.link();
      if ( !result )
      qWarning() << "Could not link shader program:" << m_shader.log();

      return result;
    }
