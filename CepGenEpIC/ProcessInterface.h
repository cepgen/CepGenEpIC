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
        service_->computeTask(task);
        if (!service_->getKinematicModule()->runTest())
          CG_WARNING("ProcessServiceInterface") << "Kinematic module test failed.";
        evt_gen_ = dynamic_cast<EventGenerator*>(service_->getEventGeneratorModule().get());
        ranges_ = evt_gen_->ranges();
        service_->setRanges(ranges_);
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
        evt_gen_->setCoordinates(coords);
        service_->run();
        event = writer_->event();
        return service_->getEventDistribution(const_cast<std::vector<double>&>(coords));
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
