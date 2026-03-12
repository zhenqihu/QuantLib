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

#include <algorithm>
#include <cmath>
#include <utility>

#include <ql/errors.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmdiscretedoublebarrierstepcondition.hpp>

namespace QuantLib {

    FdmDiscreteDoubleBarrierStepCondition::
    FdmDiscreteDoubleBarrierStepCondition(
        const std::vector<Date>& monitoringDates,
        const Date& referenceDate,
        const DayCounter& dayCounter,
        ext::shared_ptr<FdmMesher> mesher,
        Real lowerBarrier,
        Real upperBarrier,
        Real rebate,
        Size direction,
        bool logCoordinates)
    : mesher_(std::move(mesher)), lowerBarrier_(lowerBarrier),
      upperBarrier_(upperBarrier), rebate_(rebate), direction_(direction),
      logCoordinates_(logCoordinates) {

        QL_REQUIRE(mesher_ != nullptr, "null mesher given");
        QL_REQUIRE(lowerBarrier_ > 0.0, "low barrier must be positive");
        QL_REQUIRE(upperBarrier_ > 0.0, "high barrier must be positive");
        QL_REQUIRE(lowerBarrier_ < upperBarrier_,
                   "low barrier must be below high barrier");
        QL_REQUIRE(direction_ < mesher_->layout()->dim().size(),
                   "direction (" << direction_ << ") out of range");

        monitoringTimes_.reserve(monitoringDates.size());
        for (const auto& monitoringDate : monitoringDates) {
            monitoringTimes_.push_back(
                dayCounter.yearFraction(referenceDate, monitoringDate));
        }

        std::sort(monitoringTimes_.begin(), monitoringTimes_.end());
        monitoringTimes_.erase(
            std::unique(monitoringTimes_.begin(), monitoringTimes_.end()),
            monitoringTimes_.end());
    }

    const std::vector<Time>&
    FdmDiscreteDoubleBarrierStepCondition::monitoringTimes() const {
        return monitoringTimes_;
    }

    void FdmDiscreteDoubleBarrierStepCondition::applyTo(Array& a, Time t) const {
        if (std::find(monitoringTimes_.begin(), monitoringTimes_.end(), t)
                == monitoringTimes_.end()) {
            return;
        }

        const ext::shared_ptr<FdmLinearOpLayout> layout = mesher_->layout();
        QL_REQUIRE(layout->size() == a.size(), "inconsistent array dimensions");

        const FdmLinearOpIterator endIter = layout->end();
        for (FdmLinearOpIterator iter = layout->begin(); iter != endIter; ++iter) {
            Real underlying = mesher_->location(iter, direction_);
            if (logCoordinates_) {
                underlying = std::exp(underlying);
            }

            if (underlying <= lowerBarrier_ || underlying >= upperBarrier_) {
                a[iter.index()] = rebate_;
            }
        }
    }
}
