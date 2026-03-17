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

/*! \file fdblackscholescnvariantvanillaengine.hpp
    \brief Finite-differences Black-Scholes vanilla option engine with
           Crank-Nicolson variant solver
*/

#ifndef quantlib_fd_black_scholes_cn_variant_vanilla_engine_hpp
#define quantlib_fd_black_scholes_cn_variant_vanilla_engine_hpp

#include <ql/instruments/dividendvanillaoption.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>
#include <ql/pricingengine.hpp>

namespace QuantLib {

    class FdmQuantoHelper;
    class GeneralizedBlackScholesProcess;

    class FdBlackScholesCnVariantVanillaEngine
        : public DividendVanillaOption::engine {
      public:
        enum CashDividendModel { Spot, Escrowed };
        enum EquityMesher { Concentrating, Uniform };

        explicit FdBlackScholesCnVariantVanillaEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess>,
            Size tGrid = 100,
            Size xGrid = 100,
            Size dampingSteps = 0,
            const FdmSchemeDesc& schemeDesc =
                FdmSchemeDesc::CrankNicolson(),
            bool localVol = false,
            Real illegalLocalVolOverwrite = -Null<Real>(),
            CashDividendModel cashDividendModel = Spot,
            EquityMesher equityMesher = Uniform,
            Real xMinConstraint = Null<Real>(),
            Real xMaxConstraint = Null<Real>());

        FdBlackScholesCnVariantVanillaEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess>,
            ext::shared_ptr<FdmQuantoHelper> quantoHelper,
            Size tGrid = 100,
            Size xGrid = 100,
            Size dampingSteps = 0,
            const FdmSchemeDesc& schemeDesc =
                FdmSchemeDesc::CrankNicolson(),
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

    class MakeFdBlackScholesCnVariantVanillaEngine {
      public:
        explicit MakeFdBlackScholesCnVariantVanillaEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process);

        MakeFdBlackScholesCnVariantVanillaEngine& withQuantoHelper(
            const ext::shared_ptr<FdmQuantoHelper>& quantoHelper);

        MakeFdBlackScholesCnVariantVanillaEngine& withTGrid(Size tGrid);
        MakeFdBlackScholesCnVariantVanillaEngine& withXGrid(Size xGrid);
        MakeFdBlackScholesCnVariantVanillaEngine& withDampingSteps(
            Size dampingSteps);

        MakeFdBlackScholesCnVariantVanillaEngine& withFdmSchemeDesc(
            const FdmSchemeDesc& schemeDesc);

        MakeFdBlackScholesCnVariantVanillaEngine& withLocalVol(bool localVol);
        MakeFdBlackScholesCnVariantVanillaEngine&
        withIllegalLocalVolOverwrite(Real illegalLocalVolOverwrite);

        MakeFdBlackScholesCnVariantVanillaEngine& withCashDividendModel(
            FdBlackScholesCnVariantVanillaEngine::CashDividendModel
                cashDividendModel);

        MakeFdBlackScholesCnVariantVanillaEngine& withEquityMesher(
            FdBlackScholesCnVariantVanillaEngine::EquityMesher equityMesher);

        MakeFdBlackScholesCnVariantVanillaEngine& withXMinConstraint(
            Real xMinConstraint);
        MakeFdBlackScholesCnVariantVanillaEngine& withXMaxConstraint(
            Real xMaxConstraint);

        operator ext::shared_ptr<PricingEngine>() const;

      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        Size tGrid_, xGrid_, dampingSteps_;
        ext::shared_ptr<FdmSchemeDesc> schemeDesc_;
        bool localVol_;
        Real illegalLocalVolOverwrite_;
        ext::shared_ptr<FdmQuantoHelper> quantoHelper_;
        FdBlackScholesCnVariantVanillaEngine::CashDividendModel
            cashDividendModel_;
        FdBlackScholesCnVariantVanillaEngine::EquityMesher equityMesher_;
        Real xMinConstraint_, xMaxConstraint_;
    };
}

#endif
