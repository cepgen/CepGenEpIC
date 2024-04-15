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

#include <CepGen/Core/Exception.h>
#include <CepGen/Event/Event.h>
#include <CepGen/Modules/ProcessFactory.h>
#include <CepGen/Physics/PDG.h>
#include <CepGen/Process/Process.h>
#include <CepGen/Utils/Filesystem.h>
#include <CepGen/Utils/String.h>

#include <cstring>

// Partons includes
#include <ElementaryUtils/logger/CustomException.h>

// EpIC includes
#include <Epic.h>
#include <automation/MonteCarloScenario.h>
#include <managers/RandomSeedManager.h>
#include <managers/ServiceObjectRegistry.h>
#include <services/AutomationService.h>
#include <services/DDVCSGeneratorService.h>
#include <services/DVCSGeneratorService.h>
#include <services/DVMPGeneratorService.h>
#include <services/GAM2GeneratorService.h>
#include <services/TCSGeneratorService.h>

#include "CepGenEpIC/ProcessInterface.h"
#include "CepGenEpIC/ScenarioParser.h"

using namespace cepgen;
using namespace std::string_literals;

/// Interface object to an EpIC process
class EpICProcess final : public cepgen::proc::Process {
public:
  explicit EpICProcess(const ParametersList& params)
      : cepgen::proc::Process(params), seed_(steer<unsigned long long>("seed")) {}
  EpICProcess(const EpICProcess& oth) : EpICProcess(oth.parameters()) {}

  ~EpICProcess() {
    if (epic_)
      epic_->close();
  }

  proc::ProcessPtr clone() const override { return proc::ProcessPtr(new EpICProcess(*this)); }

