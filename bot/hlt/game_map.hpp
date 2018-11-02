#pragma once

#include "types.hpp"
#include "map_cell.hpp"

#include <queue>
#include <vector>
#include <algorithm>
#include <unordered_map>

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
                return at(a.second) > at(b.second);
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

                for (Position next : current.get_surrounding_cardinals()) {
                    next = normalize(next);
                    double new_cost = cost_so_far[current] + at(next)->halite * 0.1;

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
