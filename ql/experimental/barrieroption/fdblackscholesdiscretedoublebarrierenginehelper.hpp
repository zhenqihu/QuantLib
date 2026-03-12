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

#ifndef quantlib_fd_black_scholes_discrete_double_barrier_engine_helper_hpp
#define quantlib_fd_black_scholes_discrete_double_barrier_engine_helper_hpp

#include <ql/errors.hpp>
#include <ql/exercise.hpp>
#include <ql/experimental/barrieroption/doublebarrieroption.hpp>
#include <ql/methods/finitedifferences/meshers/fdmblackscholesmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/solvers/fdmsolverdesc.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmdiscretedoublebarrierstepcondition.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmstepconditioncomposite.hpp>
#include <ql/methods/finitedifferences/utilities/fdmdirichletboundary.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>
#include <ql/methods/finitedifferences/utilities/fdmquantohelper.hpp>
#include <ql/processes/blackscholesprocess.hpp>

#include <cmath>
#include <list>
#include <utility>

namespace QuantLib {

    struct FdBlackScholesDiscreteDoubleBarrierSolverSetup {
        FdmSolverDesc solverDesc;
        Real strike;
        Real spot;
    };

    inline std::pair<Real, Real> defaultDiscreteDoubleBarrierDomain(
        Real lowerBarrier,
        Real upperBarrier) {
        const Real logWidth = std::log(upperBarrier) - std::log(lowerBarrier);
        return std::make_pair(std::log(lowerBarrier) - logWidth,
                              std::log(upperBarrier) + logWidth);
    }

    inline FdBlackScholesDiscreteDoubleBarrierSolverSetup
    makeFdBlackScholesDiscreteDoubleBarrierSolverSetup(
        const DoubleBarrierOption::arguments& arguments,
        const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
        const ext::shared_ptr<FdmQuantoHelper>& quantoHelper,
        Size tGrid,
        Size xGrid,
        Size dampingSteps,
        bool concentratingEquityMesher,
        Real xMinConstraint,
        Real xMaxConstraint) {

        QL_REQUIRE(arguments.barrierType == DoubleBarrier::KnockOut,
                   "only Knock-Out double barrier options are supported");
        QL_REQUIRE(arguments.rebate == 0.0,
                   "only zero-rebate discrete double barrier options are "
                   "supported");
        QL_REQUIRE(arguments.exercise->type() == Exercise::European
                       || arguments.exercise->type() == Exercise::Bermudan,
                   "only European or Bermudan exercise are supported");

        const Date maturityDate = arguments.exercise->lastDate();
        const Time maturity = process->time(maturityDate);

        const ext::shared_ptr<StrikedTypePayoff> payoff =
            ext::dynamic_pointer_cast<StrikedTypePayoff>(arguments.payoff);
        QL_REQUIRE(payoff, "non-striked payoff given");

        if (xMinConstraint == Null<Real>() || xMaxConstraint == Null<Real>()) {
            const std::pair<Real, Real> defaultDomain =
                defaultDiscreteDoubleBarrierDomain(arguments.barrier_lo,
                                                   arguments.barrier_hi);

            if (xMinConstraint == Null<Real>()) {
                xMinConstraint = defaultDomain.first;
            }
            if (xMaxConstraint == Null<Real>()) {
                xMaxConstraint = defaultDomain.second;
            }
        }

        QL_REQUIRE(xMinConstraint < xMaxConstraint,
                   "xMinConstraint must be smaller than xMaxConstraint");

        if (concentratingEquityMesher) {
            const Real logStrike = std::log(payoff->strike());
            QL_REQUIRE(xMinConstraint <= logStrike,
                       "xMinConstraint must not exclude the log-strike "
                       "when using the concentrating equity mesher");
            QL_REQUIRE(xMaxConstraint >= logStrike,
                       "xMaxConstraint must not exclude the log-strike "
                       "when using the concentrating equity mesher");
        }

        const std::pair<Real, Real> cPoint =
            concentratingEquityMesher
                ? std::make_pair(payoff->strike(), 0.1)
                : std::make_pair(Null<Real>(), Null<Real>());

        const ext::shared_ptr<Fdm1dMesher> equityMesher =
            ext::make_shared<FdmBlackScholesMesher>(
                xGrid, process, maturity, payoff->strike(),
                xMinConstraint, xMaxConstraint, 0.0001, 1.5, cPoint,
                DividendSchedule(), quantoHelper);

        const ext::shared_ptr<FdmMesher> mesher =
            ext::make_shared<FdmMesherComposite>(equityMesher);

        const ext::shared_ptr<FdmInnerValueCalculator> calculator =
            ext::make_shared<FdmLogInnerValue>(payoff, mesher, 0);

        std::list<ext::shared_ptr<StepCondition<Array> > > stepConditions;
        std::list<std::vector<Time> > stoppingTimes;

        const ext::shared_ptr<FdmDiscreteDoubleBarrierStepCondition>
            barrierCondition =
                ext::make_shared<FdmDiscreteDoubleBarrierStepCondition>(
                    arguments.exercise->dates(),
                    process->riskFreeRate()->referenceDate(),
                    process->riskFreeRate()->dayCounter(),
                    mesher,
                    arguments.barrier_lo,
                    arguments.barrier_hi,
                    arguments.rebate);

        stepConditions.push_back(barrierCondition);
        stoppingTimes.push_back(barrierCondition->monitoringTimes());

        const ext::shared_ptr<FdmStepConditionComposite> conditions =
            ext::make_shared<FdmStepConditionComposite>(stoppingTimes,
                                                        stepConditions);

        FdmBoundaryConditionSet boundaries;
        boundaries.push_back(ext::make_shared<FdmDirichletBoundary>(
            mesher, 0.0, 0, FdmDirichletBoundary::Lower));
        boundaries.push_back(ext::make_shared<FdmDirichletBoundary>(
            mesher, 0.0, 0, FdmDirichletBoundary::Upper));

        return {{mesher, boundaries, conditions, calculator, maturity, tGrid,
                 dampingSteps},
                payoff->strike(),
                process->x0()};
    }
}

#endif
