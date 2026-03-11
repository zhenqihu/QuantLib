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

/*! \file fdblackscholesexpfitvanillaengine.hpp
    \brief Finite-differences Black-Scholes vanilla option engine with
           exponential fitting
*/

#ifndef quantlib_fd_black_scholes_exp_fit_vanilla_engine_hpp
#define quantlib_fd_black_scholes_exp_fit_vanilla_engine_hpp

#include <ql/instruments/dividendvanillaoption.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>
#include <ql/pricingengine.hpp>

namespace QuantLib {

    class FdmQuantoHelper;
    class GeneralizedBlackScholesProcess;

    class FdBlackScholesExpFitVanillaEngine
        : public DividendVanillaOption::engine {
      public:
        enum CashDividendModel { Spot, Escrowed };
        enum EquityMesher { Concentrating, Uniform };

        explicit FdBlackScholesExpFitVanillaEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess>,
            Size tGrid = 100,
            Size xGrid = 100,
            Size dampingSteps = 0,
            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::ImplicitEuler(),
            bool localVol = false,
            Real illegalLocalVolOverwrite = -Null<Real>(),
            CashDividendModel cashDividendModel = Spot,
            EquityMesher equityMesher = Uniform,
            Real xMinConstraint = Null<Real>(),
            Real xMaxConstraint = Null<Real>());

        FdBlackScholesExpFitVanillaEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess>,
            ext::shared_ptr<FdmQuantoHelper> quantoHelper,
            Size tGrid = 100,
            Size xGrid = 100,
            Size dampingSteps = 0,
            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::ImplicitEuler(),
            bool localVol = false,
            Real illegalLocalVolOverwrite = -Null<Real>(),
            CashDividendModel cashDividendModel = Spot,
            EquityMesher equityMesher = Uniform,
            Real xMinConstraint = Null<Real>(),
            Real xMaxConstraint = Null<Real>());

        void calculate() const override;

      private:
        const ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        const Size tGrid_, xGrid_, dampingSteps_;
        const FdmSchemeDesc schemeDesc_;
        const bool localVol_;
        const Real illegalLocalVolOverwrite_;
        const ext::shared_ptr<FdmQuantoHelper> quantoHelper_;
        const CashDividendModel cashDividendModel_;
        const EquityMesher equityMesher_;
        const Real xMinConstraint_, xMaxConstraint_;
    };

    class MakeFdBlackScholesExpFitVanillaEngine {
      public:
        explicit MakeFdBlackScholesExpFitVanillaEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process);

        MakeFdBlackScholesExpFitVanillaEngine& withQuantoHelper(
            const ext::shared_ptr<FdmQuantoHelper>& quantoHelper);

        MakeFdBlackScholesExpFitVanillaEngine& withTGrid(Size tGrid);
        MakeFdBlackScholesExpFitVanillaEngine& withXGrid(Size xGrid);
        MakeFdBlackScholesExpFitVanillaEngine& withDampingSteps(
            Size dampingSteps);

        MakeFdBlackScholesExpFitVanillaEngine& withFdmSchemeDesc(
            const FdmSchemeDesc& schemeDesc);

        MakeFdBlackScholesExpFitVanillaEngine& withLocalVol(bool localVol);
        MakeFdBlackScholesExpFitVanillaEngine& withIllegalLocalVolOverwrite(
            Real illegalLocalVolOverwrite);

        MakeFdBlackScholesExpFitVanillaEngine& withCashDividendModel(
            FdBlackScholesExpFitVanillaEngine::CashDividendModel
                cashDividendModel);

        MakeFdBlackScholesExpFitVanillaEngine& withEquityMesher(
            FdBlackScholesExpFitVanillaEngine::EquityMesher equityMesher);

        MakeFdBlackScholesExpFitVanillaEngine& withXMinConstraint(
            Real xMinConstraint);
        MakeFdBlackScholesExpFitVanillaEngine& withXMaxConstraint(
            Real xMaxConstraint);

        operator ext::shared_ptr<PricingEngine>() const;

      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        Size tGrid_, xGrid_, dampingSteps_;
        ext::shared_ptr<FdmSchemeDesc> schemeDesc_;
        bool localVol_;
        Real illegalLocalVolOverwrite_;
        ext::shared_ptr<FdmQuantoHelper> quantoHelper_;
        FdBlackScholesExpFitVanillaEngine::CashDividendModel
            cashDividendModel_;
        FdBlackScholesExpFitVanillaEngine::EquityMesher equityMesher_;
        Real xMinConstraint_, xMaxConstraint_;
    };
}

#endif
