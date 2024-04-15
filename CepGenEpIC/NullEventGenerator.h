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

#ifndef CepGenEpIC_NullEventGenerator_h
#define CepGenEpIC_NullEventGenerator_h

#include <CepGen/Core/Exception.h>
#include <services/GeneratorService.h>

#include <memory>

namespace cepgen {
  namespace epic {
    class NullEventGenerator : public EPIC::EventGeneratorModule {
    public:
      using EPIC::EventGeneratorModule::EventGeneratorModule;
      explicit NullEventGenerator(const std::string& name = "NullEventGenerator");
      NullEventGenerator(const NullEventGenerator&);
      virtual ~NullEventGenerator() = default;

      static const unsigned int classId;
      NullEventGenerator* clone() const override;

      void configure(const ElemUtils::Parameters&);
      void initialise(const std::vector<EPIC::KinematicRange>&, const EPIC::EventGeneratorInterface&) override;
      std::pair<std::vector<double>, double> generateEvent() override;
      std::pair<double, double> getIntegral() override { return std::make_pair(1., 1.); }
    };
  }  // namespace epic
}  // namespace cepgen

#endif
