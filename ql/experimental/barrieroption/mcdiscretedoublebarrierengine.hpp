/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 OpenAI

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

/*! \file mcdiscretedoublebarrierengine.hpp
    \brief Monte Carlo engine for discretely monitored double barrier options
*/

#ifndef quantlib_mc_discrete_double_barrier_engine_hpp
#define quantlib_mc_discrete_double_barrier_engine_hpp

#include <ql/experimental/barrieroption/mcdoublebarrierengine.hpp>
#include <ql/settings.hpp>
#include <algorithm>
#include <vector>

namespace QuantLib {

    template <class RNG = PseudoRandom, class S = Statistics>
    class MCDiscreteDoubleBarrierEngine
        : public DoubleBarrierOption::engine,
          public McSimulation<SingleVariate, RNG, S> {
      public:
        typedef typename McSimulation<SingleVariate, RNG, S>::path_generator_type
            path_generator_type;
        typedef typename McSimulation<SingleVariate, RNG, S>::path_pricer_type
            path_pricer_type;

        MCDiscreteDoubleBarrierEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process,
            bool brownianBridge,
            bool antithetic,
            Size requiredSamples,
            Real requiredTolerance,
            Size maxSamples,
            BigNatural seed);

        void calculate() const override;

      protected:
        TimeGrid timeGrid() const override;
        ext::shared_ptr<path_generator_type> pathGenerator() const override {
            const TimeGrid grid = timeGrid();
            typename RNG::rsg_type gen =
                RNG::make_sequence_generator(grid.size() - 1, seed_);
            return ext::shared_ptr<path_generator_type>(
                new path_generator_type(process_, grid, gen, brownianBridge_));
        }
        ext::shared_ptr<path_pricer_type> pathPricer() const override;

