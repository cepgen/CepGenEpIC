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

#include <ElementaryUtils/logger/CustomException.h>
#include <Epic.h>
#include <automation/MonteCarloScenario.h>
#include <automation/MonteCarloTask.h>
#include <managers/RandomSeedManager.h>
#include <managers/ServiceObjectRegistry.h>
#include <partons/Partons.h>
#include <partons/ServiceObjectRegistry.h>
#include <partons/services/automation/AutomationService.h>
#include <services/AutomationService.h>
#include <services/DDVCSGeneratorService.h>
#include <services/DVCSGeneratorService.h>
#include <services/DVMPGeneratorService.h>
#include <services/GAM2GeneratorService.h>
#include <services/TCSGeneratorService.h>

#include <cstring>

#include "CepGen/Core/Exception.h"
#include "CepGen/Event/Event.h"
#include "CepGen/Modules/ProcessFactory.h"
#include "CepGen/Process/Process.h"
#include "CepGen/Utils/Filesystem.h"
#include "CepGen/Utils/String.h"

using namespace cepgen;
using namespace std::string_literals;

/// Interface object to an EpIC process
class EpICProcess final : public cepgen::proc::Process {
public:
  explicit EpICProcess(const ParametersList& params)
      : cepgen::proc::Process(params), epic_(EPIC::Epic::getInstance()), partons_(PARTONS::Partons::getInstance()) {
    const auto seed = steer<unsigned long long>("seed");
    const auto scenario_file = steerPath("scenario");
    // initialise Partons/EpIC with an argc/argv pair
    std::vector<std::string> args{fs::canonical("/proc/self/exe").parent_path().parent_path() / "partons" /
                                  "partons.properties"};
    args.emplace_back(utils::format("--seed=%zu", seed));
    args.emplace_back("--scenario=" + scenario_file);
    std::vector<char*> argv;
    std::transform(args.begin(), args.end(), std::back_inserter(argv), [](const std::string& str) -> char* {
      char* out = new char[str.size() + 1];
      std::strcpy(out, str.data());
      return out;
    });
    try {
      partons_->init(args.size(), argv.data());
      epic_->init(args.size(), argv.data());
    } catch (const ElemUtils::CustomException& exc) {
      throw CG_FATAL("EpICProcess") << "Fatal EpIC/Partons exception: " << exc.what();
    }
    // steer Partons with the scenario card
    auto* pAutomationService = partons_->getServiceObjectRegistry()->getAutomationService();
    auto* pScenario = pAutomationService->parseXMLFile(scenario_file);
    pAutomationService->playScenario(pScenario);
    // initialise the EpIC instance
    epic_->getRandomSeedManager()->setSeedCount(seed);
    auto scenario = EPIC::AutomationService::getInstance()->parseXMLFile(scenario_file);
    for (auto it : scenario->getTasks()) {
      if (it.getServiceName() == "DVCSGeneratorService") {
        auto* generatorService = epic_->getServiceObjectRegistry()->getDVCSGeneratorService();
        generatorService->setScenarioDescription(scenario->getDescription());
        generatorService->setScenarioDate(scenario->getDate());
        generatorService->computeTask(it);
      }
      if (it.getServiceName() == "TCSGeneratorService") {
        auto* generatorService = epic_->getServiceObjectRegistry()->getTCSGeneratorService();
        generatorService->setScenarioDescription(scenario->getDescription());
        generatorService->setScenarioDate(scenario->getDate());
        generatorService->computeTask(it);
      }
      if (it.getServiceName() == "DVMPGeneratorService") {
        auto* generatorService = epic_->getServiceObjectRegistry()->getDVMPGeneratorService();
        generatorService->setScenarioDescription(scenario->getDescription());
        generatorService->setScenarioDate(scenario->getDate());
        generatorService->computeTask(it);
      }
      if (it.getServiceName() == "GAM2GeneratorService") {
        auto* generatorService = epic_->getServiceObjectRegistry()->getGAM2GeneratorService();
        generatorService->setScenarioDescription(scenario->getDescription());
        generatorService->setScenarioDate(scenario->getDate());
        generatorService->computeTask(it);
      }
      if (it.getServiceName() == "DDVCSGeneratorService") {
        auto* generatorService = epic_->getServiceObjectRegistry()->getDDVCSGeneratorService();
        generatorService->setScenarioDescription(scenario->getDescription());
        generatorService->setScenarioDate(scenario->getDate());
        generatorService->computeTask(it);
      }
    }
  }
  ~EpICProcess() {
    if (epic_)
      epic_->close();
    if (partons_)
      partons_->close();
  }

  proc::ProcessPtr clone() const override { return proc::ProcessPtr(new EpICProcess(*this)); }

  static ParametersDescription description() {
    auto desc = cepgen::proc::Process::description();
    desc.setDescription("EpIC process");
    desc.add("scenario", ""s).setDescription("path to xml scenario");
    desc.add("seed", 42ull).setDescription("initial random seed");
    return desc;
  }

private:
  void prepareKinematics() override {}
  void addEventContent() override {}
  double computeWeight() override { return 1.; }
  void fillKinematics() override {}

  std::shared_ptr<EPIC::Epic> epic_{nullptr};
  std::shared_ptr<PARTONS::Partons> partons_{nullptr};
};
REGISTER_PROCESS("epic", EpICProcess);
