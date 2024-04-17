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

#include "CepGenEpIC/EventGenerator.h"

namespace cepgen {
  namespace epic {
    const unsigned int EventGenerator::classId = PARTONS::BaseObjectRegistry::getInstance()->registerBaseObject(
        new EventGenerator("cepgen::epic::EventGenerator"));

    EventGenerator::EventGenerator(const std::string& name) : EPIC::EventGeneratorModule(name) {}

    EventGenerator::EventGenerator(const EventGenerator& oth) : EPIC::EventGeneratorModule(oth) {}

    EventGenerator* EventGenerator::clone() const { return new EventGenerator(*this); }

    void EventGenerator::configure(const ElemUtils::Parameters& params) { EventGeneratorModule::configure(params); }

    void EventGenerator::initialise(const std::vector<EPIC::KinematicRange>& ranges,
                                    const EPIC::EventGeneratorInterface& gen_interface) {
      ranges_.clear();
      for (const auto& range : ranges)  // transforme EPIC ranges into CepGen Limits
        ranges_.emplace_back(Limits{range.getMin(), range.getMax()});
      // initialise the coordinates: "shoot" right in the middle of the range
      coords_.clear();
      for (const auto& range : ranges_)
        coords_.emplace_back(range.x(0.5));
      CG_DEBUG("epic:EventGenerator") << "Prepared for cross section computation and event generation: "
                                      << "f(" << coords_ << ") = " << gen_interface.getEventDistribution(coords_)
                                      << ".";
    }

    void EventGenerator::setCoordinates(const std::vector<double>& coords) {
      for (size_t i = 0; i < ranges_.size(); ++i)
        coords_[i] = ranges_.at(i).x(coords.at(i));
    }
  }  // namespace epic
}  // namespace cepgen
