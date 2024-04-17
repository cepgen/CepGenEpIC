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

#ifndef CepGenEpIC_EventGenerator_h
#define CepGenEpIC_EventGenerator_h

#include <CepGen/Core/Exception.h>
#include <CepGen/Utils/Limits.h>
#include <services/GeneratorService.h>

#include <memory>

namespace cepgen {
  namespace epic {
    class EventGenerator : public EPIC::EventGeneratorModule {
    public:
      using EPIC::EventGeneratorModule::EventGeneratorModule;
      explicit EventGenerator(const std::string& name = "cepgen::epic::EventGenerator");
      EventGenerator(const EventGenerator&);
      virtual ~EventGenerator() = default;

      static const unsigned int classId;
      EventGenerator* clone() const override;

      void configure(const ElemUtils::Parameters&);
      void initialise(const std::vector<EPIC::KinematicRange>&, const EPIC::EventGeneratorInterface&) override;
      std::pair<std::vector<double>, double> generateEvent() override { return std::make_pair(coords_, 1.); }
      std::pair<double, double> getIntegral() override { return std::make_pair(1., 1.); }

      void setCoordinates(const std::vector<double>&);
      const std::vector<Limits>& ranges() const { return ranges_; }

    private:
      std::vector<double> coords_;
      std::vector<Limits> ranges_;
    };
  }  // namespace epic
}  // namespace cepgen

#endif
