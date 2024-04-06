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

#include <automation/MonteCarloTask.h>
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
    template <class Kr, class P, class Km, class R>
    class GeneratorServiceInterface : public EPIC::GeneratorService<Kr, P, Km, R> {
    public:
      using EGS = EPIC::GeneratorService<Kr, P, Km, R>;

      explicit GeneratorServiceInterface(const EGS& service) : EGS(service) {}

      void initialise(const EPIC::MonteCarloTask& task) {
        CG_WARNING("") << 1;
        EGS::getGeneralConfigurationFromTask(task);
        CG_WARNING("") << 2;
        EGS::getAdditionalGeneralConfigurationFromTask(task);
        EGS::getExperimentalConditionsFromTask(task);
        EGS::getKinematicRangesFromTask(task);
        EGS::getProcessModuleFromTask(task);
        EGS::getEventGeneratorModuleFromTask(task);
        EGS::getKinematicModuleFromTask(task);
        EGS::getRCModuleFromTask(task);
        //EGS::getWriterModuleFromTask(task);
      }
    };
    /// Interface to an EpIC generator service
    template <class Kr, class P, class Km, class R>
    class ServiceInterface : public ProcessInterface {
    public:
      using EGS = EPIC::GeneratorService<Kr, P, Km, R>;
      using GSI = GeneratorServiceInterface<Kr, P, Km, R>;

      explicit ServiceInterface(EGS* service,
                                //const std::shared_ptr<EPIC::MonteCarloScenario>& scenario,
                                const EPIC::MonteCarloScenario& scenario,
                                const EPIC::MonteCarloTask& task)
          : service_(service) {
        service_->setScenarioDescription(scenario.getDescription());
        service_->setScenarioDate(scenario.getDate());
        interface()->initialise(task);
      }
      double weight(std::vector<double>& coords) const override { return service_->getEventDistribution(coords); }

    private:
      EGS* service_;
      GSI* interface() { return dynamic_cast<GSI*>(service_); }
    };
  }  // namespace epic
}  // namespace cepgen

#endif
