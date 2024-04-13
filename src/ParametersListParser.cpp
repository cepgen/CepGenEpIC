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

#include <CepGen/Utils/Message.h>

#include "CepGenEpIC/ParametersListParser.h"

namespace cepgen {
  namespace epic {
    EPIC::MonteCarloScenario parseScenario(const ParametersList& params) {
      EPIC::MonteCarloScenario scenario;
      if (params.has<std::string>("date"))
        scenario.setDate(params.get<std::string>("date"));
      if (params.has<std::string>("description"))
        scenario.setDescription(params.get<std::string>("description"));
      for (const auto& plist_task : params.get<std::vector<ParametersList> >("tasks"))
        scenario.addTask(parseTask(plist_task));
      CG_INFO("epic:parseScenario").log([&scenario](auto& log) {
        const auto sep1 = std::string(70, '=') + "\n", sep2 = std::string(70, '-') + "\n";
        log << "Dump of scenario parsed from CepGen configuration\n"
            << "Date: " << scenario.getDate() << "\n"
            << "Description: " << scenario.getDescription() << "\n"
            << "Tasks:\n";
        size_t i = 0;
        for (const auto& task : scenario.getTasks())
          log << sep1 << "Task #" << ++i << "\n"
              << "Service name: " << task.getServiceName() << "\n"
              << "Method name: " << task.getMethodName() << "\n"
              << sep1 << "General configuration\n"
              << sep2 << task.getGeneralConfiguration().toString() << "Kinematics range\n"
              << sep2 << task.getKinematicRange().toString() << "Experimental conditions\n"
              << sep2 << task.getExperimentalConditions().toString() << "Computation configuration\n"
              << sep2 << task.getComputationConfiguration().toString() << "Generator configuration\n"
              << sep2 << task.getGeneratorConfiguration().toString() << "Kinematic configuration\n"
              << sep2 << task.getKinematicConfiguration().toString() << "RC configuration\n"
              << sep2 << task.getRCConfiguration().toString() << "Writer configuration\n"
              << sep2 << task.getWriterConfiguration().toString();
      });
      return scenario;
    }

    EPIC::MonteCarloTask parseTask(const ParametersList& params) {
      EPIC::MonteCarloTask task;
      task.setServiceName(params.name());
      task.setMethodName(params.get<std::string>("method"));
      PARTONS::BaseObjectData gen, kin, exp, comp, gtr, kincfg, rc, wrt;
      task.setGeneralConfiguration(parseParameters(params.get<ParametersList>("general_configuration"), gen));
      task.setKinematicRange(parseParameters(params.get<ParametersList>("kinematic_range"), kin));
      task.setExperimentalConditions(parseParameters(params.get<ParametersList>("experimental_conditions"), exp));
      task.setComputationConfiguration(parseParameters(params.get<ParametersList>("computation_configuration"), comp));
      task.setGeneratorConfiguration(parseParameters(params.get<ParametersList>("generator_configuration"), gtr));
      task.setKinematicConfiguration(parseParameters(params.get<ParametersList>("kinematic_configuration"), kincfg));
      task.setRCConfiguration(parseParameters(params.get<ParametersList>("rc_configuration"), rc));
      task.setWriterConfiguration(parseParameters(params.get<ParametersList>("writer_configuration"), wrt));
      return task;
    }

    PARTONS::BaseObjectData parseParameters(const ParametersList& params, PARTONS::BaseObjectData& obj) {
      if (params.hasName())  // steering a module
        obj.setModuleClassName(params.name());
      if (const auto modules = params.keysOf<ParametersList>(); !modules.empty()) {  // list of submodules
        PARTONS::BaseObjectData mod;
        for (const auto& key : modules) {
          const auto mod_params = params.get<ParametersList>(key);
          mod = obj.addSubModule(key, mod_params.name());
          mod.setModuleType(key);
          parseParameters(mod_params, mod);
        }
        return mod;
      }
      // steering the scalar parameters
      for (const auto& key : params.keysOf<std::string>())
        if (key != cepgen::MODULE_NAME)
          obj.addParameter(ElemUtils::Parameter(key, params.get<std::string>(key)));
      for (const auto& key : params.keysOf<int>())
        obj.addParameter(ElemUtils::Parameter(key, params.get<int>(key)));
      for (const auto& key : params.keysOf<double>())
        obj.addParameter(ElemUtils::Parameter(key, params.get<double>(key)));
      return obj;
    }
  }  // namespace epic
}  // namespace cepgen
