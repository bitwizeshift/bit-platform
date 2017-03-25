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

} // namespace

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
} // anonymous namespace
