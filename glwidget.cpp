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
  // Resolve OpenGL functions
  glewExperimental = true;
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_ALPHA_TEST);
    /*
  glDepthFunc(GL_LESS);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  */
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
    std::string texture_1_name;
    std::string texture_2_name;
    std::string normal_map_name;
    if (filename_ == "" || filename_ == "Cube_KEYPRESS") {
      loadObjImage(CUBE_OBJ);
      texture_1_name = CUBE_TEXTURE;
      texture_2_name = F16T_texture;
      normal_map_name = CUBE_NORMAL;
    } else if (filename_ == "F16_KEYPRESS") {
      loadObjImage(F16);
      texture_1_name = F16S_texture;
      texture_2_name = F16T_texture;
      normal_map_name = F16_normal_map;
    } else {
      loadObjImage(GLWidget::filename_);
    }
    qDebug() << "Obj load complete";
    model_transform_ = std::unique_ptr<mcgbri004::Model>(new mcgbri004::Model());

    // If bunny fails to load, load a placeholder cube
    if (model_.numTriangles == 0) {
      qDebug() << "The bunny failed to load, loading a placeholder cube instead.";
      model_ = GLWidget::ObjModel();
      model_.numTriangles = 1;
      model_.normals =  std::unique_ptr<glm::vec3[]>(new glm::vec3[model_.numTriangles * 3]);
      model_.vertices = std::unique_ptr<glm::vec4[]>(new glm::vec4[model_.numTriangles*3]);
      model_.vertices[0] = glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f);
      model_.vertices[1] = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
      model_.vertices[2] = glm::vec4(1.0f, -1.0f, 0.0f, 1.0f);
      model_.normals[0] = glm::vec3(0.0f, 0.0f, 1.0f);
      model_.normals[1] = glm::vec3(0.0f, 0.0f, 1.0f);
      model_.normals[2] = glm::vec3(0.0f, 0.0f, 1.0f);
    }

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
    m_vertex_buffer_.allocate(model_.vertices.get(), model_.numTriangles * 3 * sizeof(glm::vec4));

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
    m_normal_buffer_.allocate(model_.normals.get(), model_.numTriangles * 3 * sizeof(glm::vec3));
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
    qDebug() << "UV's:" << model_.uvs[0][0];
    m_uv_buffer_.allocate(model_.uvs.get(), model_.numTriangles * 3 * sizeof(glm::vec2));
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
    m_texture_id_buffer_.allocate(model_.texture_id.get(), model_.numTriangles * 3 * sizeof(int));
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
    m_tangent_buffer_.allocate(model_.tangents.get(), model_.numTriangles * 3 * sizeof(glm::vec3));
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
    m_bitangent_buffer_.allocate(model_.bitangents.get(), model_.numTriangles * 3 * sizeof(glm::vec3));
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

  bool GLWidget::loadObjImage(const std::string & file_name) {
    std::vector< unsigned int > vertex_indices, uv_indices, normal_indices, texture_indices;
    std::vector< glm::vec3 > temp_vertices;
    std::vector< glm::vec2 > temp_uvs;
    std::vector< glm::vec3 > temp_normals;
    std::cout << file_name << std::endl;
    unsigned int texture_index = 0;
    unsigned int texture_index_count = 0;
    std::unordered_map<std::string, int> texture_map;
    std::ifstream file_stream(file_name);
    if (file_stream.fail()) {
      qWarning() << "An error occurred accessing the file does the file exist?";
      // return false;
    }
    std::string line;
    while (getline(file_stream, line)) {
      if (line.substr(0, 2) == "v ") {
        line = line.substr(2);
        std::stringstream ss(line);
        float x, y, z;
        ss >> x;
        ss >> y;
        ss >> z;
        temp_vertices.push_back(glm::vec3(x, y, z));
      }
      else if (line.substr(0, 3) == "vt ") {
        line = line.substr(3);
        std::stringstream ss(line);
        float x, y;
        ss >> x;
        ss >> y;
        temp_uvs.push_back(glm::vec2(x, y));
      } else if (line.substr(0, 3) == "vn ") {
        line = line.substr(2);
        std::stringstream ss(line);
        float x, y, z;
        ss >> x;
        ss >> y;
        ss >> z;
        temp_normals.push_back(glm::vec3(x, y, z));
      } else if (line.substr(0, 2) == "f ") {
        line = line.substr(2);
        std::istringstream vertex_set(line);
        std::string vertex_values;
        unsigned int vertex_index[3], uv_index[3], normal_index[3];
        for (int i = 0; i < 3; ++i) {
          texture_indices.push_back(texture_index);
          vertex_set >> vertex_values;
          unsigned int count = 0;
          int index = vertex_values.find("/");
          while (index != std::string::npos || vertex_values != "") {
            std::string first_value;
            if (index != std::string::npos) {
              first_value = vertex_values.substr(0, index);
              vertex_values.erase(0, index + 1);
            } else {
              first_value = vertex_values;
              vertex_values = "";
            }
            if (first_value != "") {
              std::stringstream ss(first_value);
              unsigned int value;

              ss >> value;
              if (count == 0) {
                vertex_indices.push_back(value);
              } else if (count == 1) {
                uv_indices.push_back(value);
              } else if (count == 2) {
                normal_indices.push_back(value);
              }
            }
            ++count;
            index = vertex_values.find("/");
          }
        }
      } else if (line.length() > 7 && line.substr(0, 7) == "usemtl ") {
        line = line.substr(7);

        std::unordered_map<std::string, int>::const_iterator got = texture_map.find(line);
        if (got == texture_map.end()) {
          ++texture_index_count;
          texture_map.insert(std::pair<std::string, int>(line, texture_index_count));
          qDebug() << texture_index_count << "First";
          texture_index = texture_index_count;
        } else {
          texture_index = got->second;
        }
      } else {
        // Not yet supported
      }
    }
    GLWidget::ObjModel model;  // Change this -- this is just an inital pass
    model.numTriangles = vertex_indices.size() / 3;
    qDebug() << "Number of Triangles" << model.numTriangles;
    model.normals = std::unique_ptr<glm::vec3[]>(new glm::vec3[model.numTriangles * 3]);
    model.vertices = std::unique_ptr<glm::vec4[]>(new glm::vec4[model.numTriangles * 3]);
    model.uvs = std::unique_ptr<glm::vec2[]>(new glm::vec2[model.numTriangles * 3]);
    model.texture_id = std::unique_ptr<int[]>(new int[model.numTriangles * 3]);
    model.tangents = std::unique_ptr<glm::vec3[]>(new glm::vec3[model.numTriangles * 3]);
    model.bitangents = std::unique_ptr<glm::vec3[]>(new glm::vec3[model.numTriangles * 3]);
    for (unsigned int i = 0; i < vertex_indices.size(); ++i) {
      model.vertices[i] = glm::vec4(temp_vertices[vertex_indices[i] - 1], 1.0f);
      if (i < uv_indices.size()) {
        model.uvs[i] = temp_uvs[uv_indices[i] - 1];
      }      model.normals[i] = temp_normals[normal_indices[i] - 1];
      model.texture_id[i] = texture_indices[i];
    }
    setTangentVectors(model);
    model_ = std::move(model);
    qDebug() << "Complete loading obj";
    return true;
  }

  void GLWidget::setTangentVectors(ObjModel & model) {
    /*
    for(unsigned int i = 0; i < model.numTriangles * 3; i += 3) {
      glm::vec4 & v0 = model.vertices[i];
      glm::vec4 & v1 = model.vertices[i + 1];
      glm::vec4 & v2 = model.vertices[i + 2];

      glm::vec2 & u0 = model.uvs[i];
      glm::vec2 & u1 = model.uvs[i + 1];
      glm::vec2 & u2 = model.uvs[i + 2];

      glm::vec3 e1 = glm::vec3(v1) - glm::vec3(v0);
      glm::vec3 e2 = glm::vec3(v2) - glm::vec3(v0);

      float deltaU1 = u1.x - u0.x;
      float deltaV1 = u1.y - u0.y;
      float deltaU2 = u2.x - u0.x;
      float deltaV2 = u2.y - u0.y;

      float f = 1.0f / (deltaU1 * deltaV2 - deltaU2 * deltaV1);

      glm::vec3 tangent, bitangent;
      tangent.x = f * (deltaV2 * e1.x - deltaV1 * e2.x);
      tangent.x = f * (deltaV2 * e1.y - deltaV1 * e2.y);
      tangent.x = f * (deltaV2 * e1.z - deltaV1 * e2.z);

      bitangent.x = f * (deltaU2 * e1.x - deltaU1 * e2.x);
      bitangent.x = f * (deltaU2 * e1.y - deltaU1 * e2.y);
      bitangent.x = f * (deltaU2 * e1.z - deltaU1 * e2.z);

      model.tangents[i] = tangent;
      model.tangents[i + 1] = tangent;
      model.tangents[i + 2] = tangent;

      model.bitangents[i] = bitangent;
      model.bitangents[i + 1] = bitangent;
      model.bitangents[i + 2] = bitangent;
    }

    for(unsigned int i = 0; i < model.numTriangles * 3; ++i) {
      model.tangents[i] = glm::normalize(model.tangents[i]);
    }
    */
    for (unsigned int i = 0; i < model.numTriangles * 3; i += 3) {
      glm::vec3 delta_pos_1 = glm::vec3(model.vertices[i + 1].x, model.vertices[i + 1].y, model.vertices[i + 1].z) - glm::vec3(model.vertices[i].x, model.vertices[i].y, model.vertices[i].z);
      glm::vec3 delta_pos_2 = glm::vec3(model.vertices[i + 2].x, model.vertices[i + 2].y, model.vertices[i + 2].z) - glm::vec3(model.vertices[i].x, model.vertices[i].y, model.vertices[i].z);
      glm::vec2 delta_uv_1 = model.uvs[i + 1] - model.uvs[i];
      glm::vec2 delta_uv_2 = model.uvs[i + 2] - model.uvs[i];
      float r = 1.0f / (delta_uv_1.x * delta_uv_2.y - delta_uv_1.y * delta_uv_2.x);
      glm::vec3 tangent = (delta_pos_1 * delta_uv_2.y - delta_pos_2 * delta_uv_1.y) * r;
      glm::vec3 bitangent = (delta_pos_2 * delta_uv_1.x - delta_pos_1 * delta_uv_2.x) * r;
      for (unsigned int g = 0; g < 3; ++g) {
        model.tangents[i + g] = tangent;
        model.bitangents[i + g] = bitangent;
      }
    }
    for (unsigned int i = 0; i < model.numTriangles * 3; ++i) {
      glm::vec3 & normal = model.normals[i];
      glm::vec3 & tangent = model.tangents[i];
      glm::vec3 & bitangent = model.bitangents[i];
      tangent = glm::normalize(tangent - normal * glm::dot(normal, tangent));
      if (glm::dot(glm::cross(normal, tangent), bitangent) < 0.0f) {
        tangent = tangent * -1.0f;
      }
    }
  }

  void GLWidget::resizeGL(int w, int h) {
    window_width_ = w;
    window_height_ = h;
    // Set the viewport to window dimensions
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
    glUniform3f(glGetUniformLocation(m_shader_.programId(), "rotating_light"), rotated_light_1[0], rotated_light_1[1], rotated_light_1[2]);
    glUniform4f(glGetUniformLocation(m_shader_.programId(), "ambprod_rotating_light"), ambprod_rotating_light_[0], ambprod_rotating_light_[1], ambprod_rotating_light_[2], ambprod_rotating_light_[3]);
    glUniform4f(glGetUniformLocation(m_shader_.programId(), "diffprod_rotating_light"), diffprod_rotating_light_[0], diffprod_rotating_light_[1], diffprod_rotating_light_[2], diffprod_rotating_light_[3]);
    glUniform4f(glGetUniformLocation(m_shader_.programId(), "specprod_rotating_light"), specprod_rotating_light_[0], specprod_rotating_light_[1], specprod_rotating_light_[2], specprod_rotating_light_[3]);
    glUniform1f(glGetUniformLocation(m_shader_.programId(), "shine_rotating_light"), 500.0);

    // Static light
    glm::vec3 rotated_light_2 = glm::mat3x3(glm::vec3(1, 0, 0), glm::vec3(0, glm::cos(radian_rotation), -glm::sin(radian_rotation)), glm::vec3(0, glm::sin(radian_rotation), glm::cos(radian_rotation))) * rotating_light_2_start_position_;
    glUniform3f(glGetUniformLocation(m_shader_.programId(), "static_light"), rotated_light_2[0], rotated_light_2[1], rotated_light_2[2]);
    glUniform4f(glGetUniformLocation(m_shader_.programId(), "ambprod_static_light"), ambprod_static_light_[0], ambprod_static_light_[1], ambprod_static_light_[2], ambprod_static_light_[3]);
    glUniform4f(glGetUniformLocation(m_shader_.programId(), "diffprod_static_light"), diffprod_static_light_[0], diffprod_static_light_[1], diffprod_static_light_[2], diffprod_static_light_[3]);
    glUniform4f(glGetUniformLocation(m_shader_.programId(), "specprod_static_light"), specprod_static_light_[0], specprod_static_light_[1], specprod_static_light_[2], specprod_static_light_[3]);
    glUniform1f(glGetUniformLocation(m_shader_.programId(), "shine_static_light"), 500.0f);

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
    glDrawArrays(GL_TRIANGLES, 0, model_.numTriangles * 3);
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
      } else if (mode_ == 2) {
        // translateZ += change * translateSpeed;  // Disabled
        // repaint();
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
      normal_mapping_active_ = !normal_mapping_active_;
      if (normal_mapping_active_) {
        qDebug() << "Normal mapping turned on.";
      } else {
        qDebug() << "Normal mapping turned off.";
      }
      break;

    case Qt::Key_1:
    setFileName("Cube_KEYPRESS");
    reallocate_buffers();
    break;

    case Qt::Key_2:
    setFileName("F16_KEYPRESS");
    reallocate_buffers();
    break;

      // Color from assignment 2 removed
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

  // Currently using an existing implementation to save time
  GLuint GLWidget::loadBmpImage(const std::string & imagepath) {
    qDebug() << "reading image" << imagepath.c_str();

    // model_ read from the header of the BMP file
    unsigned char header[54];
    unsigned int model_Pos;
    unsigned int imageSize;
    unsigned int width, height;
    // Actual RGB model_
    unsigned char * model_;

    // Open the file
    FILE * file = fopen(imagepath.c_str(), "rb");
    if (!file) {
      qDebug() << imagepath.c_str() << "could not be opened.";
      getchar();
      return 0;
    }
    // Read the header, i.e. the 54 first bytes
    // If less than 54 bytes are read, problem
    if (fread(header, 1, 54, file) != 54) {
      qDebug() << imagepath.c_str() <<"not a correct BMP file";
      return 0;
    }
    // A BMP files always begins with "BM"
    if (header[0] != 'B' || header[1] != 'M') {
      qDebug() << imagepath.c_str() <<"not a correct BMP file";
      return 0;
    }
    // Make sure this is a 24bpp file
    if (*reinterpret_cast<int*>(&header[0x1E]) != 0) {
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

    // Some BMP files are misformatted, guess missing information
    if (imageSize == 0) {
      imageSize = width * height * 3;  // 3 : one byte for each Red, Green and Blue component
    }
    if (model_Pos == 0) {
      model_Pos = 54;                    // The BMP header is done that way
    }

    // Create a buffer
    model_ = new unsigned char[imageSize];

    // Read the actual model_ from the file into the buffer
    fread(model_, 1, imageSize, file);

    // Everything is in memory now, the file wan be closed
    fclose(file);

    // Create one OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Give the image to OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, model_);

    // OpenGL has now copied the model_. Free our own version
    delete [] model_;

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);  // Not part of core openGL, disabled for now

    // Return the ID of the texture we just created
    return textureID;
  }

  void GLWidget::timerTick() {
    light_rotation_ += 0.2f;
    if (light_rotation_ > 360.0f) {
      light_rotation_ -= 360.0f;
    }
    repaint();
  }

  // Not used in Assigment-4
  void GLWidget::loadSTLFile(const std::string & file_name) {
    std::ifstream file_stream;
    file_stream.open(file_name, std::ios_base::in | std::ios::binary);
    if (file_stream.fail()) {
      qWarning() << "An error occurred accessing the file does the file exist?";
      file_stream.close();
      model_ = std::move(GLWidget::ObjModel());
    }

    // Ignore header
    file_stream.ignore(80, EOF);

    // Number of triangles
    unsigned int numTriangles;
    file_stream.read(reinterpret_cast<char *>(&numTriangles), 4);
    if (file_stream.fail()) {
      qWarning() << "An error occurred reading from file";
      file_stream.close();
      model_ = std::move(GLWidget::ObjModel());
      return;
    }

    GLWidget::ObjModel model;
    model.numTriangles = numTriangles;
    qDebug() << "Number of triangles:" << numTriangles;
    model.normals = std::unique_ptr<glm::vec3[]>(new glm::vec3[numTriangles * 3]);
    model.vertices = std::unique_ptr<glm::vec4[]>(new glm::vec4[numTriangles * 3]);
    unsigned int triCounter = 0;
    for (unsigned int i = 0; i < numTriangles; ++i) {
      float normX, normY, normZ, x, y, z;
      file_stream.read(reinterpret_cast<char *>(&normX), 4);
      file_stream.read(reinterpret_cast<char *>(&normY), 4);
      file_stream.read(reinterpret_cast<char *>(&normZ), 4);
      file_stream.read(reinterpret_cast<char *>(&x), 4);
      file_stream.read(reinterpret_cast<char *>(&y), 4);
      file_stream.read(reinterpret_cast<char *>(&z), 4);
      if (file_stream.fail()) {
        qWarning() << "An error occurred reading from file";
        file_stream.close();
        model_ = std::move(GLWidget::ObjModel());
        m_vertex_buffer_.allocate( model_.vertices.get(), model_.numTriangles * 3 * sizeof(glm::vec4) );
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
      file_stream.read(reinterpret_cast<char *>(&x), 4);
      file_stream.read(reinterpret_cast<char *>(&y), 4);
      file_stream.read(reinterpret_cast<char *>(&z), 4);
      if (file_stream.fail()) {
        qWarning() << "An error occurred reading from file";
        file_stream.close();
        model_ = std::move(GLWidget::ObjModel());
        m_vertex_buffer_.allocate( model_.vertices.get(), model_.numTriangles * 3 * sizeof(glm::vec4) );
        return;
      }
      model.vertices[triCounter+1] = glm::vec4(x, y, z, 1.0f);

      // Vertex 3
      x = 0.0f;
      y = 0.0f;
      z = 0.0f;
      file_stream.read(reinterpret_cast<char *>(&x), 4);
      file_stream.read(reinterpret_cast<char *>(&y), 4);
      file_stream.read(reinterpret_cast<char *>(&z), 4);
      if (file_stream.fail()) {
        qWarning() << "An error occurred reading from file";
        file_stream.close();
        model_ = std::move(GLWidget::ObjModel());
        m_vertex_buffer_.allocate( model_.vertices.get(), model_.numTriangles * 3 * sizeof(glm::vec4) );
        return;
      }
      model.vertices[triCounter+2] = glm::vec4(x, y, z, 1.0f);

      // Attribute byte count
      file_stream.ignore(2, EOF);

      triCounter+=3;
    }
    model_ = std::move(model);
    return;
  }
