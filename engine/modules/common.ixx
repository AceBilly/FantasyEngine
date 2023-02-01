//
// Created by yhtse on 2023/2/1.
//
module;
#include <cstdint>
export module common;
import Render.Concepts;
using namespace render::concepts;

export
template<CoordsType Coord_t>
struct Point {
    Coord_t x;
    Coord_t y;
};
export
template<CoordsType Coord_t>
struct Rect {
    Point<Coord_t> rect_origin_coord;
    Coord_t height, width;
};