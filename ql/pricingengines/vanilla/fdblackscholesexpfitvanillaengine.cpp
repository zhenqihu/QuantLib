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

/*! \file fdblackscholesexpfitvanillaengine.cpp
*/

#include <ql/exercise.hpp>
#include <ql/methods/finitedifferences/meshers/fdmblackscholesmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/solvers/fdmblackscholesexpfitsolver.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmstepconditioncomposite.hpp>
#include <ql/methods/finitedifferences/utilities/escroweddividendadjustment.hpp>
#include <ql/methods/finitedifferences/utilities/fdmescrowedloginnervaluecalculator.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>
#include <ql/methods/finitedifferences/utilities/fdmquantohelper.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesexpfitvanillaengine.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <cmath>
#include <utility>

namespace QuantLib {

    FdBlackScholesExpFitVanillaEngine::FdBlackScholesExpFitVanillaEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process,
        Size tGrid,
        Size xGrid,
        Size dampingSteps,
        const FdmSchemeDesc& schemeDesc,
        bool localVol,
        Real illegalLocalVolOverwrite,
        CashDividendModel cashDividendModel,
        EquityMesher equityMesher,
        Real xMinConstraint,
        Real xMaxConstraint)
    : process_(std::move(process)), tGrid_(tGrid), xGrid_(xGrid),
      dampingSteps_(dampingSteps), schemeDesc_(schemeDesc),
      localVol_(localVol),
      illegalLocalVolOverwrite_(illegalLocalVolOverwrite),
      quantoHelper_(ext::shared_ptr<FdmQuantoHelper>()),
      cashDividendModel_(cashDividendModel), equityMesher_(equityMesher),
      xMinConstraint_(xMinConstraint), xMaxConstraint_(xMaxConstraint) {
        registerWith(process_);
    }

    FdBlackScholesExpFitVanillaEngine::FdBlackScholesExpFitVanillaEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process,
        ext::shared_ptr<FdmQuantoHelper> quantoHelper,
        Size tGrid,
        Size xGrid,
        Size dampingSteps,
        const FdmSchemeDesc& schemeDesc,
        bool localVol,
        Real illegalLocalVolOverwrite,
        CashDividendModel cashDividendModel,
        EquityMesher equityMesher,
        Real xMinConstraint,
        Real xMaxConstraint)
    : process_(std::move(process)), tGrid_(tGrid), xGrid_(xGrid),
      dampingSteps_(dampingSteps), schemeDesc_(schemeDesc),
      localVol_(localVol),
      illegalLocalVolOverwrite_(illegalLocalVolOverwrite),
      quantoHelper_(std::move(quantoHelper)),
      cashDividendModel_(cashDividendModel), equityMesher_(equityMesher),
      xMinConstraint_(xMinConstraint), xMaxConstraint_(xMaxConstraint) {
        registerWith(process_);
        registerWith(quantoHelper_);
    }

    void FdBlackScholesExpFitVanillaEngine::calculate() const {
        const Date exerciseDate = arguments_.exercise->lastDate();
        const Time maturity = process_->time(exerciseDate);
        const Date settlementDate = process_->riskFreeRate()->referenceDate();

        Real spotAdjustment = 0.0;
        DividendSchedule dividendSchedule = DividendSchedule();

        ext::shared_ptr<EscrowedDividendAdjustment> escrowedDivAdj;

        switch (cashDividendModel_) {
          case Spot:
            dividendSchedule = arguments_.cashFlow;
            break;
          case Escrowed:
            if (arguments_.exercise->type() != Exercise::European) {
                for (const auto& cf : arguments_.cashFlow) {
                    dividendSchedule.push_back(
                        ext::make_shared<FixedDividend>(0.0, cf->date()));
                }
            }

            QL_REQUIRE(quantoHelper_ == nullptr,
                       "Escrowed dividend model is not supported for "
                       "Quanto-Options");

            escrowedDivAdj = ext::make_shared<EscrowedDividendAdjustment>(
                arguments_.cashFlow, process_->riskFreeRate(),
                process_->dividendYield(),
                [&](Date d) { return process_->time(d); }, maturity);

            spotAdjustment = escrowedDivAdj->dividendAdjustment(
                process_->time(settlementDate));

            QL_REQUIRE(process_->x0() + spotAdjustment > 0.0,
                       "spot minus dividends becomes negative");
            break;
          default:
            QL_FAIL("unknwon cash dividend model");
        }

        const ext::shared_ptr<StrikedTypePayoff> payoff =
            ext::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-striked payoff given");

        if (xMinConstraint_ != Null<Real>() && xMaxConstraint_ != Null<Real>()) {
            QL_REQUIRE(xMinConstraint_ < xMaxConstraint_,
                       "xMinConstraint must be smaller than xMaxConstraint");
        }

        if (equityMesher_ == Concentrating) {
            const Real logStrike = std::log(payoff->strike());
            QL_REQUIRE(xMinConstraint_ == Null<Real>()
                           || xMinConstraint_ <= logStrike,
                       "xMinConstraint must not exclude the log-strike "
                       "when using the concentrating equity mesher");
            QL_REQUIRE(xMaxConstraint_ == Null<Real>()
                           || xMaxConstraint_ >= logStrike,
                       "xMaxConstraint must not exclude the log-strike "
                       "when using the concentrating equity mesher");
        }

        const std::pair<Real, Real> cPoint =
            (equityMesher_ == Concentrating)
                ? std::make_pair(payoff->strike(), 0.1)
                : std::make_pair(Null<Real>(), Null<Real>());

        const ext::shared_ptr<Fdm1dMesher> equityMesher =
            ext::make_shared<FdmBlackScholesMesher>(
                xGrid_, process_, maturity, payoff->strike(),
                xMinConstraint_, xMaxConstraint_, 0.0001, 1.5, cPoint,
                dividendSchedule,
                quantoHelper_, spotAdjustment);

        const ext::shared_ptr<FdmMesher> mesher =
            ext::make_shared<FdmMesherComposite>(equityMesher);

        ext::shared_ptr<FdmInnerValueCalculator> calculator;
        switch (cashDividendModel_) {
          case Spot:
            calculator =
                ext::make_shared<FdmLogInnerValue>(payoff, mesher, 0);
            break;
          case Escrowed:
            calculator =
                ext::make_shared<FdmEscrowedLogInnerValueCalculator>(
                    escrowedDivAdj, payoff, mesher, 0);
            break;
          default:
            QL_FAIL("unknwon cash dividend model");
        }

        const ext::shared_ptr<FdmStepConditionComposite> conditions =
            FdmStepConditionComposite::vanillaComposite(
                dividendSchedule, arguments_.exercise, mesher, calculator,
                process_->riskFreeRate()->referenceDate(),
                process_->riskFreeRate()->dayCounter());

        const FdmBoundaryConditionSet boundaries;

        const FdmSolverDesc solverDesc = {mesher, boundaries, conditions,
                                          calculator, maturity, tGrid_,
                                          dampingSteps_};

        const ext::shared_ptr<FdmBlackScholesExpFitSolver> solver =
            ext::make_shared<FdmBlackScholesExpFitSolver>(
                Handle<GeneralizedBlackScholesProcess>(process_),
                payoff->strike(), solverDesc, schemeDesc_, localVol_,
                illegalLocalVolOverwrite_,
                Handle<FdmQuantoHelper>(quantoHelper_));

        const Real spot = process_->x0() + spotAdjustment;

        results_.value = solver->valueAt(spot);
        results_.delta = solver->deltaAt(spot);
        results_.gamma = solver->gammaAt(spot);
        results_.theta = solver->thetaAt(spot);
    }

    MakeFdBlackScholesExpFitVanillaEngine::
        MakeFdBlackScholesExpFitVanillaEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process)
    : process_(std::move(process)), tGrid_(100), xGrid_(100),
      dampingSteps_(0),
      schemeDesc_(
          ext::make_shared<FdmSchemeDesc>(FdmSchemeDesc::ImplicitEuler())),
      localVol_(false), illegalLocalVolOverwrite_(-Null<Real>()),
      quantoHelper_(ext::shared_ptr<FdmQuantoHelper>()),
      cashDividendModel_(FdBlackScholesExpFitVanillaEngine::Spot),
      equityMesher_(FdBlackScholesExpFitVanillaEngine::Uniform),
      xMinConstraint_(Null<Real>()), xMaxConstraint_(Null<Real>()) {}

    MakeFdBlackScholesExpFitVanillaEngine&
    MakeFdBlackScholesExpFitVanillaEngine::withQuantoHelper(
        const ext::shared_ptr<FdmQuantoHelper>& quantoHelper) {
        quantoHelper_ = quantoHelper;
        return *this;
    }

    MakeFdBlackScholesExpFitVanillaEngine&
    MakeFdBlackScholesExpFitVanillaEngine::withTGrid(Size tGrid) {
        tGrid_ = tGrid;
        return *this;
    }

    MakeFdBlackScholesExpFitVanillaEngine&
    MakeFdBlackScholesExpFitVanillaEngine::withXGrid(Size xGrid) {
        xGrid_ = xGrid;
        return *this;
    }

    MakeFdBlackScholesExpFitVanillaEngine&
    MakeFdBlackScholesExpFitVanillaEngine::withDampingSteps(
        Size dampingSteps) {
        dampingSteps_ = dampingSteps;
        return *this;
    }

    MakeFdBlackScholesExpFitVanillaEngine&
    MakeFdBlackScholesExpFitVanillaEngine::withFdmSchemeDesc(
        const FdmSchemeDesc& schemeDesc) {
        schemeDesc_ = ext::make_shared<FdmSchemeDesc>(schemeDesc);
        return *this;
    }

    MakeFdBlackScholesExpFitVanillaEngine&
    MakeFdBlackScholesExpFitVanillaEngine::withLocalVol(bool localVol) {
        localVol_ = localVol;
        return *this;
    }

    MakeFdBlackScholesExpFitVanillaEngine&
    MakeFdBlackScholesExpFitVanillaEngine::withIllegalLocalVolOverwrite(
        Real illegalLocalVolOverwrite) {
        illegalLocalVolOverwrite_ = illegalLocalVolOverwrite;
        return *this;
    }

    MakeFdBlackScholesExpFitVanillaEngine&
    MakeFdBlackScholesExpFitVanillaEngine::withCashDividendModel(
        FdBlackScholesExpFitVanillaEngine::CashDividendModel
            cashDividendModel) {
        cashDividendModel_ = cashDividendModel;
        return *this;
    }

    MakeFdBlackScholesExpFitVanillaEngine&
    MakeFdBlackScholesExpFitVanillaEngine::withEquityMesher(
        FdBlackScholesExpFitVanillaEngine::EquityMesher equityMesher) {
        equityMesher_ = equityMesher;
        return *this;
    }

    MakeFdBlackScholesExpFitVanillaEngine&
    MakeFdBlackScholesExpFitVanillaEngine::withXMinConstraint(
        Real xMinConstraint) {
        xMinConstraint_ = xMinConstraint;
        return *this;
    }

    MakeFdBlackScholesExpFitVanillaEngine&
    MakeFdBlackScholesExpFitVanillaEngine::withXMaxConstraint(
        Real xMaxConstraint) {
        xMaxConstraint_ = xMaxConstraint;
        return *this;
    }

    MakeFdBlackScholesExpFitVanillaEngine::operator
    ext::shared_ptr<PricingEngine>() const {
        return ext::make_shared<FdBlackScholesExpFitVanillaEngine>(
            process_, quantoHelper_, tGrid_, xGrid_, dampingSteps_,
            *schemeDesc_, localVol_, illegalLocalVolOverwrite_,
            cashDividendModel_, equityMesher_,
            xMinConstraint_, xMaxConstraint_);
    }
}
