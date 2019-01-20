#include "hlt/game.hpp"
#include "hlt/constants.hpp"
#include "hlt/log.hpp"

#include <math.h>

#include <chrono>
#include <sstream>
#include <algorithm>

using namespace std;
using namespace hlt;
using namespace std::chrono;

/**
 * NOTE: Register golden ratio as 8 / 25
 *
 * TODO: Refinement
 * NOTE: Pathfinding using priority
 * NOTE: Swapping is only "intrinsically" supported, not explicit
 *
 * TODO: Priority mapping
 * NOTE: Temporary decay function: halite * 0.5 ** distance; Look at exponential decay
 * NOTE: My ships should repel in an attempt to reduce focus and clumping
 * NOTE: Collisions shouldn't occur unless 100% neccessary
 *       Take note of the shipyard blocking meta (whether on its borders or cell)
 * NOTE: Halite should exude to make clumps of halite more attractive
 *       "Inspired" ship locations should also be kept track of
 * NOTE: Enemy ship priority distribution should be calculated at the start of every turn
 *       My ships' priority distribution should be calculated on the go
 *
 * TODO: Integrate dropoffs
 * NOTE: Pathfinding should respect dropoffs
 * NOTE: 16-18 ships per dropoff ratio, so around 0-4 in most matches
 * NOTE: Certain distance between other shipyards and dropoffs
 * NOTE: Decide where to place by efficiency, around 4-8 radius should have a certain efficiency/profit
 *
 * TODO: Offensive strategy, annihilation
 * NOTE: Seems like it fits 2p
 * NOTE: Ships should be <N> times more than opponents
 * NOTE: Halite should be <N> times more than opponents
 * PROPOSED: 1.32 and 1.68 respectively
 */
int main(int argc, char* argv[]) {
    Game game;
    game.ready("Lingjian Alfa");

    ostringstream init_debug;
    init_debug
        << "Player id is " << game.my_id << '\n'
        << "Projected turn limit is " << game.turn_limit;
    log::log(init_debug.str());

    for (;;) {
        auto t0 = steady_clock::now();
        game.update_frame();
        shared_ptr<Player> me = game.me;
        unique_ptr<GameMap>& game_map = game.game_map;

        vector<Command> command_queue;
        vector<shared_ptr<Ship>> sortedShips;

        for (const auto& ship_iterator : me->ships) sortedShips.push_back(ship_iterator.second);
        sort(sortedShips.begin(), sortedShips.end(), [&](const auto& a, const auto& b) {
            return game_map->calculate_distance(a->position, me->shipyard->position) < game_map->calculate_distance(b->position, me->shipyard->position);
        });

        for (const auto& ship : sortedShips) {
            ostringstream action_debug;
            MapCell *cell = game_map->at(ship);

            vector<Position> path_to_shipyard = game_map->calculate_path(ship->position, me->shipyard->position);
            array<Position, 4> borders = cell->position.get_surrounding_cardinals_in_range(2);

            sort(borders.begin(), borders.end(), [&](Position& a, Position& b) {
                return game_map->average_priority_in_range(a, 2) > game_map->average_priority_in_range(b, 2);
            });

            ship->suicidal = game.turn_number >= game.turn_limit - (int) path_to_shipyard.size();
            if (game_map->at(ship)->has_structure() || ship->halite == 0) ship->reset_status();

            if (ship->is_full() ||
                ship->is_suicidal() ||
                ship->is_returning() ||
                ship->halite >= constants::MAX_HALITE * 0.925 ||
                (ship->halite >= constants::MAX_HALITE * 0.65 && path_to_shipyard.size() <= 5))
            {
                Position targetPosition = ship->is_suicidal() ? path_to_shipyard[0] : me->shipyard->position;
                Direction direction = ship->is_suicidal() ? game_map->get_unsafe_moves(ship->position, targetPosition)[0] : game_map->naive_navigate(ship, targetPosition);
                Direction border_dir = game_map->naive_navigate(ship, borders[rand() % borders.size()]);
                Direction chosen_dir = direction != Direction::STILL ? direction : border_dir;
                Position end_node = game_map->at(ship)->position.directional_offset(chosen_dir);

                ship->returning = true;
                action_debug << "MOV " << ship << " " << end_node;
                command_queue.push_back(ship->move(chosen_dir));
            }
            else if (ship->halite < cell->halite * 0.1 ||
                     cell->halite > constants::MAX_HALITE / 10 + ((int) (15 * (game.turn_number / game.turn_limit))) ||
                     cell->priority * 1.32 > game_map->at(borders.at(0))->priority)
            {
                action_debug << "COL " << ship << " " << cell->position << " " << cell->halite * 0.1;
                command_queue.push_back(ship->stay_still());
            }
            else
            {
                action_debug << "MOV " << ship << " " << borders[0];
                command_queue.push_back(ship->move(game_map->naive_navigate(ship, borders[0])));
            }
            log::log(action_debug.str());
        }

        if ((me->ships.size() < 5 || me->ships.size() < game.average_enemy_ships() * (1 + pow(2, game.players.size() * 0.1))) &&
            game.turn_number < game.turn_limit * 0.5 &&
            me->halite >= constants::SHIP_COST &&
            (!game_map->at(me->shipyard)->is_occupied() || game_map->at(me->shipyard)->ship->owner != me->id))
        {
            ostringstream spawn_debug;
            spawn_debug << "GEN S" << game_map->total_ships() + 1;

            log::log(spawn_debug.str());
            command_queue.push_back(me->shipyard->spawn());
        }

        if (!game.end_turn(command_queue)) {
            break;
        }

        auto t1 = steady_clock::now();
        auto delta = duration_cast<microseconds>(t1 - t0).count();
        log::log("Turn took " + to_string(delta) + "us");
    }

    return 0;
}
