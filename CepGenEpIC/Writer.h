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

#ifndef CepGenEpIC_Writer_h
#define CepGenEpIC_Writer_h

#include <CepGen/Core/Exception.h>
#include <CepGen/Event/Event.h>
#include <modules/writer/WriterModule.h>

#include <memory>

namespace cepgen {
  namespace epic {
    class Writer : public EPIC::WriterModule {
    public:
      using EPIC::WriterModule::WriterModule;
      explicit Writer(const std::string& name = "cepgen::epic::Writer");
      Writer(const Writer&);
      virtual ~Writer() = default;

      static const unsigned int classId;
      Writer* clone() const override;

      void open() override {}
      void saveGenerationInformation(const EPIC::GenerationInformation&) override {}
      void close() override {}
      void write(const EPIC::Event& evt) override;
      void write(const std::vector<EPIC::Event>&) override {}

      const Event& event() const { return evt_; }

    private:
      Event evt_;
      std::map<size_t, size_t> cg_vs_epic_;
    };
  }  // namespace epic
}  // namespace cepgen

#endif
