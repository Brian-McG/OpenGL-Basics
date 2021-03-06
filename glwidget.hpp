// Copyright[2015] <Brian Mc George>
// MCGBRI004

#ifndef GLWIDGET_H_
#define GLWIDGET_H_

#include <string>
#include <vector>
#include "glm/glm.hpp"
#include "./glheaders.hpp"  // Must be included before QT opengl headers
#include <QGLWidget>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QTimer>
#include <memory>
#include "./model.hpp"

class GLWidget : public QGLWidget {
  Q_OBJECT

 public:
  explicit GLWidget(const QGLFormat& format, QWidget* parent = 0);
  struct ObjModel {
    int number_of_triangles = 0;
    std::unique_ptr<glm::vec4[]> vertices;
    std::unique_ptr<glm::vec3[]> normals;
    std::unique_ptr<glm::vec2[]> uvs;
    std::unique_ptr<int[]> texture_id;
    std::unique_ptr<glm::vec3[]> tangents;
    std::unique_ptr<glm::vec3[]> bitangents;
  };
  void reallocate_buffers();
  void reset();
  void setFileName(const std::string & name);
  GLuint loadBmpImage(const std::string & imagepath);
  void setNormalMapping(bool state);

 protected:
  virtual void initializeGL();
  virtual void resizeGL(int w, int h);
  virtual void paintGL();
  virtual void mouseMoveEvent(QMouseEvent* m);
  virtual void keyPressEvent(QKeyEvent* e);
  virtual void wheelEvent(QWheelEvent* e);

 private slots:
  void timerTick();

 private:
  void rotX(const int & change);
  void rotY(const int & change);
  void rotZ(const int & change);
  void scaleBy(const int & change);
  bool prepareShaderProgram(const QString& vertexShaderPath, const QString& fragmentShaderPath);

  QGLFormat desiredFormat();
  glm::mat4 projection_;
  glm::mat4 view_projection_;
  glm::mat4 model_projection_;
  glm::mat4 MVP;
  int window_width_;
  int window_height_;
  int last_cursor_x_;
  int last_cursor_y_;
  float light_rotation_ = 0.0f;
  QTimer redraw_timer_;
  char mode_ = -1;
  char rotation_mode_ = -1;
  bool normal_mapping_active_ = true;
  mcgbri004::Model model_;
  mcgbri004::Model::ObjModel model_obj_;
  std::unique_ptr<mcgbri004::Model> model_transform_;
  std::string filename_;
  GLuint texture_0_;
  GLuint texture_1_;
  GLuint normal_map_;
  QOpenGLShaderProgram m_shader_;
  QOpenGLBuffer m_vertex_buffer_;
  QOpenGLBuffer m_normal_buffer_;
  QOpenGLBuffer m_uv_buffer_;
  QOpenGLBuffer m_texture_id_buffer_;
  QOpenGLBuffer m_tangent_buffer_;
  QOpenGLBuffer m_bitangent_buffer_;

  // Lights
  // Rotating light 1
  glm::vec3 rotating_light_1_start_position_ = glm::vec3(0.0f, 0.0f, 10.0f);
  glm::vec4 ambprod_rotating_light_1_ = glm::vec4(0.01f, 0.01f, 0.01f, 1.0f);
  glm::vec4 diffprod_rotating_light_1_ = glm::vec4(0.4f, 0.0f, 0.0f, 1.0f);
  glm::vec4 specprod_rotating_light_1_ = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
  float shine_rotating_light_1_ = 500.0f;
  // Rotating light 2
  glm::vec3 rotating_light_2_start_position_ = glm::vec3(0.0f, 0.0f, 10.0f);
  glm::vec4 ambprod_rotating_light_2_ = glm::vec4(0.01f, 0.01f, 0.01f, 1.0f);
  glm::vec4 diffprod_rotating_light_2_ = glm::vec4(0.0f, 0.4f, 0.0f, 1.0f);
  glm::vec4 specprod_rotating_light_2_ = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
  float shine_rotating_light_2_ = 500.0f;
  // Rotating light 3
  glm::vec3 rotating_light_3_start_position_ = glm::vec3(-10.0f, 0.0f, 0.0f);
  glm::vec4 ambprod_rotating_light_3_ = glm::vec4(0.01f, 0.01f, 0.01f, 1.0f);
  glm::vec4 diffprod_rotating_light_3_ = glm::vec4(0.0f, 0.0f, 0.4f, 1.0f);
  glm::vec4 specprod_rotating_light_3_ = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
  float shine_rotating_light_3_ = 500.0f;
};
#endif  // GLWIDGET_H_
