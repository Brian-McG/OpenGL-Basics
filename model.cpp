// Copyright[2015] <Brian Mc George>
// MCGBRI004

#include <memory>
#include <utility>
#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include "./model.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glheaders.hpp"

mcgbri004::Model::ObjModel mcgbri004::Model::loadObjImage(const std::string & file_name) {
  std::vector< unsigned int > vertex_indices, uv_indices, normal_indices, texture_indices;
  std::vector< glm::vec3 > temp_vertices;
  std::vector< glm::vec2 > temp_uvs;
  std::vector< glm::vec3 > temp_normals;
  unsigned int texture_index = 0;
  unsigned int texture_index_count = 0;
  std::unordered_map<std::string, int> texture_map;
  std::ifstream file_stream(file_name);
  if (file_stream.fail()) {
    qWarning() << "An error occurred accessing the file does the file exist?";
    return mcgbri004::Model::ObjModel();
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
      for (int i = 0; i < 3; ++i) {
        texture_indices.push_back(texture_index);
        vertex_set >> vertex_values;
        unsigned int count = 0;
        size_t index = vertex_values.find("/");
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
        texture_index = texture_index_count;
      } else {
        texture_index = got->second;
      }
    } else {
      // Not yet supported
    }
  }
  Model::ObjModel model;
  model.number_of_triangles = vertex_indices.size() / 3;
  model.normals = std::unique_ptr<glm::vec3[]>(new glm::vec3[model.number_of_triangles * 3]);
  model.vertices = std::unique_ptr<glm::vec4[]>(new glm::vec4[model.number_of_triangles * 3]);
  model.uvs = std::unique_ptr<glm::vec2[]>(new glm::vec2[model.number_of_triangles * 3]);
  model.texture_id = std::unique_ptr<int[]>(new int[model.number_of_triangles * 3]);
  model.tangents = std::unique_ptr<glm::vec3[]>(new glm::vec3[model.number_of_triangles * 3]);
  model.bitangents = std::unique_ptr<glm::vec3[]>(new glm::vec3[model.number_of_triangles * 3]);
  for (unsigned int i = 0; i < vertex_indices.size(); ++i) {
    model.vertices[i] = glm::vec4(temp_vertices[vertex_indices[i] - 1], 1.0f);
    if (i < uv_indices.size()) {
      model.uvs[i] = temp_uvs[uv_indices[i] - 1];
    }
    model.normals[i] = temp_normals[normal_indices[i] - 1];
    model.texture_id[i] = texture_indices[i];
  }
  setTangentVectors(model);
  qDebug() << "Complete loading obj";
  return model;
}

void mcgbri004::Model::setTangentVectors(ObjModel & model) {
  for (unsigned int i = 0; i < model.number_of_triangles * 3; i += 3) {
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
  for (unsigned int i = 0; i < model.number_of_triangles * 3; ++i) {
    glm::vec3 & normal = model.normals[i];
    glm::vec3 & tangent = model.tangents[i];
    glm::vec3 & bitangent = model.bitangents[i];
    tangent = glm::normalize(tangent - normal * glm::dot(normal, tangent));
    if (glm::dot(glm::cross(normal, tangent), bitangent) < 0.0f) {
      tangent = tangent * -1.0f;
    }
  }
}

bool mcgbri004::Model::rotateX(const int & change) {
  if (change != 0) {
    x_angle_ += rotate_speed_ * change;
    if (x_angle_ > 360.0f) {
      x_angle_ -= 360.0f;
    }
    if (x_angle_ < -360.0f) {
      x_angle_ += 360.0f;
    }
    return true;
  }
  return false;
}

bool mcgbri004::Model::rotateY(const int & change) {
  if (change != 0) {
    y_angle_ += rotate_speed_ * change;
    if (y_angle_ > 360.0f) {
      y_angle_ -= 360.0f;
    }
    if (y_angle_ < -360.0f) {
      y_angle_ += 360.0f;
    }
    return true;
  }
  return false;
}

bool mcgbri004::Model::rotateZ(const int & change) {
  if (change != 0) {
    z_angle_ += rotate_speed_ * change;
    if (z_angle_ > 360.0f) {
      z_angle_ -= 360.0f;
    }
    if (z_angle_ < -360.0f) {
      z_angle_ += 360.0f;
    }
    return true;
  }
  return false;
}

bool mcgbri004::Model::scaleBy(const int & change) {
  if (change != 0) {
    scale_ -= change * scale_speed_;
    if (scale_ < 0.0f) {
      scale_ = 0.0f;
    }
    return true;
  }
  return false;
}

void mcgbri004::Model::translateX(const int & dx) {
  x_translation_ += dx * translate_speed_;
}

void mcgbri004::Model::translateY(const int & dy) {
  y_translation_ += dy * translate_speed_;
}

void mcgbri004::Model::reset() {
  x_angle_ = 0.0f;
  y_angle_ = 0.0f;
  z_angle_ = 0.0f;
  scale_ = 1.0f;
  x_translation_ = 0.0f;
  y_translation_ = 0.0f;
  z_translation_ = 3.0f;
}

float mcgbri004::Model::get_scale() {
  return scale_;
}

float mcgbri004::Model::get_x_angle() {
  return x_angle_;
}

float mcgbri004::Model::get_y_angle() {
  return y_angle_;
}

float mcgbri004::Model::get_z_angle() {
  return z_angle_;
}

float mcgbri004::Model::get_x_translation() {
  return x_translation_;
}

float mcgbri004::Model::get_y_translation() {
  return y_translation_;
}

float mcgbri004::Model::get_z_translation() {
  return z_translation_;
}
