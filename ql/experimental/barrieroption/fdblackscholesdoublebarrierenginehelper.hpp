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

#ifndef quantlib_fd_black_scholes_double_barrier_engine_helper_hpp
#define quantlib_fd_black_scholes_double_barrier_engine_helper_hpp

#include <ql/errors.hpp>
#include <ql/exercise.hpp>
#include <ql/experimental/barrieroption/doublebarrieroption.hpp>
#include <ql/methods/finitedifferences/meshers/fdmblackscholesmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/solvers/fdmsolverdesc.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmstepconditioncomposite.hpp>
#include <ql/methods/finitedifferences/utilities/fdmdirichletboundary.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>
#include <ql/methods/finitedifferences/utilities/fdmquantohelper.hpp>
#include <ql/processes/blackscholesprocess.hpp>

#include <cmath>
#include <list>
#include <utility>

namespace QuantLib {

    struct FdBlackScholesDoubleBarrierSolverSetup {
        FdmSolverDesc solverDesc;
        Real strike;
        Real spot;
    };

    inline FdBlackScholesDoubleBarrierSolverSetup
    makeFdBlackScholesDoubleBarrierSolverSetup(
        const DoubleBarrierOption::arguments& arguments,
        const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
        const ext::shared_ptr<FdmQuantoHelper>& quantoHelper,
        Size tGrid,
        Size xGrid,
        Size dampingSteps,
        bool concentratingEquityMesher) {

        QL_REQUIRE(arguments.barrierType == DoubleBarrier::KnockOut,
                   "only Knock-Out double barrier options are supported");
        QL_REQUIRE(arguments.exercise->type() == Exercise::European,
                   "only European exercise is supported");

        const Date maturityDate = arguments.exercise->lastDate();
        const Time maturity = process->time(maturityDate);

        const ext::shared_ptr<StrikedTypePayoff> payoff =
            ext::dynamic_pointer_cast<StrikedTypePayoff>(arguments.payoff);
        QL_REQUIRE(payoff, "non-striked payoff given");

        const Real lowerBarrier = arguments.barrier_lo;
        const Real upperBarrier = arguments.barrier_hi;
        const Real spot = process->x0();

        QL_REQUIRE(lowerBarrier > 0.0, "low barrier must be positive");
        QL_REQUIRE(upperBarrier > 0.0, "high barrier must be positive");
        QL_REQUIRE(lowerBarrier < upperBarrier,
                   "low barrier must be below high barrier");
        QL_REQUIRE(spot > lowerBarrier && spot < upperBarrier,
                   "barrier touched");

        if (concentratingEquityMesher) {
            QL_REQUIRE(payoff->strike() > lowerBarrier
                           && payoff->strike() < upperBarrier,
                       "concentrating equity mesher requires the strike to "
                       "lie strictly inside the barrier corridor");
        }

        const std::pair<Real, Real> cPoint =
            concentratingEquityMesher
                ? std::make_pair(payoff->strike(), 0.1)
                : std::make_pair(Null<Real>(), Null<Real>());

        const ext::shared_ptr<Fdm1dMesher> equityMesher =
            ext::make_shared<FdmBlackScholesMesher>(
                xGrid, process, maturity, payoff->strike(),
                std::log(lowerBarrier), std::log(upperBarrier), 0.0001, 1.5,
                cPoint, DividendSchedule(), quantoHelper);

        const ext::shared_ptr<FdmMesher> mesher =
            ext::make_shared<FdmMesherComposite>(equityMesher);

        const ext::shared_ptr<FdmInnerValueCalculator> calculator =
            ext::make_shared<FdmLogInnerValue>(payoff, mesher, 0);

        std::list<std::vector<Time> > stoppingTimes;
        std::list<ext::shared_ptr<StepCondition<Array> > > stepConditions;
        const ext::shared_ptr<FdmStepConditionComposite> conditions =
            ext::make_shared<FdmStepConditionComposite>(stoppingTimes,
                                                        stepConditions);

        FdmBoundaryConditionSet boundaries;
        boundaries.push_back(ext::make_shared<FdmDirichletBoundary>(
            mesher, arguments.rebate, 0, FdmDirichletBoundary::Lower));
        boundaries.push_back(ext::make_shared<FdmDirichletBoundary>(
            mesher, arguments.rebate, 0, FdmDirichletBoundary::Upper));

        return {{mesher, boundaries, conditions, calculator, maturity, tGrid,
                 dampingSteps},
                payoff->strike(),
                spot};
    }
}

#endif
