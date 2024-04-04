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

#include <memory>

namespace cepgen {
  namespace epic {
    class ProcessInterface {
    public:
      ProcessInterface() {}

      virtual double weight(std::vector<double>&) const = 0;

    protected:
    };
    /// Interface to an EpIC generator service
    /// \tparam T process-specific EpIC generator service object
    template <typename T>
    class ServiceInterface : public ProcessInterface, public std::shared_ptr<T> {
    public:
      explicit ServiceInterface(T* service, std::shared_ptr<EPIC::MonteCarloScenario>& scenario)
          : std::shared_ptr<T>(service) {
        (*this)->setScenarioDescription(scenario->getDescription());
        (*this)->setScenarioDate(scenario->getDate());
      }

      double weight(std::vector<double>& coords) const override { return (*this)->getEventDistribution(coords); }
    };
  }  // namespace epic
}  // namespace cepgen

#endif
