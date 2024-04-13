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

#include <partons/BaseObjectRegistry.h>

#include "CepGenEpIC/NullEventGenerator.h"

namespace cepgen {
  namespace epic {
    const unsigned int NullEventGenerator::classId =
        PARTONS::BaseObjectRegistry::getInstance()->registerBaseObject(new NullEventGenerator("NullEventGenerator"));

    NullEventGenerator::NullEventGenerator(const std::string& name) : EPIC::EventGeneratorModule(name) {}

    NullEventGenerator::NullEventGenerator(const NullEventGenerator& oth) : EPIC::EventGeneratorModule(oth) {}

    NullEventGenerator* NullEventGenerator::clone() const { return new NullEventGenerator(*this); }
  }  // namespace epic
}  // namespace cepgen
