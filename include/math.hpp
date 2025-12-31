#pragma once
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "glm/geometric.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/trigonometric.hpp"

using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat4 = glm::mat4;
using mat2 = glm::mat2;

inline float random_range(float min, float max) { return min + (float(rand()) / float(RAND_MAX)) * (max - min); }
