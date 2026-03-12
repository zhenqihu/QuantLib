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

/*! \file fdblackscholesdiscretedoublebarrierengine.hpp
    \brief Finite-differences Black-Scholes discrete double barrier option engine
*/

#ifndef quantlib_fd_black_scholes_discrete_double_barrier_engine_hpp
#define quantlib_fd_black_scholes_discrete_double_barrier_engine_hpp

#include <ql/experimental/barrieroption/doublebarrieroption.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>

namespace QuantLib {

    class FdmQuantoHelper;
    class GeneralizedBlackScholesProcess;

    //! Finite-differences Black-Scholes discrete double barrier option engine
    /*! Bermudan exercise dates are interpreted as monitoring dates.
        No early-exercise feature is modeled by this engine.
    */
    class FdBlackScholesDiscreteDoubleBarrierEngine
        : public DoubleBarrierOption::engine {
      public:
        enum EquityMesher { Concentrating, Uniform };

        explicit FdBlackScholesDiscreteDoubleBarrierEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess>,
            Size tGrid = 100,
            Size xGrid = 100,
            Size dampingSteps = 0,
            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Douglas(),
            bool localVol = false,
            Real illegalLocalVolOverwrite = -Null<Real>(),
            EquityMesher equityMesher = Uniform,
            Real xMinConstraint = Null<Real>(),
            Real xMaxConstraint = Null<Real>());

        FdBlackScholesDiscreteDoubleBarrierEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess>,
            ext::shared_ptr<FdmQuantoHelper> quantoHelper,
            Size tGrid = 100,
            Size xGrid = 100,
            Size dampingSteps = 0,
            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Douglas(),
            bool localVol = false,
            Real illegalLocalVolOverwrite = -Null<Real>(),
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
        const EquityMesher equityMesher_;
        const Real xMinConstraint_, xMaxConstraint_;
    };
}

#endif
