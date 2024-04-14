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

#ifndef CepGenEpIC_ScenarioParser_h
#define CepGenEpIC_ScenarioParser_h

#include <CepGen/Core/SteeredObject.h>
#include <automation/MonteCarloScenario.h>
#include <partons/beans/automation/BaseObjectData.h>

namespace cepgen {
  namespace epic {
    class ScenarioParser : public SteeredObject<ScenarioParser>, public EPIC::MonteCarloScenario {
    public:
      explicit ScenarioParser(const ParametersList&);

      static ParametersDescription description();

    private:
      EPIC::MonteCarloTask parseTask(const ParametersList&);
      PARTONS::BaseObjectData parseParameters(const ParametersList&, PARTONS::BaseObjectData&);
    };
  }  // namespace epic
}  // namespace cepgen

#endif
