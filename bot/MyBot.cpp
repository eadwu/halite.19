#include "hlt/game.hpp"
#include "hlt/constants.hpp"
#include "hlt/log.hpp"

#include <math.h>

#include <sstream>
#include <algorithm>

using namespace std;
using namespace hlt;

int main(int argc, char* argv[]) {
    Game game;
    game.ready("Lingjian");

    log::log("Lingjian has been loaded, player id is " + to_string(game.my_id));
    log::log("Projected turn limit is " + to_string(game.turn_limit));
    for (;;) {
        game.update_frame();
        shared_ptr<Player> me = game.me;
        unique_ptr<GameMap>& game_map = game.game_map;

        vector<Command> command_queue;

        for (const auto& ship_iterator : me->ships) {
            std::ostringstream output;
            shared_ptr<Ship> ship = ship_iterator.second;
            MapCell *cell = game_map->at(ship);
            std::array<Position, 4> borders = cell->position.get_surrounding_cardinals();

            sort(borders.begin(), borders.end(), [&](const auto a, const auto b) {
                return game_map->at(a)->priority > game_map->at(b)->priority;
            });

            if (game_map->at(ship)->has_structure() || ship->halite == 0) ship->returning = false;

            if (ship->is_full() ||
                ship->is_returning() ||
                ship->halite >= constants::MAX_HALITE * 0.925 ||
                (ship->halite >= constants::MAX_HALITE * 0.65 && game_map->calculate_distance(ship->position, me->shipyard->position) <= 3))
            {
                Direction naiveDir = game_map->naive_navigate(ship, me->shipyard->position);
                Direction borderDir = game_map->naive_navigate(ship, borders[0]);
                Direction chosenDir = naiveDir != Direction::STILL ? naiveDir : borderDir;
                Position newPos = game_map->at(ship)->position.directional_offset(chosenDir);

                ship->returning = true;

                output << "MOV " << ship << " " << newPos;
                command_queue.push_back(ship->move(chosenDir));
            }
            else if (ship->halite < cell->halite * 0.1 ||
                     cell->halite > constants::MAX_HALITE / 20)
            {
                output << "COL " << ship << " " << cell->position << " " << cell->halite * 0.1;
                command_queue.push_back(ship->stay_still());
            }
            else
            {
                output << "MOV " << ship << " " << borders[0];
                command_queue.push_back(ship->move(game_map->naive_navigate(ship, borders[0])));
            }
            log::log(output.str());
        }

        if (me->ships.size() < (game.total_ships() - me->ships.size()) / (game.players.size() - 1) * (1 + pow(2, game.players.size() * 0.1)) &&
            me->halite >= constants::SHIP_COST &&
            !game_map->at(me->shipyard)->is_occupied())
        {
            log::log("GEN S" + to_string(game.total_ships() + 1));
            command_queue.push_back(me->shipyard->spawn());
        }

        if (!game.end_turn(command_queue)) {
            break;
        }
    }

    return 0;
}
