// Copyright[2015] <Brian Mc George>
// MCGBRI004

#ifndef MODEL_H_
#define MODEL_H_
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
namespace mcgbri004 {
class Model {
 private:
  float x_angle_ = 0.0f;
  float y_angle_ = 0.0f;
  float z_angle_ = 0.0f;
  float scale_ = 1.0f;
  float scale_speed_ = 0.02f;
  float x_translation_ = 0.0f;
  float y_translation_ = 0.0f;
  float z_translation_ = 3.0f;
  float translate_speed_ = 0.005f;
  float rotate_speed_ = 0.02f;

 public:
  bool rotateX(const int & change);
  bool rotateY(const int & change);
  bool rotateZ(const int & change);
  bool scaleBy(const int & change);
  void translateX(const int & dx);
  void translateY(const int & dy);
  float get_scale();
  float get_x_angle();
  float get_y_angle();
  float get_z_angle();
  float get_x_translation();
  float get_y_translation();
  float get_z_translation();
  void reset();
};
}  // namespace mcgbri004
#endif  // MODEL_H_
