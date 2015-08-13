// Copyright[2015] <Brian Mc George>
// MCGBRI004

#include "./model.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"

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
