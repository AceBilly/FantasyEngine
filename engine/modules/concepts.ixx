module;
#include <concepts>
#include <Windows.h>

export module Render.Concepts;
import Common;

export
namespace render::concepts {
    template<typename T>
    concept CoordsType = std::integral<T> || std::floating_point<T>;

    template<typename T>
    concept Pointer = requires(T obj) {
        obj == nullptr;
    };
// todo:
    template<typename T>
    concept Releaseable = requires {
      {&T::Release};
    };
}  // render::concepts

