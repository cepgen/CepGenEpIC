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

#include <beans/physics/Particle.h>
#include <beans/physics/Vertex.h>
#include <partons/BaseObjectRegistry.h>

#include "CepGenEpIC/Writer.h"

namespace cepgen {
  namespace epic {
    const unsigned int Writer::classId =
        PARTONS::BaseObjectRegistry::getInstance()->registerBaseObject(new Writer("cepgen::epic::Writer"));

    Writer::Writer(const std::string& name) : EPIC::WriterModule(name) {}

    Writer::Writer(const Writer& oth) : EPIC::WriterModule(oth) {}

    Writer* Writer::clone() const { return new Writer(*this); }

    void Writer::write(const EPIC::Event& evt) {
      static const auto convert_mom = [](const auto& epic_mom) -> Momentum {
        return Momentum::fromPxPyPzE(epic_mom.Px(), epic_mom.Py(), epic_mom.Pz(), epic_mom.E());
      };
      if (!initialised_) {  // first initialisation of the event content
        size_t i = 0;
        for (const auto& type_vs_ppart : evt.getParticles()) {
          const auto& mom = type_vs_ppart.second->getFourMomentum();
          auto role = Particle::Role::CentralSystem;
          if (type_vs_ppart.first == EPIC::ParticleCodeType::BEAM)
            role = mom.Pz() > 0. ? Particle::Role::IncomingBeam1 : Particle::Role::IncomingBeam2;
          auto part = evt_.addParticle(role);
          part.get().setMomentum(convert_mom(mom), true).setIntegerPdgId(type_vs_ppart.second->getType());
          cg_vs_epic_[part.get().id()] = i++;
        }
        const auto find_part_equiv = [this](const auto& epic_part) -> ParticleRef {
          for (const auto& role : evt_.roles())
            for (auto& part : evt_[role]) {
              if (part.get().momentum().p() == convert_mom(epic_part->getFourMomentum()).p())
                return part;
            }
          throw CG_FATAL("epic:Writer") << "Failed to find an equivalence between the EpIC and CepGen particles.";
        };
        for (const auto& pvtx : evt.getVertices()) {
          const auto &pins = pvtx->getParticlesIn(), &pouts = pvtx->getParticlesOut();
          for (const auto& pout : pouts) {
            auto evt_pout = find_part_equiv(pout);
            for (const auto& pin : pins) {
              auto& evt_pin = find_part_equiv(pin).get();
              evt_pout.get().addMother(evt_pin);
              if (evt_pin.role() == Particle::Role::IncomingBeam1) {
                if (evt_pout.get().integerPdgId() == evt_pin.integerPdgId())
                  evt_pout.get().setRole(Particle::Role::OutgoingBeam1);
              } else if (evt_pin.role() == Particle::Role::IncomingBeam2) {
                if (evt_pout.get().integerPdgId() == evt_pin.integerPdgId())
                  evt_pout.get().setRole(Particle::Role::OutgoingBeam2);
              } else
                evt_pin.setRole(Particle::Role::Intermediate);
            }
          }
        }
        initialised_ = true;
        return;
      }
      const auto& parts = evt.getParticles();
      for (size_t i = 0; i < parts.size(); ++i)
        evt_[cg_vs_epic_.at(i)].setMomentum(convert_mom(parts.at(i).second->getFourMomentum()), true);
    }

    void Writer::write(const std::vector<EPIC::Event>&) { throw CG_FATAL("epic:Writer:write") << "Not implemented."; }
  }  // namespace epic
}  // namespace cepgen
