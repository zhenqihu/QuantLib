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

/*! \file fdmweightedvalueop.hpp
    \brief weighted value operator
*/

#ifndef quantlib_fdm_weighted_value_op_hpp
#define quantlib_fdm_weighted_value_op_hpp

#include <ql/methods/finitedifferences/operators/triplebandlinearop.hpp>

namespace QuantLib {

    class FdmWeightedValueOp : public TripleBandLinearOp {
      public:
        FdmWeightedValueOp(Size direction,
                           const ext::shared_ptr<FdmMesher>& mesher);

        void setWeights(const Array& omega);
    };
}

#endif
