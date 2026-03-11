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

/*! \file fdblackscholesuniformvanillaengine.hpp
    \brief Finite-differences Black-Scholes vanilla option engine with
           configurable equity mesher
*/

#ifndef quantlib_fd_black_scholes_uniform_vanilla_engine_hpp
#define quantlib_fd_black_scholes_uniform_vanilla_engine_hpp

#include <ql/instruments/dividendvanillaoption.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>
#include <ql/pricingengine.hpp>

namespace QuantLib {

    class FdmQuantoHelper;
    class GeneralizedBlackScholesProcess;

    class FdBlackScholesUniformVanillaEngine
        : public DividendVanillaOption::engine {
      public:
        enum CashDividendModel { Spot, Escrowed };

        explicit FdBlackScholesUniformVanillaEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess>,
            Size tGrid = 100,
            Size xGrid = 100,
            Size dampingSteps = 0,
            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Douglas(),
            bool localVol = false,
            Real illegalLocalVolOverwrite = -Null<Real>(),
            CashDividendModel cashDividendModel = Spot,
            Real xMinConstraint = Null<Real>(),
            Real xMaxConstraint = Null<Real>());

        FdBlackScholesUniformVanillaEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess>,
            ext::shared_ptr<FdmQuantoHelper> quantoHelper,
            Size tGrid = 100,
            Size xGrid = 100,
            Size dampingSteps = 0,
            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Douglas(),
            bool localVol = false,
            Real illegalLocalVolOverwrite = -Null<Real>(),
            CashDividendModel cashDividendModel = Spot,
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
        const Real xMinConstraint_, xMaxConstraint_;
    };

    class MakeFdBlackScholesUniformVanillaEngine {
      public:
        explicit MakeFdBlackScholesUniformVanillaEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process);

        MakeFdBlackScholesUniformVanillaEngine& withQuantoHelper(
            const ext::shared_ptr<FdmQuantoHelper>& quantoHelper);

        MakeFdBlackScholesUniformVanillaEngine& withTGrid(Size tGrid);
        MakeFdBlackScholesUniformVanillaEngine& withXGrid(Size xGrid);
        MakeFdBlackScholesUniformVanillaEngine& withDampingSteps(
            Size dampingSteps);

        MakeFdBlackScholesUniformVanillaEngine& withFdmSchemeDesc(
            const FdmSchemeDesc& schemeDesc);

        MakeFdBlackScholesUniformVanillaEngine& withLocalVol(bool localVol);
        MakeFdBlackScholesUniformVanillaEngine&
        withIllegalLocalVolOverwrite(Real illegalLocalVolOverwrite);

        MakeFdBlackScholesUniformVanillaEngine& withCashDividendModel(
            FdBlackScholesUniformVanillaEngine::CashDividendModel
                cashDividendModel);

        MakeFdBlackScholesUniformVanillaEngine& withXMinConstraint(
            Real xMinConstraint);
        MakeFdBlackScholesUniformVanillaEngine& withXMaxConstraint(
            Real xMaxConstraint);

        operator ext::shared_ptr<PricingEngine>() const;

      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        Size tGrid_, xGrid_, dampingSteps_;
        ext::shared_ptr<FdmSchemeDesc> schemeDesc_;
        bool localVol_;
        Real illegalLocalVolOverwrite_;
        ext::shared_ptr<FdmQuantoHelper> quantoHelper_;
        FdBlackScholesUniformVanillaEngine::CashDividendModel
            cashDividendModel_;
        Real xMinConstraint_, xMaxConstraint_;
    };
}

#endif
