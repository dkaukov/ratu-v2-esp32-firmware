#pragma once

#include <Arduino.h>

namespace Model {

class TMatchModel {
private:
  double z1r, z2r, z3r, z1i, z2i, z3i;
  double zlr, zli;

  double qc1 = 2000.0;
  double qc2 = 2000.0;
  double qel = 100.0;
  double f;

public:
  void setZ1(double zr, double zi) {
    z1r = zr;
    z1i = zi;
  }

  void setZ2(double zr, double zi) {
    z2r = zr;
    z2i = zi;
  }

  void setZ3(double zr, double zi) {
    z3r = zr;
    z3i = zi;
  }

  void setZl(double zr, double zi) {
    zlr = zr;
    zli = zi;
  }

  void setFreq(double freq) {
    f = freq;
  }

  float getFreq() {
    return f;
  }

  double getRho() {
    double zinr, zini, zl2r, zl2i, tr, ti, t, r;
    tr = zlr + z3r + z2r;
    ti = zli + z3i + z2i;
    t = 1.0 / (tr * tr + ti * ti);
    zl2r = z2r * z2r - z2i * z2i;
    zl2i = 2.0 * z2r * z2i;
    zinr = z1r + z2r - (zl2r * tr + zl2i * ti) * t;
    zini = z1i + z2i - (zl2i * tr - zl2r * ti) * t;
    r = (zinr - 1.0) * (zinr - 1.0) + zini * zini;
    t = (zinr + 1.0) * (zinr + 1.0) + zini * zini;
    return sqrt(r / t);
  }

  double getSwr() {
    double rho = getRho();
    return (1.0 + rho) / (1.0 - rho);
  }

  double getLoss() {
    double zinr, zl2r, zl2i, tr, ti, t, z;
    tr = zlr + z3r + z2r;
    ti = zli + z3i + z2i;
    t = 1.0 / (tr * tr + ti * ti);
    zl2r = z2r * z2r - z2i * z2i;
    zl2i = 2.0 * z2r * z2i;
    zinr = z1r + z2r - (zl2r * tr + zl2i * ti) * t;
    z = z2r * z2r + z2i * z2i;
    t = (z3r + z2r + zlr) * (z3r + z2r + zlr) + (z3i + z2i + zli) * (z3i + z2i + zli);
    return 1.0 - zlr * z / (t * zinr);
  }

  void setComponents(double c1, double el, double c2) {
    double yr, yi, y;

    yi = 2.0 * PI * f * c1 * 1.e-6;
    yr = yi / qc1;
    y = 1.0 / (yr * yr + yi * yi);
    z1r = yr * y;
    z1i = -yi * y;

    yi = 2.0 * PI * f * c2 * 1.e-6;
    yr = yi / qc2;
    y = 1.0 / (yr * yr + yi * yi);
    z3r = yr * y;
    z3i = -yi * y;

    z2i = 2.0 * PI * f * el;
    z2r = z2i / qel;

    z1r *= .02;
    z1i *= .02;
    z2r *= .02;
    z2i *= .02;
    z3r *= .02;
    z3i *= .02;
  }

  void setLoad(double azlr, double azli) {
    zlr = azlr;
    zli = azli;

    zlr *= .02;
    zli *= .02;
  }

  static float estimateLoss(float frMhz, float c1Pf, float lMh, float qel) {
    float pi = 4.0 * atan(1);
    float xc = 1.0 / ((2.0 * pi * frMhz * c1Pf) / 1e6);
    float xl = 2.0 * pi * frMhz * lMh;
    float vlvi = sqrt(pow(50, 2) + pow(xc, 2)) / 50.0;
    return (pow(vlvi, 2.0) / xl / qel) * 50.0;
  }
};

} // namespace Model