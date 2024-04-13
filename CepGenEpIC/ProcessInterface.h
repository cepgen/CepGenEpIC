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
#include <automation/MonteCarloTask.h>
#include <partons/BaseObjectRegistry.h>
#include <services/GeneratorService.h>

#include <memory>

namespace cepgen {
  namespace epic {
    class ProcessInterface {
    public:
      ProcessInterface() {}

      virtual double weight(std::vector<double>&) const = 0;

    protected:
    };

    class NullEventGenerator : public EPIC::EventGeneratorModule {
    public:
      using EPIC::EventGeneratorModule::EventGeneratorModule;
      static const unsigned int classId;
      explicit NullEventGenerator(const std::string& name = "NullEventGenerator") : EventGeneratorModule(name) {}
      void initialise(const std::vector<EPIC::KinematicRange>&, const EPIC::EventGeneratorInterface&) override {}
      std::pair<std::vector<double>, double> generateEvent() override {
        return std::make_pair(std::vector<double>{}, 0.);
      }
      std::pair<double, double> getIntegral() override { return std::make_pair(0., 0.); }
      PARTONS::ModuleObject* clone() const override { return nullptr; }
    };

    /// Interface to an EpIC generator service
    template <typename T>
    class ProcessServiceInterface : public ProcessInterface {
    public:
      explicit ProcessServiceInterface(T* service,
                                       const EPIC::MonteCarloScenario& scenario,
                                       const EPIC::MonteCarloTask& task)
          : service_(service) {
        if (!service_)
          throw CG_FATAL("ProcessServiceInterface")
              << "Failed to interface the EPIC generator service to build a CepGen-compatible process definition.";
        service_->setScenarioDescription(scenario.getDescription());
        service_->setScenarioDate(scenario.getDate());
        //service_->initialise(task);
        service_->computeTask(task);
      }
      double weight(std::vector<double>& coords) const override {
        CG_ASSERT(service_);
        return service_->getEventDistribution(coords);
      }

    private:
      T* service_{nullptr};
    };
  }  // namespace epic
}  // namespace cepgen

#endif
