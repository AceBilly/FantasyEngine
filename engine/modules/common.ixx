//
// Created by yhtse on 2023/2/1.
//
// todo: module Common and concepts should merge;
module;
#include <cstdint>
#include <utility>
#include <type_traits>
#include <concepts>
#include <ranges>
#include <filesystem>
export module Common;
namespace fs = std::filesystem;
export
namespace common {

template<typename T>
concept CoordsType = std::integral<T> || std::floating_point<T>;

template<typename T>
concept Pointer = requires(T obj) {
  obj == nullptr;
};
template<typename T>
concept Pointer_T = std::is_pointer_v<T>;
// todo:
template<typename T>
concept Releaseable =Pointer_T<T> && requires {
  typename std::remove_pointer_t<T>;
  {&std::remove_pointer_t<T>::Release};
};
template<typename T>
concept ReleaseableContainer = std::ranges::range<T> && Releaseable<typename T::value_type>;
template<typename T>
concept OneOrContainerReleasable = Releaseable<T> || ReleaseableContainer<T>;


struct RenderCoreCommonData {
  uint8_t resolution_width;
  uint8_t resolution_height;
  uint8_t buffer_count;
  uint8_t current_buffer_index;
  fs::path assets_directory;
};

// interface: Render

template<typename T>
concept RenderType = requires {
      {&T::OnInit};
      {&T::OnUpdate};
      {&T::OnDestory};
      {&T::OnRender};
      {std::declval<T>().m_data} -> std::same_as<RenderCoreCommonData&&>;
};


template<CoordsType Coord_t>
struct Point {
  Coord_t x;
  Coord_t y;
};


template<CoordsType Coord_t>
struct Rect {
  Point<Coord_t> rect_origin_coord;
  Coord_t height, width;
};

}  // end namespace common
