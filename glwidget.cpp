// Copyright[2015] <Brian Mc George>
// MCGBRI004

#include <QKeyEvent>
#include <QCoreApplication>
#include <utility>
#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include "./glwidget.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"

#define VERT_SHADER ":/simple.vert"
#define FRAG_SHADER ":/simple.frag"
#define BUNNY_LOCATION_OBJ "models/Bunny/bunny.obj"
#define F16 "models/F16/f16.obj"
#define F16S_texture "models/F16/F16s.bmp"
#define F16T_texture "models/F16/F16t.bmp"
#define F16_normal_map "models/F16/normal_map.bmp"
#define F16_normal_map2 "models/F16/normal_map2.bmp"
#define F16_normal_map3 "models/F16/bricks_norm.bmp"
#define CUBE_OBJ "models/Cube/cube.obj"
#define CUBE_TEXTURE "models/Cube/randomtest.bmp"
#define CUBE_NORMAL "models/Cube/sampleNormal.bmp"

GLWidget::GLWidget(const QGLFormat& format, QWidget* parent)
: QGLWidget(format, parent),
m_vertex_buffer_(QOpenGLBuffer::VertexBuffer) {
}

void GLWidget::initializeGL() {
  glewExperimental = true;
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_ALPHA_TEST);

  GLenum GlewInitResult = glewInit();
  if (GlewInitResult != GLEW_OK) {
    const GLubyte* errorStr = glewGetErrorString(GlewInitResult);
    size_t size = strlen(reinterpret_cast<const char*>(errorStr));
    qDebug() << "Glew error "
    << QString::fromUtf8(
      reinterpret_cast<const char*>(errorStr), size);
    }
    m_normal_buffer_.destroy();
    m_vertex_buffer_.destroy();
    m_shader_.removeAllShaders();
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

    qDebug() << "Load obj";
    model_ = mcgbri004::Model();
    std::string texture_1_name;
    std::string texture_2_name;
    std::string normal_map_name;
    if (filename_ == "" || filename_ == "Cube_KEYPRESS") {
      model_obj_ = model_.loadObjImage(CUBE_OBJ);
      setNormalMapping(true);
      texture_1_name = CUBE_TEXTURE;
      texture_2_name = F16T_texture;
      normal_map_name = CUBE_NORMAL;
    } else if (filename_ == "F16_KEYPRESS") {
      setNormalMapping(false);
      model_obj_ = model_.loadObjImage(F16);
      texture_1_name = F16S_texture;
      texture_2_name = F16T_texture;
      normal_map_name = F16_normal_map;
    } else {
      model_obj_ = model_.loadObjImage(GLWidget::filename_);
    }
    model_transform_ = std::unique_ptr<mcgbri004::Model>(new mcgbri004::Model());

    // projection_ matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit to 100 units
    projection_ = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);

    // Camera Matrix
    view_projection_ = glm::lookAt(glm::vec3(0, 0, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    model_projection_ = glm::translate(glm::mat4(1), glm::vec3(0, 0, 0));
    MVP = projection_ * view_projection_ * model_projection_;

    // Vertex
    m_vertex_buffer_.create();
    m_vertex_buffer_.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (!m_vertex_buffer_.bind()) {
      qWarning() << "Could not bind vertex buffer to the context";
      return;
    }
    m_vertex_buffer_.allocate(model_obj_.vertices.get(), model_obj_.number_of_triangles * 3 * sizeof(glm::vec4));

    qDebug() << "Attempting vertex shader load from " << VERT_SHADER;
    qDebug() << "Attempting fragment shader load from " << FRAG_SHADER;

    // Prepare a complete shader program...
    if ( !prepareShaderProgram( VERT_SHADER, FRAG_SHADER) )
    std::runtime_error("Failed to load shader");
    // Bind the shader program so that we can associate variables from
    // our application to the shaders
    if (!m_shader_.bind()) {
      qWarning() << "Could not bind shader program to context";
      return;
    }
    // Enable the "vertex" attribute to bind it to our currently bound
    // vertex buffer.
    m_shader_.enableAttributeArray("vertex");
    m_shader_.setAttributeBuffer("vertex", GL_FLOAT, 0, 4);

    // Normals
    m_normal_buffer_.create();
    m_normal_buffer_.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (!m_normal_buffer_.bind()) {
      qWarning() << "Could not bind normal buffer to the context";
      return;
    }
    m_normal_buffer_.allocate(model_obj_.normals.get(), model_obj_.number_of_triangles * 3 * sizeof(glm::vec3));
    m_shader_.bind();
    m_shader_.enableAttributeArray("normal");
    m_shader_.setAttributeBuffer("normal", GL_FLOAT, 0, 3);

    // UV
    m_uv_buffer_.create();
    m_uv_buffer_.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (!m_uv_buffer_.bind()) {
      qWarning() << "Could not bind uv buffer to the context";
      return;
    }
    m_uv_buffer_.allocate(model_obj_.uvs.get(), model_obj_.number_of_triangles * 3 * sizeof(glm::vec2));
    m_shader_.bind();
    m_shader_.enableAttributeArray("uv");
    m_shader_.setAttributeBuffer("uv", GL_FLOAT, 0, 2);

    // Texture ID
    m_texture_id_buffer_.create();
    m_texture_id_buffer_.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (!m_texture_id_buffer_.bind()) {
      qWarning() << "Could not bind texture id to the context";
      return;
    }
    m_texture_id_buffer_.allocate(model_obj_.texture_id.get(), model_obj_.number_of_triangles * 3 * sizeof(int));
    m_shader_.bind();
    m_shader_.enableAttributeArray("sampler_index");
    m_shader_.setAttributeBuffer("sampler_index", GL_FLOAT, 0, 1);

    // Tangent
    m_tangent_buffer_.create();
    m_tangent_buffer_.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (!m_tangent_buffer_.bind()) {
      qWarning() << "Could not bind normal buffer to the context";
      return;
    }
    m_tangent_buffer_.allocate(model_obj_.tangents.get(), model_obj_.number_of_triangles * 3 * sizeof(glm::vec3));
    m_shader_.bind();
    m_shader_.enableAttributeArray("tangent");
    m_shader_.setAttributeBuffer("tangent", GL_FLOAT, 0, 3);

    // Bitangent
    m_bitangent_buffer_.create();
    m_bitangent_buffer_.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (!m_bitangent_buffer_.bind()) {
      qWarning() << "Could not bind normal buffer to the context";
      return;
    }
    m_bitangent_buffer_.allocate(model_obj_.bitangents.get(), model_obj_.number_of_triangles * 3 * sizeof(glm::vec3));
    m_shader_.bind();
    m_shader_.enableAttributeArray("bitangent");
    m_shader_.setAttributeBuffer("bitangent", GL_FLOAT, 0, 3);

    texture_0_ = loadBmpImage(texture_1_name);
    texture_1_ = loadBmpImage(texture_2_name);
    normal_map_ = loadBmpImage(normal_map_name);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_0_);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture_1_);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, normal_map_);
    qDebug() << "Texture load complete";

    GLuint MatrixID = glGetUniformLocation(m_shader_.programId(), "MVP");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // Setup light timer
    if (!redraw_timer_.isActive()) {
      connect(&redraw_timer_, SIGNAL(timeout()), this, SLOT(timerTick()));
      if (format().swapInterval() == -1) {  // V-sync unavailable
        redraw_timer_.setInterval(17);
      } else {  // V-sync available
        redraw_timer_.setInterval(0);
      }
      redraw_timer_.start();
    }
  }

  void GLWidget::reallocate_buffers() {
    initializeGL();
  }

  void GLWidget::reset() {
    model_transform_->reset();
    last_cursor_x_ = 20000;
    last_cursor_y_ = 20000;
    mode_ = -1;
    rotation_mode_ = -1;
    repaint();
  }

  // Set the viewport to window dimensions
  void GLWidget::resizeGL(int w, int h) {
    window_width_ = w;
    window_height_ = h;
    glViewport(0, 0, w, qMax(h, 1));
  }

  void GLWidget::paintGL() {
    // (Re)calculate model view projection
    model_projection_ = glm::translate(glm::mat4(1), glm::vec3(-model_transform_->get_x_translation(), -model_transform_->get_y_translation(), 0));
    model_projection_ = glm::rotate(model_projection_, model_transform_->get_x_angle(), glm::vec3(1.0f, 0, 0));
    model_projection_ = glm::rotate(model_projection_, model_transform_->get_y_angle(), glm::vec3(0, 1.0f, 0));
    model_projection_ = glm::rotate(model_projection_, model_transform_->get_z_angle(), glm::vec3(0, 0, 1.0f));
    float scale = model_transform_->get_scale();
    model_projection_ = glm::scale(model_projection_, glm::vec3(scale, scale, scale));
    MVP = projection_  * view_projection_ * model_projection_;
    GLuint MatrixID = glGetUniformLocation(m_shader_.programId(), "MVP");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // (Re)calculate model view normal projection
    glm::mat4 MVN = glm::inverse(glm::transpose(glm::mat4(view_projection_ * model_projection_)));
    GLuint NormalID = glGetUniformLocation(m_shader_.programId(), "MVN");
    glUniformMatrix4fv(NormalID, 1, GL_FALSE, &MVN[0][0]);

    // View projection
    GLuint ViewID = glGetUniformLocation(m_shader_.programId(), "view");
    glUniformMatrix4fv(ViewID, 1, GL_FALSE, &view_projection_[0][0]);

    // Model projection
    GLuint modelID = glGetUniformLocation(m_shader_.programId(), "model");
    glUniformMatrix4fv(modelID, 1, GL_FALSE, &model_projection_[0][0]);

    // Model projection
    GLuint mvID = glGetUniformLocation(m_shader_.programId(), "MV");
    glm::mat3 mv = glm::mat3(view_projection_ * model_projection_);
    glUniformMatrix3fv(mvID, 1, GL_FALSE, &mv[0][0]);

    // Rotating light
    float radian_rotation = glm::radians(light_rotation_);
    glm::vec3 rotated_light_1 = glm::mat3x3(glm::vec3(glm::cos(radian_rotation), 0, glm::sin(radian_rotation)), glm::vec3(0, 1, 0), glm::vec3(-glm::sin(radian_rotation), 0, glm::cos(radian_rotation))) * rotating_light_1_start_position_;
    glUniform3f(glGetUniformLocation(m_shader_.programId(), "rotating_light_1"), rotated_light_1[0], rotated_light_1[1], rotated_light_1[2]);
    glUniform4f(glGetUniformLocation(m_shader_.programId(), "ambprod_rotating_light_1"), ambprod_rotating_light_1_[0], ambprod_rotating_light_1_[1], ambprod_rotating_light_1_[2], ambprod_rotating_light_1_[3]);
    glUniform4f(glGetUniformLocation(m_shader_.programId(), "diffprod_rotating_light_1"), diffprod_rotating_light_1_[0], diffprod_rotating_light_1_[1], diffprod_rotating_light_1_[2], diffprod_rotating_light_1_[3]);
    glUniform4f(glGetUniformLocation(m_shader_.programId(), "specprod_rotating_light_1"), specprod_rotating_light_1_[0], specprod_rotating_light_1_[1], specprod_rotating_light_1_[2], specprod_rotating_light_1_[3]);
    glUniform1f(glGetUniformLocation(m_shader_.programId(), "shine_rotating_light_1"), shine_rotating_light_1_);

    // Static light
    glm::vec3 rotated_light_2 = glm::mat3x3(glm::vec3(1, 0, 0), glm::vec3(0, glm::cos(radian_rotation), -glm::sin(radian_rotation)), glm::vec3(0, glm::sin(radian_rotation), glm::cos(radian_rotation))) * rotating_light_2_start_position_;
    glUniform3f(glGetUniformLocation(m_shader_.programId(), "rotating_light_2"), rotated_light_2[0], rotated_light_2[1], rotated_light_2[2]);
    glUniform4f(glGetUniformLocation(m_shader_.programId(), "ambprod_rotating_light_2"), ambprod_rotating_light_2_[0], ambprod_rotating_light_2_[1], ambprod_rotating_light_2_[2], ambprod_rotating_light_2_[3]);
    glUniform4f(glGetUniformLocation(m_shader_.programId(), "diffprod_rotating_light_2"), diffprod_rotating_light_2_[0], diffprod_rotating_light_2_[1], diffprod_rotating_light_2_[2], diffprod_rotating_light_2_[3]);
    glUniform4f(glGetUniformLocation(m_shader_.programId(), "specprod_rotating_light_2"), specprod_rotating_light_2_[0], specprod_rotating_light_2_[1], specprod_rotating_light_2_[2], specprod_rotating_light_2_[3]);
    glUniform1f(glGetUniformLocation(m_shader_.programId(), "shine_rotating_light_2"), shine_rotating_light_2_);

    // Texture samplers
    GLuint texture_id_16s  = glGetUniformLocation(m_shader_.programId(), "texture_sampler_f16s");
    glUniform1i(texture_id_16s, 0);
    GLuint texture_id_f16t  = glGetUniformLocation(m_shader_.programId(), "texture_sampler_f16t");
    glUniform1i(texture_id_f16t, 1);
    GLuint texture_id_normal = glGetUniformLocation(m_shader_.programId(), "normal_texture_sampler");
    glUniform1i(texture_id_normal, 2);

    // Normal mapping status
    glUniform1i(glGetUniformLocation(m_shader_.programId(), "normal_mapping_active"), normal_mapping_active_);

    // Clear the buffer with the current clearing color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw triangles
    glDrawArrays(GL_TRIANGLES, 0, model_obj_.number_of_triangles * 3);
  }

  QGLFormat GLWidget::desiredFormat() {
    QGLFormat fmt;
    fmt.setSwapInterval(1);
    return fmt;
  }

  void GLWidget::rotX(const int & change) {
    if (model_transform_->rotateX(change)) {
      repaint();
    }
  }

  void GLWidget::rotY(const int & change) {
    if (model_transform_->rotateY(change)) {
      repaint();
    }
  }

  void GLWidget::rotZ(const int & change) {
    if (model_transform_->rotateZ(change)) {
      repaint();
    }
  }

  void GLWidget::scaleBy(const int & change) {
    if (model_transform_->scaleBy(change)) {
      repaint();
    }
  }

  void GLWidget::mouseMoveEvent(QMouseEvent* e) {
    if (mode_ == 0 || mode_ == 1 || mode_ == 2) {
      // Update change in mouse position
      int x = (window_width_ / 2) - e->x();
      int y = (window_height_ / 2) - e->y();
      int dx = last_cursor_x_ - x;
      int dy = last_cursor_y_ - y;
      if (dx > 5 || dx < -5) {
        dx = 0;
      }
      if (dy > 5 || dy < -5) {
        dy = 0;
      }
      last_cursor_x_ = x;
      last_cursor_y_ = y;
      if (mode_ == 0) {
        // x-rotation
        if (rotation_mode_ == 0) {
          rotX(-dy);
        } else if (rotation_mode_ == 1) {  // y-rotation
          rotY(-dx);
        } else if (rotation_mode_ == 2) {  // z-rotation
          rotZ(dx);
        }
      } else if (mode_ == 1) {
        // Up->increase scale
        // Down->decrease scale
        scaleBy(dy);
      } else if (mode_ == 2) {
        model_transform_->translateX(-dx);
        model_transform_->translateY(dy);
        repaint();
      }
    }
  }

  void GLWidget::wheelEvent(QWheelEvent *e) {
    if (mode_ == 0 || mode_ == 1 || mode_ == 2) {
      // Update change in wheel
      int wheel = e->delta();
      int change = 0;
      if (wheel > 0) {
        change = 5;
      } else if (wheel < 0) {
        change = -5;
      }
      if (mode_ == 0) {
        if (rotation_mode_ == 0) {  // x-rotation
          rotX(change);
        } else if (rotation_mode_ == 1) {  // y-rotation
          rotY(change);
        } else if (rotation_mode_ == 2) {  // z-rotation
          rotZ(change);
        }
      } else if (mode_ == 1) {
        scaleBy(-change);
      }
    }
  }

  void GLWidget::keyPressEvent(QKeyEvent* e) {
    switch ( e->key() ) {
      case Qt::Key_R:  // Rotation mode_
      last_cursor_x_ = 20000;
      last_cursor_y_ = 20000;
      mode_ = 0;
      rotation_mode_ += 1;
      if (rotation_mode_ >= 3) {
        rotation_mode_ = 0;
      }
      if (rotation_mode_ == 0) {
        qDebug() << "x rotation mode_";
      }
      if (rotation_mode_ == 1) {
        qDebug() << "y rotation mode_";
      }
      if (rotation_mode_ == 2) {
        qDebug() << "z rotation mode_";
      }
      break;
      case Qt::Key_S:  // Scale Mode
      qDebug() << "Scale mode_";
      last_cursor_x_ = 20000;
      last_cursor_y_ = 20000;
      mode_ = 1;
      break;

      case Qt::Key_T:  // Translate Mode
      qDebug() << "Translate mode_";
      last_cursor_x_ = 20000;
      last_cursor_y_ = 20000;
      mode_ = 2;
      break;

      case Qt::Key_N:
      setNormalMapping(!normal_mapping_active_);
      break;

      case Qt::Key_1:
      setFileName("Cube_KEYPRESS");
      reallocate_buffers();
      break;

      case Qt::Key_2:
      setFileName("F16_KEYPRESS");
      reallocate_buffers();
      break;

      case Qt::Key_Escape:
      QCoreApplication::instance()->quit();
      break;

      default:
      QGLWidget::keyPressEvent(e);
    }
  }

  bool GLWidget::prepareShaderProgram(const QString& vertexShaderPath, const QString& fragmentShaderPath) {
    // First we load and compile the vertex shader...
    bool result = m_shader_.addShaderFromSourceFile(QOpenGLShader::Vertex, vertexShaderPath);
    if ( !result )
    qWarning() << m_shader_.log();

    // ...now the fragment shader...
    result = m_shader_.addShaderFromSourceFile(QOpenGLShader::Fragment, fragmentShaderPath);
    if ( !result )
    qWarning() << m_shader_.log();

    // ...and finally we link them to resolve any references.
    result = m_shader_.link();
    if ( !result )
    qWarning() << "Could not link shader program:" << m_shader_.log();

    return result;
  }

  void GLWidget::setFileName(const std::string & name) {
    filename_ = name;
  }

  GLuint GLWidget::loadBmpImage(const std::string & imagepath) {
    unsigned char header[54];
    unsigned int model_Pos;
    unsigned int imageSize;
    unsigned int width, height;

    unsigned char * model_obj_;

    FILE * file = fopen(imagepath.c_str(), "rb");
    if (!file) {
      qDebug() << imagepath.c_str() << "could not be opened.";
      getchar();
      return 0;
    }
    // Read header
    // If less than 54 bytes are read, problem
    if (fread(header, 1, 54, file) != 54) {
      qDebug() << imagepath.c_str() <<"not a correct BMP file";
      return 0;
    }
    if (header[0] != 'B' || header[1] != 'M') {
      qDebug() << imagepath.c_str() <<"not a correct BMP file";
      return 0;
    }

    if (*reinterpret_cast<int*>(&header[0x1E]) != 0) {  // check it is a 24bpp file
      qDebug() << imagepath.c_str() <<"not a correct BMP file";
      return 0;
    }
    if (*reinterpret_cast<int*>(&header[0x1C]) != 24) {
      qDebug() << imagepath.c_str() <<"not a correct BMP file";
      return 0;
    }
    // Read the information about the image
    model_Pos    = *reinterpret_cast<int*>(&header[0x0A]);
    imageSize  = *reinterpret_cast<int*>(&header[0x22]);
    width      = *reinterpret_cast<int*>(&header[0x12]);
    height     = *reinterpret_cast<int*>(&header[0x16]);

    if (imageSize == 0) {
      imageSize = width * height * 3;  // one byte for each red, green and blue component
    }
    if (model_Pos == 0) {
      model_Pos = 54;
    }
    model_obj_ = new unsigned char[imageSize];
    fread(model_obj_, 1, imageSize, file);
    fclose(file);
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, model_obj_);
    delete [] model_obj_;
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);  // Not part of core openGL, disabled for now

    return textureID;
  }

  void GLWidget::timerTick() {
    light_rotation_ += 0.2f;
    if (light_rotation_ > 360.0f) {
      light_rotation_ -= 360.0f;
    }
    repaint();
  }

  void GLWidget::setNormalMapping(bool state) {
    normal_mapping_active_ = state;
    if (normal_mapping_active_) {
      qDebug() << "Normal mapping turned on.";
    } else {
      qDebug() << "Normal mapping turned off.";
    }
  }
