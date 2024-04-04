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

#include <Epic.h>
#include <partons/Partons.h>

#include "CepGen/Core/Exception.h"
#include "CepGen/Event/Event.h"
#include "CepGen/Modules/ProcessFactory.h"
#include "CepGen/Process/Process.h"

using namespace cepgen;

/// Interface object to an EpIC process
class EpICProcess final : public cepgen::proc::Process {
public:
  explicit EpICProcess(const ParametersList& params) : cepgen::proc::Process(params) {}
  EpICProcess(const EpICProcess& oth) : cepgen::proc::Process(oth) {}
  proc::ProcessPtr clone() const override { return proc::ProcessPtr(new EpICProcess(*this)); }

  static ParametersDescription description() {
    auto desc = cepgen::proc::Process::description();
    desc.setDescription("EpIC process");
    desc.add<double>("value", 1.);
    return desc;
  }

private:
  void prepareKinematics() override {}
  void addEventContent() override {}
  double computeWeight() override { return 1.; }
  void fillKinematics() override {}

  std::unique_ptr<EPIC::Epic> epic_{nullptr};
  std::unique_ptr<PARTONS::Partons> partons_{nullptr};
};
REGISTER_PROCESS("epic", EpICProcess);
