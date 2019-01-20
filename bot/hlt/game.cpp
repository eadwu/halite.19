#include "game.hpp"
#include "input.hpp"

#include <sstream>

hlt::Game::Game() : turn_number(0) {
    std::ios_base::sync_with_stdio(false);

    hlt::constants::populate_constants(hlt::get_string());

    int num_players;
    std::stringstream input(get_string());
    input >> num_players >> my_id;

    log::open(my_id);

    for (int i = 0; i < num_players; ++i) {
        players.push_back(Player::_generate());
    }
    me = players[my_id];
    game_map = GameMap::_generate();
    turn_limit = 400 + (game_map->width - 32) / 8 * 25;
}

void hlt::Game::ready(const std::string& name) {
    std::cout << name << std::endl;
}

void hlt::Game::update_frame() {
    hlt::get_sstream() >> turn_number;
    log::log("=============== TURN " + std::to_string(turn_number) + " ================");

    for (size_t i = 0; i < players.size(); ++i) {
        PlayerId current_player_id;
        int num_ships;
        int num_dropoffs;
        Halite halite;
        hlt::get_sstream() >> current_player_id >> num_ships >> num_dropoffs >> halite;

        players[current_player_id]->_update(num_ships, num_dropoffs, halite);
    }

    game_map->_update();

    for (auto& row : game_map->cells) {
        for (auto& cell : row) {
            cell.priority = enemy_ships_in_range(cell.position, 4).size() >= 2 ? cell.halite * 2 : cell.halite;
        }
    }

    for (const auto& player : players) {
        for (auto& ship_iterator : player->ships) {
            auto ship = ship_iterator.second;
            game_map->at(ship)->mark_unsafe(ship);

            if (player->id != me->id) {
                // game_map->exude_priority_in_range(game_map->at(ship)->position, 2, -1000);
                // game_map->exude_priority_in_range(game_map->at(ship)->position, 2, -1.0 * (game_map->at(ship)->halite / 2));
            }
        }

        game_map->at(player->shipyard)->structure = player->shipyard;

        for (auto& dropoff_iterator : player->dropoffs) {
            auto dropoff = dropoff_iterator.second;
            game_map->at(dropoff)->structure = dropoff;
        }
    }
}

bool hlt::Game::end_turn(const std::vector<hlt::Command>& commands) {
    for (const auto& command : commands) {
        std::cout << command << ' ';
    }
    std::cout << std::endl;
    return std::cout.good();
}