      private:
        std::vector<Date> monitoringDates() const;
        std::vector<Time> monitoringTimes() const;

        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        Size requiredSamples_, maxSamples_;
        Real requiredTolerance_;
        bool brownianBridge_;
        BigNatural seed_;
    };

    template <class RNG = PseudoRandom, class S = Statistics>
    class MakeMCDiscreteDoubleBarrierEngine {
      public:
        explicit MakeMCDiscreteDoubleBarrierEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process);

        MakeMCDiscreteDoubleBarrierEngine& withBrownianBridge(bool b = true);
        MakeMCDiscreteDoubleBarrierEngine& withAntitheticVariate(bool b = true);
        MakeMCDiscreteDoubleBarrierEngine& withSamples(Size samples);
        MakeMCDiscreteDoubleBarrierEngine& withAbsoluteTolerance(Real tolerance);
        MakeMCDiscreteDoubleBarrierEngine& withMaxSamples(Size samples);
        MakeMCDiscreteDoubleBarrierEngine& withSeed(BigNatural seed);

        operator ext::shared_ptr<PricingEngine>() const;

      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        bool brownianBridge_, antithetic_;
        Size samples_, maxSamples_;
        Real tolerance_;
        BigNatural seed_;
    };

    template <class RNG, class S>
    inline MCDiscreteDoubleBarrierEngine<RNG, S>::
        MCDiscreteDoubleBarrierEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process,
            bool brownianBridge,
            bool antitheticVariate,
            Size requiredSamples,
            Real requiredTolerance,
            Size maxSamples,
            BigNatural seed)
    : McSimulation<SingleVariate, RNG, S>(antitheticVariate, false),
      process_(std::move(process)), requiredSamples_(requiredSamples),
      maxSamples_(maxSamples), requiredTolerance_(requiredTolerance),
      brownianBridge_(brownianBridge), seed_(seed) {
        registerWith(process_);
    }

    template <class RNG, class S>
    inline void MCDiscreteDoubleBarrierEngine<RNG, S>::calculate() const {
        const Real spot = process_->x0();
        QL_REQUIRE(spot >= 0.0, "negative or null underlying given");
        QL_REQUIRE(arguments_.barrierType == DoubleBarrier::KnockOut,
                   "only knock-out discrete double barrier MC is supported");

        const std::vector<Date> dates = monitoringDates();
        const Date today = Settings::instance().evaluationDate();
        const bool includesToday = !dates.empty() && dates.front() == today;

        if (includesToday && triggered(spot)) {
            results_.value = arguments_.rebate;
            if (RNG::allowsErrorEstimate)
                results_.errorEstimate = 0.0;
            return;
        }

        McSimulation<SingleVariate, RNG, S>::calculate(requiredTolerance_,
                                                       requiredSamples_,
                                                       maxSamples_);
        results_.value = this->mcModel_->sampleAccumulator().mean();
        if (RNG::allowsErrorEstimate)
            results_.errorEstimate =
                this->mcModel_->sampleAccumulator().errorEstimate();
    }

    template <class RNG, class S>
    inline TimeGrid MCDiscreteDoubleBarrierEngine<RNG, S>::timeGrid() const {
        const std::vector<Time> times = monitoringTimes();
        return TimeGrid(times.begin(), times.end());
    }

    template <class RNG, class S>
    inline
    ext::shared_ptr<typename MCDiscreteDoubleBarrierEngine<RNG, S>::path_pricer_type>
    MCDiscreteDoubleBarrierEngine<RNG, S>::pathPricer() const {
        ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        const TimeGrid grid = timeGrid();
        std::vector<DiscountFactor> discounts(grid.size());
        for (Size i = 0; i < grid.size(); ++i)
            discounts[i] = process_->riskFreeRate()->discount(grid[i]);

        return ext::shared_ptr<path_pricer_type>(
            new DoubleBarrierPathPricer(arguments_.barrierType,
                                        arguments_.barrier_lo,
                                        arguments_.barrier_hi,
                                        arguments_.rebate,
                                        payoff->optionType(),
                                        payoff->strike(),
                                        discounts));
    }

    template <class RNG, class S>
    inline std::vector<Date>
    MCDiscreteDoubleBarrierEngine<RNG, S>::monitoringDates() const {
        std::vector<Date> dates = arguments_.exercise->dates();
        QL_REQUIRE(!dates.empty(),
                   "discrete double barrier MC requires at least one "
                   "monitoring date");

        std::sort(dates.begin(), dates.end());
        dates.erase(std::unique(dates.begin(), dates.end()), dates.end());

        const Date today = Settings::instance().evaluationDate();
        QL_REQUIRE(dates.front() >= today,
                   "monitoring dates before evaluation date are not allowed");
        QL_REQUIRE(dates.back() == arguments_.exercise->lastDate(),
                   "last monitoring date must equal exercise last date");

        return dates;
    }

    template <class RNG, class S>
    inline std::vector<Time>
    MCDiscreteDoubleBarrierEngine<RNG, S>::monitoringTimes() const {
        const std::vector<Date> dates = monitoringDates();

        std::vector<Time> times;
        times.reserve(dates.size());
        for (std::size_t i = 0; i < dates.size(); ++i) {
            const Time t = process_->time(dates[i]);
            QL_REQUIRE(t >= 0.0,
                       "monitoring time before evaluation date is not allowed");
            times.push_back(t);
        }

        return times;
    }

    template <class RNG, class S>
    inline MakeMCDiscreteDoubleBarrierEngine<RNG, S>::
        MakeMCDiscreteDoubleBarrierEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process)
    : process_(std::move(process)), brownianBridge_(false),
      antithetic_(false), samples_(Null<Size>()), maxSamples_(Null<Size>()),
      tolerance_(Null<Real>()), seed_(0) {}

    template <class RNG, class S>
    inline MakeMCDiscreteDoubleBarrierEngine<RNG, S>&
    MakeMCDiscreteDoubleBarrierEngine<RNG, S>::withBrownianBridge(
        bool brownianBridge) {
        brownianBridge_ = brownianBridge;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCDiscreteDoubleBarrierEngine<RNG, S>&
    MakeMCDiscreteDoubleBarrierEngine<RNG, S>::withAntitheticVariate(bool b) {
        antithetic_ = b;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCDiscreteDoubleBarrierEngine<RNG, S>&
    MakeMCDiscreteDoubleBarrierEngine<RNG, S>::withSamples(Size samples) {
        QL_REQUIRE(tolerance_ == Null<Real>(), "tolerance already set");
        samples_ = samples;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCDiscreteDoubleBarrierEngine<RNG, S>&
    MakeMCDiscreteDoubleBarrierEngine<RNG, S>::withAbsoluteTolerance(
        Real tolerance) {
        QL_REQUIRE(samples_ == Null<Size>(),
                   "number of samples already set");
        QL_REQUIRE(RNG::allowsErrorEstimate,
                   "chosen random generator policy does not allow an error "
                   "estimate");
        tolerance_ = tolerance;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCDiscreteDoubleBarrierEngine<RNG, S>&
    MakeMCDiscreteDoubleBarrierEngine<RNG, S>::withMaxSamples(Size samples) {
        maxSamples_ = samples;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCDiscreteDoubleBarrierEngine<RNG, S>&
    MakeMCDiscreteDoubleBarrierEngine<RNG, S>::withSeed(BigNatural seed) {
        seed_ = seed;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCDiscreteDoubleBarrierEngine<RNG, S>::
        operator ext::shared_ptr<PricingEngine>() const {
        return ext::shared_ptr<PricingEngine>(
            new MCDiscreteDoubleBarrierEngine<RNG, S>(process_,
                                                      brownianBridge_,
                                                      antithetic_,
                                                      samples_,
                                                      tolerance_,
                                                      maxSamples_,
                                                      seed_));
    }

}

#endif
