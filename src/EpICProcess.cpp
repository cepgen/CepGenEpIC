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
#include <QtCore/qcoreapplication.h>

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

using namespace cepgen;
using namespace std::string_literals;

static std::unique_ptr<QCoreApplication> kQtApp{nullptr};

/// Interface object to an EpIC process
class EpICProcess final : public cepgen::proc::Process {
public:
  explicit EpICProcess(const ParametersList& params)
      : cepgen::proc::Process(params), seed_(steer<unsigned long long>("seed")), scenario_file_(steerPath("scenario")) {
    try {  // initialise Partons/EpIC with an argc/argv pair
      auto args = parseArguments();
      int argc = args.size();
      if (!kQtApp)
        kQtApp.reset(new QCoreApplication(argc, args.data()));
    } catch (const ElemUtils::CustomException& exc) {
      throw CG_FATAL("EpICProcess") << "Fatal EpIC/Partons exception: " << exc.what();
    }
  }
  EpICProcess(const EpICProcess& oth) : EpICProcess(oth.parameters()) {}

  ~EpICProcess() {
    if (epic_)
      epic_->close();
  }

  proc::ProcessPtr clone() const override { return proc::ProcessPtr(new EpICProcess(*this)); }

  static ParametersDescription description() {
    auto desc = cepgen::proc::Process::description();
    desc.setDescription("EpIC process");
    desc.add("scenario", ""s).setDescription("path to xml scenario");
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
    auto parsed_scenario = EPIC::AutomationService::getInstance()->parseXMLFile(scenario_file_);
    for (const auto& parsed_task : parsed_scenario->getTasks()) {
      auto task = EPIC::MonteCarloTask(parsed_task);  // copy to edit
      //task.setWriterConfiguration(PARTONS::BaseObjectData{});
      scenario_.addTask(task);
      const auto& name = task.getServiceName();
      if (name == "DVCSGeneratorService")
        epic_proc_.reset(new epic::ProcessServiceInterface(
            epic_->getServiceObjectRegistry()->getDVCSGeneratorService(), scenario_, task));
      else if (name == "TCSGeneratorService")
        epic_proc_.reset(new epic::ProcessServiceInterface(
            epic_->getServiceObjectRegistry()->getTCSGeneratorService(), scenario_, task));
      else if (name == "DVMPGeneratorService")
        epic_proc_.reset(new epic::ProcessServiceInterface(
            epic_->getServiceObjectRegistry()->getDVMPGeneratorService(), scenario_, task));
      else if (name == "GAM2GeneratorService")
        epic_proc_.reset(new epic::ProcessServiceInterface(
            epic_->getServiceObjectRegistry()->getGAM2GeneratorService(), scenario_, task));
      else if (name == "DDVCSGeneratorService")
        epic_proc_.reset(new epic::ProcessServiceInterface(
            epic_->getServiceObjectRegistry()->getDDVCSGeneratorService(), scenario_, task));
      CG_INFO("EpICProcess:prepareKinematics") << "New '" << name << "' task built.";
    }
    coords_.clear();
    for (size_t i = 0; i < 20; ++i) {
      auto& coord = coords_.emplace_back();
      defineVariable(coord, Mapping::linear, {0., 1.}, utils::format("x_%zu", i));
    }
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
  double computeWeight() override { return epic_proc_->weight(coords_); }
  void fillKinematics() override {}

  std::vector<char*> parseArguments() const {
    const auto args = std::vector<std::string>{fs::current_path() / "data" / "partons.properties",
                                               utils::format("--seed=%zu", seed_),
                                               "--scenario=" + scenario_file_};
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
  const std::string scenario_file_;
  EPIC::Epic* epic_{nullptr};  //NOT owning
  std::unique_ptr<epic::ProcessInterface> epic_proc_{nullptr};
  std::vector<double> coords_;
  EPIC::MonteCarloScenario scenario_;
};
REGISTER_PROCESS("epic", EpICProcess);
