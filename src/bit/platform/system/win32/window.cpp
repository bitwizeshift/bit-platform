#include <bit/platform/system/window.hpp>

#ifdef _WIN32_WINDOWS
# undef _WIN32_WINDOWS
#endif
#ifdef _WIN32_WINNT
# undef _WIN32_WINNT
#endif
#define _WIN32_WINDOWS 0x0501
#define _WIN32_WINNT   0x0501
#define WINVER         0x0501
#include <windows.h>

namespace {

  /// \brief Sets process DPI awareness in Windows
  void set_process_dpi_aware() noexcept;

  bool create_gl_window( ::HDC* gdi, ::HGLRC* context, ::HWND* window,
                         ::HINSTANCE* instance );

  void destroy_gl_window( ::HDC* gdi, ::HGLRC* context, ::HWND* window,
                          ::HINSTANCE* instance );

//  ::LRESULT CALLBACK event_handler( ::HWND handle, ::UINT message,
//                                    ::WPARAM wparam, ::LPARAM lparam )
//  {
//    // Associate handle and Window instance when the creation message is received
//    if (message == WM_CREATE)
//    {
//        // Get WindowImplWin32 instance (it was passed as the last argument of CreateWindow)
//        ::LONG_PTR window = (::LONG_PTR)reinterpret_cast<::CREATESTRUCT*>(lparam)->lpCreateParams;
//
//        // Set as the "user data" parameter of the window
//        ::SetWindowLongPtrW(handle, GWLP_USERDATA, window);
//    }
//
//    // Get the WindowImpl instance corresponding to the window handle
//    WindowImplWin32* window = handle ? reinterpret_cast<WindowImplWin32*>(GetWindowLongPtr(handle, GWLP_USERDATA)) : NULL;
//
//    // Forward the event to the appropriate function
//    if (window)
//    {
//        window->processEvent(message, wParam, lParam);
//
//        if (window->m_callback)
//            return CallWindowProcW(reinterpret_cast<WNDPROC>(window->m_callback), handle, message, wParam, lParam);
//    }
//
//    // We don't forward the WM_CLOSE message to prevent the OS from automatically destroying the window
//    if (message == WM_CLOSE)
//        return 0;
//
//    // Don't forward the menu system command, so that pressing ALT or F10 doesn't steal the focus
//    if ((message == WM_SYSCOMMAND) && (wparam == SC_KEYMENU))
//        return 0;
//
//    return ::DefWindowProcW(handle, message, wparam, lparam);
//  }

} // namespace

class bit::platform::window::impl
{

};

bit::platform::window::window( stl::zstring_view title )
{
  ::MessageBoxA(NULL,"window::window: Failed to register window class","Error",MB_OK|MB_ICONEXCLAMATION);
}

bit::platform::window::~window() = default;

void bit::platform::window::set_title( stl::zstring_view title )
{
  ::SetWindowTextA(m_handle, title.data());
}

void bit::platform::window::set_style( style style )
{

}

void bit::platform::window::set_position( std::ptrdiff_t x, std::ptrdiff_t y )
{
  ::SetWindowPos(m_handle, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void bit::platform::window::set_size( std::size_t width, std::size_t height )
{
  auto rectangle = ::RECT{0, 0, static_cast<long>(width), static_cast<long>(height)};

  ::AdjustWindowRect(&rectangle, ::GetWindowLong(m_handle, GWL_STYLE), false);
  auto w  = rectangle.right - rectangle.left;
  auto h = rectangle.bottom - rectangle.top;

  ::SetWindowPos(m_handle, nullptr, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER);
}


namespace { // anonymous

  void set_process_dpi_aware()
    noexcept
  {
    // Try SetProcessDpiAwareness first
    ::HINSTANCE sh_core_dll = ::LoadLibraryW(L"Shcore.dll");

    if(sh_core_dll) {
      enum ProcessDpiAwareness
      {
        ProcessDpiUnaware         = 0,
        ProcessSystemDpiAware     = 1,
        ProcessPerMonitorDpiAware = 2
      };

      using set_process_fn_t = ::HRESULT(WINAPI*)(ProcessDpiAwareness);

      auto set_process_dpi_awareness_function =
        reinterpret_cast<set_process_fn_t>(::GetProcAddress(sh_core_dll, "SetProcessDpiAwareness"));

      if(set_process_dpi_awareness_function) {

        // We only check for E_INVALIDARG because we would get
        // E_ACCESSDENIED if the DPI was already set previously
        // and S_OK means the call was successful
        if (set_process_dpi_awareness_function(ProcessSystemDpiAware) == E_INVALIDARG) {

        } else {
          ::FreeLibrary(sh_core_dll);
          return;
        }
      }

      ::FreeLibrary(sh_core_dll);
    }

    // Fall back to SetProcessDPIAware if SetProcessDpiAwareness
    // is not available on this system
    ::HINSTANCE user32Dll = ::LoadLibraryW(L"user32.dll");

    if(user32Dll) {

      using SetProcessDPIAwareFuncType = ::BOOL(WINAPI*)(void);
      SetProcessDPIAwareFuncType SetProcessDPIAwareFunc = reinterpret_cast<SetProcessDPIAwareFuncType>(::GetProcAddress(user32Dll, "SetProcessDPIAware"));

      if (SetProcessDPIAwareFunc && !SetProcessDPIAwareFunc()) {
        // handle error
      }

      ::FreeLibrary(user32Dll);
    }
  }



  bool create_gl_window( ::HDC* gdi, ::HGLRC* context, ::HWND* window,
                         ::HINSTANCE* instance )
  {

  }

  void destroy_gl_window( ::HDC* gdi, ::HGLRC* context, ::HWND* window,
                          ::HINSTANCE* instance )
  {
    ::ChangeDisplaySettings(nullptr,0);
    ::ShowCursor(true);

    if( *context ){
      // Are We Able To Release The DC And RC Contexts?
      if(!::wglMakeCurrent(nullptr,nullptr)) {
        ::MessageBoxA(nullptr,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
      }

      // Are We Able To Delete The RC?
      if(!::wglDeleteContext(*context)) {
        ::MessageBoxA(nullptr,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
      }
      *context=nullptr;
    }

    // Are We Able To Release The DC
    if(*gdi && !::ReleaseDC(*window,*gdi)) {
      ::MessageBoxA(nullptr,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
      *gdi=nullptr;
    }

    // Are We Able To Destroy The Window?
    if(*window && !DestroyWindow(*window)) {
      ::MessageBoxA(nullptr,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
      *window=nullptr;
    }

    // Are We Able To Unregister Class
    if(!::UnregisterClass("OpenGL",*instance)) {
      // Set hInstance To NULL
      ::MessageBoxA(nullptr,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
      *instance=nullptr;
    }
  }
} // anonymous namespace
