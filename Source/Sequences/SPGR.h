/*
 *  SPGR.h
 *
 *  Copyright (c) 2018 Tobias Wood.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  SPGR / FLASH / FFE Sequences
 *
 */

#ifndef SEQUENCES_SPGR_H
#define SEQUENCES_SPGR_H

#include "SequenceBase.h"

namespace QI {

struct SPGR : SequenceBase {
    double TR;
    Eigen::ArrayXd FA;

    size_t size() const override;
    Eigen::ArrayXcd signal(std::shared_ptr<Model> m, const Eigen::VectorXd &par) const override;
    template<typename Archive>
    void serialize(Archive &archive) {
        archive(CEREAL_NVP(TR), CEREAL_NVP(FA));
    }
};

struct SPGREcho : SequenceBase {
    double TR, TE;
    Eigen::ArrayXd FA;

    size_t size() const override;
    Eigen::ArrayXcd signal(std::shared_ptr<Model> m, const Eigen::VectorXd &par) const override;
    template<typename Archive>
    void serialize(Archive &archive) {
        archive(CEREAL_NVP(TR), CEREAL_NVP(TE), CEREAL_NVP(FA));
    }
};

struct SPGRFinite : SequenceBase {
    double TR, TE, Trf;
    Eigen::ArrayXd FA;

    size_t size() const override;
    Eigen::ArrayXcd signal(std::shared_ptr<Model> m, const Eigen::VectorXd &par) const override;
    template<typename Archive>
    void serialize(Archive &archive) {
        archive(CEREAL_NVP(TR), CEREAL_NVP(TE), CEREAL_NVP(Trf), CEREAL_NVP(FA));
    }
};

} // End namespace QI

CEREAL_REGISTER_TYPE(QI::SPGR);
CEREAL_REGISTER_POLYMORPHIC_RELATION(QI::SequenceBase, QI::SPGR);
CEREAL_REGISTER_TYPE(QI::SPGREcho);
CEREAL_REGISTER_POLYMORPHIC_RELATION(QI::SequenceBase, QI::SPGREcho);
CEREAL_REGISTER_TYPE(QI::SPGRFinite);
CEREAL_REGISTER_POLYMORPHIC_RELATION(QI::SequenceBase, QI::SPGRFinite);

#endif // SEQUENCES_SPGR_H
