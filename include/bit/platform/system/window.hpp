/**
 * \file window.hpp
 *
 * \brief todo: fill in documentation
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */
#ifndef BIT_PLATFORM_SYSTEM_WINDOW_HPP
#define BIT_PLATFORM_SYSTEM_WINDOW_HPP

#ifdef _WIN32
struct HWND__;
#endif

namespace bit {
  namespace platform {

    enum class window_style
    {
      none,       ///< style with no border nor title bar
      fullscreen, ///< fullscreen with no borderr or title bar
      resizeable, ///< resizeable window with a close button and title bar
    };

    //////////////////////////////////////////////////////////////////////////
    ///
    ///
    //////////////////////////////////////////////////////////////////////////
    class window
    {
      //----------------------------------------------------------------------
      // Public Member Types
      //----------------------------------------------------------------------
    public:

#if defined(__MACH__)
      using native_handle_type = unsigned long;
#elif defined(__unix__)
      using native_handle_type = void*;
#elif defined(_WIN32)
      using native_handle_type = ::HWND__*;
#endif
      using style = window_style;

      //----------------------------------------------------------------------
      // Constructor / Destructor
      //----------------------------------------------------------------------
    public:

      explicit window( stl::string_view title );

      ~window();

      //----------------------------------------------------------------------
      // Modifiers
      //----------------------------------------------------------------------
    public:

      void set_title( stl::string_view title );

      void set_style( window_style style );

      void set_();

      //----------------------------------------------------------------------
      // Events
      //----------------------------------------------------------------------
    public:

      /// \brief
      ///
      /// \param event
      /// \return
      bool poll_event( event* event );

      /// \brief
      ///
      /// \param event
      /// \return
      bool wait_event( event* event );

      //----------------------------------------------------------------------
      // Native Handle
      //----------------------------------------------------------------------
    public:

      /// \brief Returns an instance of the native handle used for this window
      ///
      /// \return an instance of the native handle
      native_handle_type native_handle() noexcept;

    };

  } // namespace platform
} // namespace bit

#endif /* BIT_PLATFORM_SYSTEM_WINDOW_HPP */
