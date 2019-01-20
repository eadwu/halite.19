#pragma once

#include "types.hpp"
#include "map_cell.hpp"

#include <queue>
#include <vector>
#include <numeric>
#include <algorithm>
#include <unordered_map>

#include <math.h>

namespace hlt {
    struct GameMap {
        int width;
        int height;
        std::vector<std::vector<MapCell>> cells;

        MapCell* at(const Position& position) {
            Position normalized = normalize(position);
            return &cells[normalized.y][normalized.x];
        }

        MapCell* at(const Entity& entity) {
            return at(entity.position);
        }

        MapCell* at(const Entity* entity) {
            return at(entity->position);
        }

        MapCell* at(const std::shared_ptr<Entity>& entity) {
            return at(entity->position);
        }

        int total_ships() {
            return std::accumulate(cells.begin(), cells.end(), 0, [](int total, const auto& row) {
                return total + std::accumulate(row.begin(), row.end(), 0, [](int subtotal, const auto& cell) {
                    return cell.is_occupied() ? subtotal + 1 : subtotal;
                });
            });
        }

        std::vector<std::shared_ptr<Ship>> ships_in_range(Position& pos, int range) {
            std::vector<std::shared_ptr<Ship>> ships;
            std::vector<Position> surroundings = pos.get_surroundings_in_range(range);

            for (const auto& p : surroundings)
                if (at(p)->is_occupied() && at(at(p)->ship) == at(p)) ships.push_back(at(p)->ship);
            return ships;
        }

        double average_priority_in_range(Position& pos, int range) {
            std::vector<Position> surroundings = pos.get_surroundings_in_range(range);

            return std::accumulate(surroundings.begin(), surroundings.end(), 0., [this](int total, const auto& p) {
                return total + at(normalize(p))->priority;
            }) / surroundings.size();
        }

        void exude_priority_in_range(Position& pos, int range, double start) {
            std::vector<Position> surroundings = pos.get_surroundings_in_range(range);

            for (const auto& p : surroundings) {
                if (p.x == pos.x && p.y == pos.y) continue;
                // log::log("Old Priority: " + std::to_string(at(p)->priority));
                // log::log("Modifier: " + std::to_string(start * std::pow(0.5, calculate_distance(pos, p))));
                at(p)->priority = at(p)->priority + (start * std::pow(0.5, calculate_distance(pos, p)));
                // log::log("New Priority: " + std::to_string(at(p)->priority));
            }
        }

        void exude_priority_in_range(Position& pos, int range) {
            exude_priority_in_range(pos, range, at(pos)->halite);
        }

        int calculate_distance(const Position& source, const Position& target) {
            const auto& normalized_source = normalize(source);
            const auto& normalized_target = normalize(target);

            const int dx = std::abs(normalized_source.x - normalized_target.x);
            const int dy = std::abs(normalized_source.y - normalized_target.y);

            const int toroidal_dx = std::min(dx, width - dx);
            const int toroidal_dy = std::min(dy, height - dy);

            return toroidal_dx + toroidal_dy;
        }

        Position normalize(const Position& position) {
            const int x = ((position.x % width) + width) % width;
            const int y = ((position.y % height) + height) % height;
            return { x, y };
        }

        std::vector<Direction> get_unsafe_moves(const Position& source, const Position& destination) {
            const auto& normalized_source = normalize(source);
            const auto& normalized_destination = normalize(destination);

            const int dx = std::abs(normalized_source.x - normalized_destination.x);
            const int dy = std::abs(normalized_source.y - normalized_destination.y);
            const int wrapped_dx = width - dx;
            const int wrapped_dy = height - dy;

            std::vector<Direction> possible_moves;

            if (normalized_source.x < normalized_destination.x) {
                possible_moves.push_back(dx > wrapped_dx ? Direction::WEST : Direction::EAST);
            } else if (normalized_source.x > normalized_destination.x) {
                possible_moves.push_back(dx < wrapped_dx ? Direction::WEST : Direction::EAST);
            }

            if (normalized_source.y < normalized_destination.y) {
                possible_moves.push_back(dy > wrapped_dy ? Direction::NORTH : Direction::SOUTH);
            } else if (normalized_source.y > normalized_destination.y) {
                possible_moves.push_back(dy < wrapped_dy ? Direction::NORTH : Direction::SOUTH);
            }

            return possible_moves;
        }

        std::vector<Position> calculate_path(const Position& source, const Position& destination) {
            std::vector<Position> path;

            auto gScore = [this](const std::pair<double, Position>& a, const std::pair<double, Position>& b) {
                return a.first > b.first;
            };
            std::priority_queue<std::pair<double, Position>, std::vector<std::pair<double, Position>>, decltype(gScore)> frontier(gScore);
            std::unordered_map<Position, MapCell*> came_from;
            std::unordered_map<Position, double> cost_so_far;

            frontier.emplace(0, source);
            cost_so_far[source] = 0;
            came_from[source] = at(source);

            while (!frontier.empty()) {
                Position current = frontier.top().second;
                frontier.pop();

                if (current == destination) {
                    while (current != source) {
                        path.push_back(current);
                        current = came_from[current]->position;
                    }
                    break;
                }

                for (Position p : current.get_surroundings_in_range(1)) {
                    Position next = normalize(p);
                    double new_cost = cost_so_far[current] + at(next)->priority * 0.1;

                    if (cost_so_far.find(next) == cost_so_far.end() || new_cost < cost_so_far[next]) {
                        cost_so_far[next] = new_cost;

                        double priority = new_cost + calculate_distance(next, destination);
                        frontier.emplace(priority, next);

                        came_from[next] = at(current);
                    }
                }
            }

            std::reverse(path.begin(), path.end());
            return path;
        }

        Direction naive_navigate(std::shared_ptr<Ship> ship, const Position& destination) {
            // get_unsafe_moves normalizes for us
            for (auto direction : get_unsafe_moves(ship->position, destination)) {
                Position target_pos = ship->position.directional_offset(direction);
                if (!at(target_pos)->is_occupied()) {
                    at(target_pos)->mark_unsafe(ship);
                    return direction;
                }
            }

            return Direction::STILL;
        }

        void _update();
        static std::unique_ptr<GameMap> _generate();
    };
}
