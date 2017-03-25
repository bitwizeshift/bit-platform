/**
 * \file keyboard.hpp
 *
 * \brief This header contains a collection of keyboard scan-codes, along with
 *        a simple query API for determining if a key is pressed
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */
#ifndef BIT_PLATFORM_INPUT_KEYBOARD_HPP
#define BIT_PLATFORM_INPUT_KEYBOARD_HPP

#include <bit/stl/enum.hpp>

namespace bit {
  namespace platform {

    //////////////////////////////////////////////////////////////////////////
    /// \brief This enum holds all possible keyboard scancodes that can be
    ///        queried by the keyboard scanner
    //////////////////////////////////////////////////////////////////////////
    enum class key
    {
      // Function Keys
      f1,  ///< the F1 key
      f2,  ///< the F2 key
      f3,  ///< the F3 key
      f4,  ///< the F4 key
      f5,  ///< the F5 key
      f6,  ///< the F6 key
      f7,  ///< the F7 key
      f8,  ///< the F8 key
      f9,  ///< the F9 key
      f10, ///< the F10 key
      f11, ///< the F11 key
      f12, ///< the F12 key

      // Letters
      a, ///< the A key
      b, ///< the B key
      c, ///< the C key
      d, ///< the D key
      e, ///< the E key
      f, ///< the F key
      g, ///< the G key
      h, ///< the H key
      i, ///< the I key
      j, ///< the J key
      k, ///< the K key
      l, ///< the L key
      m, ///< the M key
      n, ///< the N key
      o, ///< the O key
      p, ///< the P key
      q, ///< the Q key
      r, ///< the R key
      s, ///< the S key
      t, ///< the T key
      u, ///< the U key
      v, ///< the V key
      w, ///< the W key
      x, ///< the X key
      y, ///< the Y key
      z, ///< the Z key

      // Number keys
      number_0, ///< the 0 key from the number row
      number_1, ///< the 1 key from the number row
      number_2, ///< the 2 key from the number row
      number_3, ///< the 3 key from the number row
      number_4, ///< the 4 key from the number row
      number_5, ///< the 5 key from the number row
      number_6, ///< the 6 key from the number row
      number_7, ///< the 7 key from the number row
      number_8, ///< the 8 key from the number row
      number_9, ///< the 9 key from the number row

      // Control Characters
      esc,       ///< the esc key
      l_ctrl,    ///< the left ctrl key
      l_shift,   ///< the left shift key
      l_alt,     ///< the left alt key
      l_system,  ///< the left OS key (windows, mac clover, etc)
      r_ctrl,    ///< the right ctrl key
      r_shift,   ///< the right shift key
      r_alt,     ///< the right alt key
      r_system,  ///< the right OS key (windows, mac clover, etc)

      //
      context,   ///< the menu key
      enter,     ///< the return key
      backspace, ///< the backspace key
      tab,       ///< the tab key
      page_up,   ///< the page-up key
      page_down, ///< the page-down key
      end,       ///< the end key
      home,      ///< the home key
      ins,       ///< the ins key
      del,       ///< the del key
      pause,     ///< the pause key

      // Symbol keys
      l_bracket,    ///< the [ key
      r_bracket,    ///< the ] key
      semicolon,    ///< the ; key
      comma,        ///< the , key
      period,       ///< the . key
      single_quote, ///< the ' key
      slash,        ///< the / key
      backslash,    ///< the \ key
      tilde,        ///< the ~ key
      equal,        ///< the = key
      dash,         ///< the - key
      space,        ///< the space key

      // Directions
      left,  ///< left arrow
      right, ///< right arrow
      up,    ///< up arrow
      down,  ///< down arrow

      // numpad
      numpad_add,      ///< the + key
      numpad_minus,    ///< the - key
      numpad_multiply, ///< the * key
      numpad_divide,   ///< the / key
      numpad_0,        ///< the numpad 0 key
      numpad_1,        ///< the numpad 1 key
      numpad_2,        ///< the numpad 2 key
      numpad_3,        ///< the numpad 3 key
      numpad_4,        ///< the numpad 4 key
      numpad_5,        ///< the numpad 5 key
      numpad_6,        ///< the numpad 6 key
      numpad_7,        ///< the numpad 7 key
      numpad_8,        ///< the numpad 8 key
      numpad_9,        ///< the numpad 9 key
      unknown = -1,
    };

    //////////////////////////////////////////////////////////////////////////
    /// \brief This type is used for monitoring keyboard input.
    ///
    /// Since most operating systems don't support binding more than one
    /// keyboard, this is modeled as a stateless class.
    //////////////////////////////////////////////////////////////////////////
    class keyboard
    {
      keyboard() = delete;
      keyboard(keyboard&&) = delete;
      keyboard(const keyboard&) = delete;
      ~keyboard() = delete;

      //----------------------------------------------------------------------
      // Public Static Members
      //----------------------------------------------------------------------
    public:

      /// \brief Checks whether the specified key is currently pressed.
      ///
      /// \return k the key being pressed
      /// \return \c true if the key is pressed
      static bool is_key_pressed( key k ) noexcept;
    };

  } // namespace platform
} // namespace bit

//----------------------------------------------------------------------------
// enum_traits : specialization
//----------------------------------------------------------------------------

template<>
const bit::platform::key* bit::stl::enum_traits<bit::platform::key>::begin();

template<>
const bit::platform::key* bit::stl::enum_traits<bit::platform::key>::end();

template<>
bit::stl::string_view
  bit::stl::enum_traits<bit::platform::key>::to_string( bit::platform::key k );

template<>
bit::platform::key
  bit::stl::enum_traits<bit::platform::key>::from_string( bit::stl::string_view s );


extern template struct bit::stl::enum_traits<bit::platform::key>;

#endif /* BIT_PLATFORM_INPUT_KEYBOARD_HPP */
