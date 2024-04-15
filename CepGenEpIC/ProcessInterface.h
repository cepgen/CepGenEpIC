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

#include <vector>

namespace cepgen {
  namespace epic {
    class ProcessInterface {
    public:
      ProcessInterface() {}
      virtual const std::vector<Limits> ranges() const = 0;
      virtual size_t ndim() const = 0;
      virtual double weight(std::vector<double>&) const = 0;
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
          : service_(service), set_ranges_transform_(rng_transform) {
        if (!service_)
          throw CG_FATAL("ProcessServiceInterface")
              << "Failed to interface the EPIC generator service to build a CepGen-compatible process definition.";
        service_->setScenarioDescription(scenario.getDescription());
        service_->setScenarioDate(scenario.getDate());
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

        CG_INFO("ProcessServiceInterface") << "Process service interface initialised for dimension-" << ndim() << " '"
                                           << service_->getClassName() << "' process.\n"
                                           << "\tKinematic ranges: " << ranges_ << ".";
      }
      const std::vector<Limits> ranges() const override { return ranges_; }
      size_t ndim() const override { return ranges_.size(); }
      double weight(std::vector<double>& coords) const override { return service_->getEventDistribution(coords); }

    private:
      T* service_{nullptr};
      std::vector<Limits> ranges_;
      const RangeTransformation set_ranges_transform_{nullptr};
    };
  }  // namespace epic
}  // namespace cepgen

#endif
