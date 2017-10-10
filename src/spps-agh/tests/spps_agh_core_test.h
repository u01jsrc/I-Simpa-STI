#pragma once

void IntegratePhongModel(core_mathlib::vec3 &specular, double phi_increment, double c_increment, int n, double diffusion, float solidAngle, core_mathlib::vec3 &position, core_mathlib::vec3 &faceNormal, double &result);
