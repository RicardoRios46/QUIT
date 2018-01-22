/*
 *  SpinEcho.cpp
 *
 *  Copyright (c) 2016 Tobias Wood.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include "SpinEcho.h"
#include "Util.h"
#include "IO.h"

using namespace std;
using namespace Eigen;

namespace QI {

ArrayXcd MultiEcho::signal(shared_ptr<Model> m, const VectorXd &p) const {
    return m->MultiEcho(p, TE, TR);
}

} // End namespace QI
