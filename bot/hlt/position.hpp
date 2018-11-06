#pragma once

#include "types.hpp"
#include "direction.hpp"

#include <vector>
#include <iostream>

namespace hlt {
    struct Position {
        int x;
        int y;

        Position(int x, int y) : x(x), y(y) {}

        bool operator==(const Position& other) const { return x == other.x && y == other.y; }
        bool operator!=(const Position& other) const { return x != other.x || y != other.y; }

        Position directional_offset_in_range(Direction d, int range) const {
            auto dx = 0;
            auto dy = 0;
            switch (d) {
                case Direction::NORTH:
                    dy = -1;
                    break;
                case Direction::SOUTH:
                    dy = 1;
                    break;
                case Direction::EAST:
                    dx = 1;
                    break;
                case Direction::WEST:
                    dx = -1;
                    break;
                case Direction::STILL:
                    // No move
                    break;
                default:
                    log::log(std::string("Error: invert_direction: unknown direction ") + static_cast<char>(d));
                    exit(1);
            }
            return Position{x + dx * range, y + dy * range};
        }

        Position directional_offset(Direction d) const {
            return directional_offset_in_range(d, 1);
        }

        std::array<Position, 4> get_surrounding_cardinals_in_range(int range) {
            return {{
                directional_offset_in_range(Direction::NORTH, range), directional_offset_in_range(Direction::SOUTH, range),
                directional_offset_in_range(Direction::EAST, range), directional_offset_in_range(Direction::WEST, range)
            }};
        }

        std::array<Position, 4> get_surrounding_cardinals() {
            return get_surrounding_cardinals_in_range(1);
        }

        Direction direction_to(Position& destination) {
            int dx = destination.x - x;
            int dy = destination.y - y;

            if (dx == 0 && dy > 0) return Direction::SOUTH;
            else if (dx == 0 && dy < 0) return Direction::NORTH;
            else if (dx > 0 && dy == 0) return Direction::EAST;
            else if (dx < 0 && dy == 0) return Direction::WEST;
            return Direction::STILL;
        }

        std::vector<Position> get_surroundings_in_range(int range) {
            std::vector<Position> surroundings;

            for (int i = -range; i <= range; ++i)
                surroundings.push_back(Position{x + i, y});

            for (int row = 1; row <= range; ++row) {
                for (int col = -range + row; col <= range - row; ++col) {
                    surroundings.push_back(Position{x + col, y + row});
                    surroundings.push_back(Position{x + col, y - row});
                }
            }
            return surroundings;
        }
    };

    static std::ostream& operator<<(std::ostream& out, const Position& position) {
        out << '[' << position.x << ',' << position.y << ']';
        return out;
    }
    static std::istream& operator>>(std::istream& in, Position& position) {
        in >> position.x >> position.y;
        return in;
    }
}

namespace std {
    template <>
    struct hash<hlt::Position> {
        std::size_t operator()(const hlt::Position& position) const {
            return ((position.x+position.y) * (position.x+position.y+1) / 2) + position.y;
        }
    };
}
