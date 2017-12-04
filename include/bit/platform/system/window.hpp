/**
 * \file window.hpp
 *
 * \brief todo: fill in documentation
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */
#ifndef BIT_PLATFORM_SYSTEM_WINDOW_HPP
#define BIT_PLATFORM_SYSTEM_WINDOW_HPP

// bit::platform
#include "../threading/concurrent_queue.hpp"
#include "event.hpp"

// bit::stl
#include <bit/stl/string_view.hpp>
#include <bit/stl/array_view.hpp>

// TODO: Generate a header that contains the native handle type through the
//       build system

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

    struct window_location
    {
      std::size_t x;
      std::size_t y;
    };

    struct window_size
    {
      std::size_t width;
      std::size_t height;
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

      explicit window( stl::zstring_view title );

      ~window();

      //----------------------------------------------------------------------
      // Modifiers
      //----------------------------------------------------------------------
    public:

      void set_title( stl::zstring_view title );

      void set_style( window_style style );

      void set_position( std::ptrdiff_t x, std::ptrdiff_t y );

      void set_size( std::size_t width, std::size_t height );

      void set_icon( std::size_t width, std::size_t height,
                     stl::array_view<stl::byte> rgba );

      //----------------------------------------------------------------------
      // Events
      //----------------------------------------------------------------------
    public:

      /// \brief Pushes an \p event into the event queue
      ///
      /// \param event the event to insert
      void push_event( const event& event );

      /// \brief Emplaces an \p event into the event queue
      ///
      /// \param args the arguments to use to construct the event
      template<typename...Args, std::enable_if<std::is_constructible<event,Args...>::value>* = nullptr>
      void emplace_event( Args&&...args );

      /// \brief Polls for an event, returning whether one was received
      ///
      /// \param event the event to assign
      /// \return \c true if an event was received
      bool poll_event( event* event );

      /// \brief Waits for a new event
      ///
      /// \return the event
      event wait_event();

      //----------------------------------------------------------------------
      // Native Handle
      //----------------------------------------------------------------------
    public:

      /// \brief Returns an instance of the native handle used for this window
      ///
      /// \return an instance of the native handle
      native_handle_type native_handle() noexcept;

      //----------------------------------------------------------------------
      // Private Member Types
      //----------------------------------------------------------------------
    private:

      class impl;

      //----------------------------------------------------------------------
      // Private Members
      //----------------------------------------------------------------------
    private:

//      std::unique_ptr<Impl> m_impl;
      native_handle_type    m_handle;

    };

  } // namespace platform
} // namespace bit

#endif /* BIT_PLATFORM_SYSTEM_WINDOW_HPP */
