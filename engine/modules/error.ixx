module;
#include <Windows.h>
#include <source_location>
#include <iostream>
#include <string>
export module Render.Error;
import Common;

using namespace common;

export
namespace render::error {
    std::ostream& operator <<(std::ostream& os, const std::source_location& location);
    void Failed(const HRESULT& result,
                const std::source_location& location_info = std::source_location::current());
    template<Pointer T>
    void Failed(const T res_ptr, const std::source_location& location_info = std::source_location::current()) {
        if (res_ptr == nullptr) {
            Failed(FWP_E_NULL_POINTER, location_info);
        }
    }
    // todo: cross platform ,this function only use in Windows
    std::string GetLastErrorAsString();

}  //render::error

module : private;

std::string render::error::GetLastErrorAsString()
{
    //Get the error message ID, if any.
    DWORD errorMessageID = ::GetLastError();
    if(errorMessageID == 0) {
        return std::string{"No error information"}; //No error message has been recorded
    }

    LPSTR messageBuffer = nullptr;

    //Ask Win32 to give us the string version of that message ID.
    //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    //Copy the error message into a std::string.
    std::string message(messageBuffer, size);

    //Free the Win32's string's buffer.
    LocalFree(messageBuffer);

    return message;
}

std::ostream &render::error::operator <<(std::ostream &os, const std::source_location &location) {
    os << location.file_name() << "("
    << location.line() << ":"
    << location.column() << ") `"
    << location.function_name() << "`: ";
    return os;
}

// check result
// shutdown and print error location information if failed
void render::error::Failed(const HRESULT &result,
            const std::source_location &location_info) {
    if (FAILED(result)) {
        std::cout << "Failed in: " << location_info << '\n';
        std::terminate();
    }

}