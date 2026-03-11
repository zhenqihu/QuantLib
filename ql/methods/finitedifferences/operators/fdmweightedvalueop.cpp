/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/fdmweightedvalueop.hpp>

namespace QuantLib {

    FdmWeightedValueOp::FdmWeightedValueOp(
        Size direction,
        const ext::shared_ptr<FdmMesher>& mesher)
    : TripleBandLinearOp(direction, mesher) {
        setWeights(Array(1, 0.0));
    }

    void FdmWeightedValueOp::setWeights(const Array& omega) {
        const ext::shared_ptr<FdmLinearOpLayout> layout = mesher_->layout();
        const Size size = layout->size();

        QL_REQUIRE(omega.size() == 1 || omega.size() == size,
                   "inconsistent size of omega");

        const Size omegaInc = (omega.size() > 1) ? 1 : 0;
        const FdmLinearOpIterator endIter = layout->end();

        for (FdmLinearOpIterator iter = layout->begin(); iter != endIter; ++iter) {
            const Size i = iter.index();
            const Real w = omega[i*omegaInc];
            const Size co = iter.coordinates()[direction_];

            if (co == 0 || co == layout->dim()[direction_] - 1) {
                lower_[i] = upper_[i] = 0.0;
                diag_[i] = 1.0;
            }
            else {
                lower_[i] = 2.0*w;
                diag_[i] = 1.0 - 4.0*w;
                upper_[i] = 2.0*w;
            }
        }
    }
}
