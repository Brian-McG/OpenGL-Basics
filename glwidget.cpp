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
#include "./glwidget.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"

#define VERT_SHADER ":/simple.vert"
#define FRAG_SHADER ":/simple.frag"
#define BUNNY_LOCATION "bunny.stl"
#define BUNNY_LOCATION_OBJ "bunny.obj"
#define F16 "/home/brian/Downloads/f16/f16.obj"

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
    m_normalBuffer.destroy();
    m_vertexBuffer.destroy();
    m_shader.removeAllShaders();
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
    if (GLWidget::filename == "") {
      loadObjImage(F16);
    } else {
      loadObjImage(GLWidget::filename);
    }
    qDebug() << "Obj load complete";

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

    // Vertex
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

    // Normals
    m_normalBuffer.create();
    m_normalBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (!m_normalBuffer.bind()) {
      qWarning() << "Could not bind normal buffer to the context";
      return;
    }
    m_normalBuffer.allocate(data.normals.get(), data.numTriangles * 3 * sizeof(glm::vec3));
    m_shader.bind();
    m_shader.enableAttributeArray("normal");
    m_shader.setAttributeBuffer("normal", GL_FLOAT, 0, 3);

    // UV
    m_uvBuffer.create();
    m_uvBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (!m_uvBuffer.bind()) {
      qWarning() << "Could not bind uv buffer to the context";
      return;
    }
    qDebug() << "UV's:" << data.uvs[0][0];
    m_uvBuffer.allocate(data.uvs.get(), data.numTriangles * 3 * sizeof(glm::vec2));
    m_shader.bind();
    m_shader.enableAttributeArray("uv");
    m_shader.setAttributeBuffer("uv", GL_FLOAT, 0, 2);

    // Texture ID
    m_texture_id.create();
    m_texture_id.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (!m_texture_id.bind()) {
      qWarning() << "Could not bind texture id to the context";
      return;
    }
    m_texture_id.allocate(data.texture_id.get(), data.numTriangles * 3 * sizeof(int));
    m_shader.bind();
    m_shader.enableAttributeArray("sampler_index");
    m_shader.setAttributeBuffer("sampler_index", GL_FLOAT, 0, 1);

    // Tangent
    m_tangent_buffer.create();
    m_tangent_buffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (!m_tangent_buffer.bind()) {
      qWarning() << "Could not bind normal buffer to the context";
      return;
    }
    m_tangent_buffer.allocate(data.tangents.get(), data.numTriangles * 3 * sizeof(glm::vec3));
    m_shader.bind();
    m_shader.enableAttributeArray("tangent");
    m_shader.setAttributeBuffer("tangent", GL_FLOAT, 0, 3);

    // Bitangent
    m_bitangent_buffer.create();
    m_bitangent_buffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (!m_bitangent_buffer.bind()) {
      qWarning() << "Could not bind normal buffer to the context";
      return;
    }
    m_bitangent_buffer.allocate(data.bitangents.get(), data.numTriangles * 3 * sizeof(glm::vec3));
    m_shader.bind();
    m_shader.enableAttributeArray("bitangent");
    m_shader.setAttributeBuffer("bitangent", GL_FLOAT, 0, 3);

    qDebug() << "Gets to bmp load";
    f16s = loadBmpImage("/home/brian/Downloads/f16/F16s.bmp");  // Hardcode for now -- Change this
    f16t = loadBmpImage("/home/brian/Downloads/f16/F16t.bmp");  // Hardcode for now -- Change this
    normal_map = loadBmpImage("/home/brian/Downloads/f16/normal_map.bmp");  // Hardcode for now -- Change this
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, f16s);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, f16t);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, normal_map);

    qDebug() << "Texture load complete";
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

  void GLWidget::reallocate_buffers() {
    initializeGL();
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
    std::ifstream file_stream;
    file_stream.open(file_name, std::ios_base::in | std::ios::binary);
    if (file_stream.fail()) {
      qWarning() << "An error occurred accessing the file does the file exist?";
      file_stream.close();
      data = std::move(GLWidget::stlData());
    }

    // Ignore header
    file_stream.ignore(80, EOF);

    // Number of triangles
    unsigned int numTriangles;
    file_stream.read(reinterpret_cast<char *>(&numTriangles), 4);
    if (file_stream.fail()) {
      qWarning() << "An error occurred reading from file";
      file_stream.close();
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
      file_stream.read(reinterpret_cast<char *>(&normX), 4);
      file_stream.read(reinterpret_cast<char *>(&normY), 4);
      file_stream.read(reinterpret_cast<char *>(&normZ), 4);
      file_stream.read(reinterpret_cast<char *>(&x), 4);
      file_stream.read(reinterpret_cast<char *>(&y), 4);
      file_stream.read(reinterpret_cast<char *>(&z), 4);
      if (file_stream.fail()) {
        qWarning() << "An error occurred reading from file";
        file_stream.close();
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
      file_stream.read(reinterpret_cast<char *>(&x), 4);
      file_stream.read(reinterpret_cast<char *>(&y), 4);
      file_stream.read(reinterpret_cast<char *>(&z), 4);
      if (file_stream.fail()) {
        qWarning() << "An error occurred reading from file";
        file_stream.close();
        data = std::move(GLWidget::stlData());
        m_vertexBuffer.allocate( data.vertices.get(), data.numTriangles * 3 * sizeof(glm::vec4) );
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
        data = std::move(GLWidget::stlData());
        m_vertexBuffer.allocate( data.vertices.get(), data.numTriangles * 3 * sizeof(glm::vec4) );
        return;
      }
      model.vertices[triCounter+2] = glm::vec4(x, y, z, 1.0f);

      // Attribute byte count
      file_stream.ignore(2, EOF);

      triCounter+=3;
    }
    data = std::move(model);
    return;
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
      } else if (line.substr(0, 3) == "vt ") {
        line = line.substr(2);
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
    GLWidget::stlData model;  // Change this -- this is just an inital pass
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
      }
      model.normals[i] = temp_normals[normal_indices[i] - 1];
      model.texture_id[i] = texture_indices[i];
    }
    setTangentVectors(model);
    data = std::move(model);
    qDebug() << "Complete loading obj";
    return true;
  }

  void GLWidget::setTangentVectors(stlData & model) {
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
      qDebug() << glm::dot(normal, tangent);
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
    glUniform3f(glGetUniformLocation(m_shader.programId(), "rotating_light"), rotated_light[0], rotated_light[1], rotated_light[2]);
    glm::vec4 ambprod_rotating_light = glm::vec4(0.00f, 0.00f, 0.00f, 1.0f);
    glm::vec4 diffprod_rotating_light = glm::vec4(0.0f, 0.3f, 0.0f, 1.0f);
    glm::vec4 specprod_rotating_light = glm::vec4(0.0f, 0.6f, 0.0f, 1.0f);
    glUniform4f(glGetUniformLocation(m_shader.programId(), "ambprod_rotating_light"), ambprod_rotating_light[0], ambprod_rotating_light[1], ambprod_rotating_light[2], ambprod_rotating_light[3]);
    glUniform4f(glGetUniformLocation(m_shader.programId(), "diffprod_rotating_light"), diffprod_rotating_light[0], diffprod_rotating_light[1], diffprod_rotating_light[2], diffprod_rotating_light[3]);
    glUniform4f(glGetUniformLocation(m_shader.programId(), "specprod_rotating_light"), specprod_rotating_light[0], specprod_rotating_light[1], specprod_rotating_light[2], specprod_rotating_light[3]);
    glUniform1f(glGetUniformLocation(m_shader.programId(), "shine_rotating_light"), 500.0);

    glm::vec3 static_light(0.0f, 0.0f, 10.0f);
    glUniform3f(glGetUniformLocation(m_shader.programId(), "static_light"), static_light[0], static_light[1], static_light[2]);
    glm::vec4 ambprod_static_light = glm::vec4(0.01, 0.01, 0.01, 1.0);
    glm::vec4 diffprod_static_light = glm::vec4(0.6, 0.6, 0.6, 1.0);
    glm::vec4 specprod_static_light = glm::vec4(1, 1, 1, 1.0);
    glUniform4f(glGetUniformLocation(m_shader.programId(), "ambprod_static_light"), ambprod_static_light[0], ambprod_static_light[1], ambprod_static_light[2], ambprod_static_light[3]);
    glUniform4f(glGetUniformLocation(m_shader.programId(), "diffprod_static_light"), diffprod_static_light[0], diffprod_static_light[1], diffprod_static_light[2], diffprod_static_light[3]);
    glUniform4f(glGetUniformLocation(m_shader.programId(), "specprod_static_light"), specprod_static_light[0], specprod_static_light[1], specprod_static_light[2], specprod_static_light[3]);
    glUniform1f(glGetUniformLocation(m_shader.programId(), "shine_static_light"), 100.0);

    GLuint MatrixID = glGetUniformLocation(m_shader.programId(), "MVP");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

    GLuint modelID = glGetUniformLocation(m_shader.programId(), "model");
    glUniformMatrix4fv(modelID, 1, GL_FALSE, &Model[0][0]);

    GLuint texture_id_16s  = glGetUniformLocation(m_shader.programId(), "texture_sampler_f16s");
    glUniform1i(texture_id_16s, 0);
    GLuint texture_id_f16t  = glGetUniformLocation(m_shader.programId(), "texture_sampler_f16t");
    glUniform1i(texture_id_f16t, 1);
    GLuint texture_id_normal  = glGetUniformLocation(m_shader.programId(), "normal_texture_sampler");
    glUniform1i(texture_id_normal, 2);

    // Clear the buffer with the current clearing color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw triangles
    glDrawArrays(GL_TRIANGLES, 0, data.numTriangles * 3);
  }

  QGLFormat GLWidget::desiredFormat() {
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

  bool GLWidget::prepareShaderProgram(const QString& vertexShaderPath, const QString& fragmentShaderPath) {
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

  void GLWidget::setFileName(const std::string & name) {
    filename = name;
  }

  // Currently using an existing implementation to save time
  // http://stackoverflow.com/a/20596072
  GLuint GLWidget::loadBmpImage(const std::string & imagepath) {
    qDebug() << "reading image" << imagepath.c_str();

    // Data read from the header of the BMP file
    unsigned char header[54];
    unsigned int dataPos;
    unsigned int imageSize;
    unsigned int width, height;
    // Actual RGB data
    unsigned char * data;

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
    dataPos    = *reinterpret_cast<int*>(&header[0x0A]);
    imageSize  = *reinterpret_cast<int*>(&header[0x22]);
    width      = *reinterpret_cast<int*>(&header[0x12]);
    height     = *reinterpret_cast<int*>(&header[0x16]);

    // Some BMP files are misformatted, guess missing information
    if (imageSize == 0) {
      imageSize = width * height * 3;  // 3 : one byte for each Red, Green and Blue component
    }
    if (dataPos == 0) {
      dataPos = 54;                    // The BMP header is done that way
    }

    // Create a buffer
    data = new unsigned char[imageSize];

    // Read the actual data from the file into the buffer
    fread(data, 1, imageSize, file);

    // Everything is in memory now, the file wan be closed
    fclose(file);

    // Create one OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Give the image to OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

    // OpenGL has now copied the data. Free our own version
    delete [] data;

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);  // Not part of core openGL, disabled for now

    // Return the ID of the texture we just created
    return textureID;
  }
