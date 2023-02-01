module;
#include <concepts>
#include <Windows.h>
export module Render.Concepts;
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

        typename decltype(&T::Release);  // clang and gcc not compile

//        std::same_as<typename decltype(&T::Release), ULONG(T* )>; // msvc not compile but clangd intelligence is successed
    };
}  // render::concepts

