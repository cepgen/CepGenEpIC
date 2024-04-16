/*
 *  CepGen: a central exclusive processes event generator
 *  Copyright (C) 2024  Laurent Forthomme
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CepGenEpIC_ProcessInterface_h
#define CepGenEpIC_ProcessInterface_h

#include <CepGen/Core/Exception.h>
#include <CepGen/Utils/Limits.h>
#include <automation/MonteCarloTask.h>
#include <services/GeneratorService.h>

#include <vector>

#include "CepGenEpIC/EventGenerator.h"
#include "CepGenEpIC/Writer.h"

namespace cepgen {
  namespace epic {
    class ProcessInterface {
    public:
      ProcessInterface() {}
      virtual const std::vector<Limits> ranges() const = 0;
      virtual size_t ndim() const = 0;
      virtual double generate(const std::vector<double>&, Event&) const = 0;
    };

    template <typename T>
    class ProcessServiceWrapper : public T {
    public:
      void setRanges(const std::vector<Limits>& ranges) {
        T::m_histograms.clear();
        for (const auto& range : ranges)
          T::m_histograms.emplace_back(
              new TH1D(Form("h_%zu", T::m_histograms.size()), "", 100, range.min(), range.max()));
      }
      void bookHistograms() override {}
    };

    /// Interface to an EpIC generator service
    template <typename T>
    class ProcessServiceInterface : public ProcessInterface {
    public:
      using RangeTransformation = std::function<void(std::vector<EPIC::KinematicRange>&)>;

      explicit ProcessServiceInterface(T* service,
                                       const EPIC::MonteCarloScenario& scenario,
                                       const EPIC::MonteCarloTask& task,
                                       const RangeTransformation& rng_transform = nullptr)
          : service_(static_cast<ProcessServiceWrapper<T>*>(service)), set_ranges_transform_(rng_transform) {
        if (!service_)
          throw CG_FATAL("ProcessServiceInterface")
              << "Failed to interface the EPIC generator service to build a CepGen-compatible process definition.";
        service_->setScenarioDescription(scenario.getDescription());
        service_->setScenarioDate(scenario.getDate());
        service_->setRanges(std::vector<Limits>(10, {0., 1.}));
        service_->computeTask(task);
        if (!service_->getKinematicModule()->runTest())
          CG_WARNING("ProcessServiceInterface") << "Kinematic module test failed.";
        auto kin_ranges = service_->getKinematicModule()->getKinematicRanges(service_->getExperimentalConditions(),
                                                                             service_->getKinematicRanges());
        if (set_ranges_transform_)
          set_ranges_transform_(kin_ranges);  // apply some process-specific variables changes
        const auto& rc_var_ranges = service_->getRCModule()->getVariableRanges();
        kin_ranges.insert(kin_ranges.end(), rc_var_ranges.begin(), rc_var_ranges.end());  // add RC variables, if any
        for (const auto& range : kin_ranges)
          ranges_.emplace_back(Limits{range.getMin(), range.getMax()});
        service_->setRanges(ranges_);
        evt_gen_ = dynamic_cast<EventGenerator*>(service_->getEventGeneratorModule().get());
        std::vector<double> coords(ranges_.size(), 0.5);
        evt_gen_->setCoordinates(coords);
        writer_ = dynamic_cast<Writer*>(service_->getWriterModule().get());
        CG_INFO("ProcessServiceInterface") << "Process service interface initialised for dimension-" << ndim() << " '"
                                           << service_->getClassName() << "' process.\n"
                                           << "\tKinematic ranges: " << ranges_ << ".";
        auto general_params = service_->getGeneralConfiguration();
        general_params.setNEvents(1);
        service_->setGeneralConfiguration(general_params);
      }
      const std::vector<Limits> ranges() const override { return ranges_; }
      size_t ndim() const override { return ranges_.size(); }
      double generate(const std::vector<double>& coords, Event& event) const override {
        std::vector<double> rescl_coords;
        for (size_t i = 0; i < coords.size(); ++i)
          rescl_coords.emplace_back(ranges_.at(i).x(coords.at(i)));
        evt_gen_->setCoordinates(rescl_coords);
        service_->run();
        if (writer_->event().empty())
          return 0.;
        event = writer_->event();
        return service_->getEventDistribution(rescl_coords);
      }

    private:
      ProcessServiceWrapper<T>* service_{nullptr};
      std::vector<Limits> ranges_;
      const RangeTransformation set_ranges_transform_{nullptr};
      EventGenerator* evt_gen_{nullptr};
      Writer* writer_{nullptr};
    };
  }  // namespace epic
}  // namespace cepgen

#endif
