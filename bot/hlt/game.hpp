#pragma once

#include "game_map.hpp"
#include "player.hpp"
#include "types.hpp"

#include <vector>
#include <numeric>
#include <iostream>

namespace hlt {
    struct Game {
        int turn_limit;
        int turn_number;
        PlayerId my_id;
        std::vector<std::shared_ptr<Player>> players;
        std::shared_ptr<Player> me;
        std::unique_ptr<GameMap> game_map;

        Game();
        void ready(const std::string& name);
        void update_frame();
        bool end_turn(const std::vector<Command>& commands);

        int average_enemy_ships() {
            return (game_map->total_ships() - me->ships.size()) / (players.size() - 1);
        }

        std::vector<std::shared_ptr<Ship>> enemy_ships_in_range(Position& pos, int range) {
            std::vector<std::shared_ptr<Ship>> ships = game_map->ships_in_range(pos, range);
            const auto& end = std::remove_if(ships.begin(), ships.end(), [&](const auto& s) {
                return (s->owner == my_id);
            });

            ships.erase(end, ships.end());
            return ships;
        }
    };
}
