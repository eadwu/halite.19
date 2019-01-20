#pragma once

#include "entity.hpp"
#include "constants.hpp"
#include "command.hpp"

#include <memory>

namespace hlt {
    struct Ship : Entity {
        Halite halite;
        bool suicidal;
        bool returning;

        Ship(PlayerId player_id, EntityId ship_id, int x, int y, Halite halite) :
            Entity(player_id, ship_id, x, y),
            halite(halite),
            suicidal(false),
            returning(false)
        {}

        bool is_full() const {
            return halite >= constants::MAX_HALITE;
        }

        bool is_suicidal() const {
            return suicidal;
        }

        bool is_returning() const {
            return returning;
        }

        void reset_status() {
            suicidal = false;
            returning = false;
        }

        Command make_dropoff() const {
            return hlt::command::transform_ship_into_dropoff_site(id);
        }

        Command move(Direction direction) const {
            return hlt::command::move(id, direction);
        }

        Command stay_still() const {
            return hlt::command::move(id, Direction::STILL);
        }

        static std::shared_ptr<Ship> _generate(PlayerId player_id);
    };

    static std::ostream& operator<<(std::ostream& out, const std::shared_ptr<Ship> ship) {
        out << 'S' << ship->id;
        return out;
    }
}
