/*
 *  EllipseHelpers.h
 *
 *  Copyright (c) 2016 Tobias Wood.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef QI_ELLIPSE_HELPERS_H
#define QI_ELLIPSE_HELPERS_H

#include <complex>
#include "Eigen/Core"

#include "Macro.h"
#include "ApplyTypes.h"
#include "Util.h"

namespace QI {

Eigen::ArrayXd Unwrap(const Eigen::ArrayXd &x);

void SemiaxesToHoff(const double A, const double B, const double c,
                    double &G, double &a, double &b);

void EllipseToMRI(const double a, const double b, const double c, const double th, const double TR, const double flip,
                  float &M0, float &T1, float &T2, float &df0, const bool debug = false);

/*
 * Convert the SSFP Ellipse parameters into a magnetization. Needs to be a template function for
 * automatic differentation in Ceres. For the same reason, return real and imaginary parts in
 * a concatenated array instead of as actual complex numbers
 */
template<typename T>
Eigen::Array<T, Eigen::Dynamic, 1> EllipseToSignal(const T &G, const T &a, const T &b,
                                                   const T &theta0, const T &psi0,
                                                   Eigen::ArrayXd const &phi) {
    typedef Eigen::Array<T, Eigen::Dynamic, 1> ArrayXT;
    const ArrayXT theta = theta0 - phi;
    const T psi = theta0/2.0 + psi0;
    const ArrayXT cos_th = cos(theta);
    const ArrayXT sin_th = sin(theta);
    const T cos_psi = cos(psi);
    const T sin_psi = sin(psi);
    const ArrayXT re_m = (cos_psi - a*(cos_th*cos_psi - sin_th*sin_psi)) * G / (1.0 - b*cos_th);
    const ArrayXT im_m = (sin_psi - a*(cos_th*sin_psi + sin_th*cos_psi)) * G / (1.0 - b*cos_th);
    ArrayXT result(re_m.rows() + im_m.rows()); result << re_m, im_m;
    return result;
}



} // End namespace QI

#endif // QI_ELLIPSEHELPERS_H