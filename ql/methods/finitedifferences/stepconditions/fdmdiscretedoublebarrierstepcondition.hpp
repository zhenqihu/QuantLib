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

/*! \file fdmdiscretedoublebarrierstepcondition.hpp
    \brief discrete double-barrier step condition for multi dimensional problems
*/

#ifndef quantlib_fdm_discrete_double_barrier_step_condition_hpp
#define quantlib_fdm_discrete_double_barrier_step_condition_hpp

#include <ql/time/daycounter.hpp>
#include <ql/methods/finitedifferences/stepcondition.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>

#include <vector>

namespace QuantLib {

    class FdmDiscreteDoubleBarrierStepCondition : public StepCondition<Array> {
      public:
        FdmDiscreteDoubleBarrierStepCondition(
            const std::vector<Date>& monitoringDates,
            const Date& referenceDate,
            const DayCounter& dayCounter,
            ext::shared_ptr<FdmMesher> mesher,
            Real lowerBarrier,
            Real upperBarrier,
            Real rebate = 0.0,
            Size direction = 0,
            bool logCoordinates = true);

        void applyTo(Array& a, Time t) const override;
        const std::vector<Time>& monitoringTimes() const;

      private:
        std::vector<Time> monitoringTimes_;
        const ext::shared_ptr<FdmMesher> mesher_;
        const Real lowerBarrier_, upperBarrier_, rebate_;
        const Size direction_;
        const bool logCoordinates_;
    };
}

#endif