  static ParametersDescription description() {
    auto desc = cepgen::proc::Process::description();
    desc.setDescription("EpIC process");
    desc += cepgen::epic::ScenarioParser::description();
    desc.add("seed", 42ull).setDescription("initial random seed");
    desc.add("process", ""s).setDescription("type of process to consider");
    return desc;
  }

private:
  void prepareKinematics() override {
    // initialise the EpIC instance
    auto args = parseArguments();
    epic_ = EPIC::Epic::getInstance();
    epic_->init(args.size(), args.data());
    epic_->getRandomSeedManager()->setSeedCount(seed_);
    const auto scenario = cepgen::epic::ScenarioParser(params_);
    for (auto& task : scenario.getTasks()) {
      const auto& name = task.getServiceName();
      if (name == "DVCSGeneratorService")
        epic_proc_.reset(new epic::ProcessServiceInterface(
            epic_->getServiceObjectRegistry()->getDVCSGeneratorService(), scenario, task, [](auto& ranges) {
              ranges.at(0).setMinMax(log(ranges.at(0).getMin()), log(ranges.at(0).getMax()));
              ranges.at(1).setMinMax(log(ranges.at(1).getMin()), log(ranges.at(1).getMax()));
              ranges.at(2).setMinMax(log(-1 * ranges.at(2).getMax()), log(-1 * ranges.at(2).getMin()));
            }));
      else if (name == "TCSGeneratorService")
        epic_proc_.reset(new epic::ProcessServiceInterface(
            epic_->getServiceObjectRegistry()->getTCSGeneratorService(), scenario, task, [](auto& ranges) {
              ranges.at(0).setMinMax(log(-1 * ranges.at(0).getMax()), log(-1 * ranges.at(0).getMin()));
              ranges.at(1).setMinMax(log(ranges.at(1).getMin()), log(ranges.at(1).getMax()));
              ranges.at(5).setMinMax(log(ranges.at(5).getMin()), log(ranges.at(5).getMax()));
              ranges.at(6).setMinMax(log(ranges.at(6).getMin()), log(ranges.at(6).getMax()));
            }));
      else if (name == "DVMPGeneratorService")
        epic_proc_.reset(new epic::ProcessServiceInterface(
            epic_->getServiceObjectRegistry()->getDVMPGeneratorService(), scenario, task, [](auto& ranges) {
              ranges.at(0).setMinMax(log(ranges.at(0).getMin()), log(ranges.at(0).getMax()));
              ranges.at(1).setMinMax(log(ranges.at(1).getMin()), log(ranges.at(1).getMax()));
              ranges.at(2).setMinMax(log(-1 * ranges.at(2).getMax()), log(-1 * ranges.at(2).getMin()));
            }));
      else if (name == "GAM2GeneratorService")
        epic_proc_.reset(new epic::ProcessServiceInterface(
            epic_->getServiceObjectRegistry()->getGAM2GeneratorService(), scenario, task, [](auto& ranges) {
              ranges.at(0).setMinMax(log(-1 * ranges.at(0).getMax()), log(-1 * ranges.at(0).getMin()));
              ranges.at(2).setMinMax(log(ranges.at(2).getMin()), log(ranges.at(2).getMax()));
              ranges.at(4).setMinMax(log(ranges.at(4).getMin()), log(ranges.at(4).getMax()));
              ranges.at(5).setMinMax(log(ranges.at(5).getMin()), log(ranges.at(5).getMax()));
            }));
      else if (name == "DDVCSGeneratorService")
        epic_proc_.reset(new epic::ProcessServiceInterface(
            epic_->getServiceObjectRegistry()->getDDVCSGeneratorService(), scenario, task, [](auto& ranges) {
              ranges.at(0).setMinMax(log(ranges.at(0).getMin()), log(ranges.at(0).getMax()));
              ranges.at(1).setMinMax(log(ranges.at(1).getMin()), log(ranges.at(1).getMax()));
              ranges.at(2).setMinMax(log(-1 * ranges.at(2).getMax()), log(-1 * ranges.at(2).getMin()));
              ranges.at(3).setMinMax(log(ranges.at(3).getMin()), log(ranges.at(3).getMax()));
            }));
      CG_INFO("EpICProcess:prepareKinematics") << "New '" << name << "' task built.";
    }
    coords_.resize(epic_proc_->ndim());
    for (size_t i = 0; i < epic_proc_->ndim(); ++i)
      defineVariable(coords_.at(i), Mapping::linear, epic_proc_->ranges().at(i), utils::format("x_%zu", i));
    CG_DEBUG("EpICProcess:prepareKinematics") << "Phase space mapped for dim-" << coords_.size() << " integrand.";
  }
  void addEventContent() override {
    proc::Process::setEventContent({{Particle::IncomingBeam1, {PDG::electron}},
                                    {Particle::IncomingBeam2, {PDG::proton}},
                                    {Particle::Parton1, {PDG::photon}},
                                    {Particle::Parton2, {PDG::photon}},
                                    {Particle::OutgoingBeam1, {PDG::electron}},
                                    {Particle::OutgoingBeam2, {PDG::proton}},
                                    {Particle::CentralSystem, {PDG::muon, PDG::muon}}});
  }
  double computeWeight() override { return epic_proc_->weight(coords_) * 1.e3; }
  void fillKinematics() override {}

  std::vector<char*> parseArguments() const {
    const auto args = std::vector<std::string>{
        fs::current_path() / "data" / "partons.properties", utils::format("--seed=%zu", seed_), "--scenario=''"};
    CG_DEBUG("EpICProcess:parseArguments") << "List of arguments handled:\n\t" << args << ".";
    std::vector<char*> argv;
    std::transform(args.begin(), args.end(), std::back_inserter(argv), [](const std::string& str) -> char* {
      char* out = new char[str.size() + 1];
      std::strcpy(out, str.data());
      return out;
    });
    return argv;
  }

  const unsigned long long seed_;
  EPIC::Epic* epic_{nullptr};  //NOT owning
  std::unique_ptr<epic::ProcessInterface> epic_proc_{nullptr};
  std::vector<double> coords_;
};
REGISTER_PROCESS("epic", EpICProcess);
