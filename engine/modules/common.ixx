//
// Created by yhtse on 2023/2/1.
//
// todo: module Common and concepts should merge;
module;
#include <cstdint>
#include <utility>
#include <type_traits>
#include <concepts>
export module Common;
import Render.Concepts;
using namespace render::concepts;

export
    struct RenderCoreCommonData {
  uint8_t width;
  uint8_t height;
};

// interface: Render
export
template<typename T>
concept RenderType = requires {
  {&T::OnInit};
  {&T::OnUpdate};
  {&T::OnDestory};
  {&T::OnRender};
  {std::declval<T>().m_data} -> std::same_as<RenderCoreCommonData&&>;
};

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