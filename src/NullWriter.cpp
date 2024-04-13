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

#include "CepGenEpIC/NullWriter.h"

namespace cepgen {
  namespace epic {
    const unsigned int NullWriter::classId =
        PARTONS::BaseObjectRegistry::getInstance()->registerBaseObject(new NullWriter("NullWriter"));

    NullWriter::NullWriter(const std::string& name) : EPIC::WriterModule(name) {}

    NullWriter::NullWriter(const NullWriter& oth) : EPIC::WriterModule(oth) {}

    NullWriter* NullWriter::clone() const { return new NullWriter(*this); }
  }  // namespace epic
}  // namespace cepgen
