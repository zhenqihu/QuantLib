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

#include <ql/experimental/barrieroption/fdblackscholescnvariantdoublebarrierengine.hpp>
#include <ql/methods/finitedifferences/solvers/fdmblackscholescnvariantsolver.hpp>

#include "fdblackscholesdoublebarrierenginehelper.hpp"

namespace QuantLib {

    FdBlackScholesCnVariantDoubleBarrierEngine::
    FdBlackScholesCnVariantDoubleBarrierEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process,
        Size tGrid,
        Size xGrid,
        Size dampingSteps,
        const FdmSchemeDesc& schemeDesc,
        bool localVol,
        Real illegalLocalVolOverwrite,
        EquityMesher equityMesher)
    : process_(std::move(process)), tGrid_(tGrid), xGrid_(xGrid),
      dampingSteps_(dampingSteps), schemeDesc_(schemeDesc),
      localVol_(localVol),
      illegalLocalVolOverwrite_(illegalLocalVolOverwrite),
      quantoHelper_(ext::shared_ptr<FdmQuantoHelper>()),
      equityMesher_(equityMesher) {
        registerWith(process_);
    }

    FdBlackScholesCnVariantDoubleBarrierEngine::
    FdBlackScholesCnVariantDoubleBarrierEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process,
        ext::shared_ptr<FdmQuantoHelper> quantoHelper,
        Size tGrid,
        Size xGrid,
        Size dampingSteps,
        const FdmSchemeDesc& schemeDesc,
        bool localVol,
        Real illegalLocalVolOverwrite,
        EquityMesher equityMesher)
    : process_(std::move(process)), tGrid_(tGrid), xGrid_(xGrid),
      dampingSteps_(dampingSteps), schemeDesc_(schemeDesc),
      localVol_(localVol),
      illegalLocalVolOverwrite_(illegalLocalVolOverwrite),
      quantoHelper_(std::move(quantoHelper)),
      equityMesher_(equityMesher) {
        registerWith(process_);
        registerWith(quantoHelper_);
    }

    void FdBlackScholesCnVariantDoubleBarrierEngine::calculate() const {
        const FdBlackScholesDoubleBarrierSolverSetup setup =
            makeFdBlackScholesDoubleBarrierSolverSetup(
                arguments_, process_, quantoHelper_, tGrid_, xGrid_,
                dampingSteps_, equityMesher_ == Concentrating);

        const ext::shared_ptr<FdmBlackScholesCnVariantSolver> solver =
            ext::make_shared<FdmBlackScholesCnVariantSolver>(
                Handle<GeneralizedBlackScholesProcess>(process_),
                setup.strike, setup.solverDesc, schemeDesc_, localVol_,
                illegalLocalVolOverwrite_,
                Handle<FdmQuantoHelper>(quantoHelper_));

        results_.value = solver->valueAt(setup.spot);
        results_.delta = solver->deltaAt(setup.spot);
        results_.gamma = solver->gammaAt(setup.spot);
        results_.theta = solver->thetaAt(setup.spot);
    }
}
