#include <bit/platform/input/keyboard.hpp>

namespace {
  bit::platform::key g_keys[] = {
      // Function Keys
      bit::platform::key::f1,
      bit::platform::key::f2,
      bit::platform::key::f3,
      bit::platform::key::f4,
      bit::platform::key::f5,
      bit::platform::key::f6,
      bit::platform::key::f7,
      bit::platform::key::f8,
      bit::platform::key::f9,
      bit::platform::key::f10,
      bit::platform::key::f11,
      bit::platform::key::f12,

      // Letters
      bit::platform::key::a,
      bit::platform::key::b,
      bit::platform::key::c,
      bit::platform::key::d,
      bit::platform::key::e,
      bit::platform::key::f,
      bit::platform::key::g,
      bit::platform::key::h,
      bit::platform::key::i,
      bit::platform::key::j,
      bit::platform::key::k,
      bit::platform::key::l,
      bit::platform::key::m,
      bit::platform::key::n,
      bit::platform::key::o,
      bit::platform::key::p,
      bit::platform::key::q,
      bit::platform::key::r,
      bit::platform::key::s,
      bit::platform::key::t,
      bit::platform::key::u,
      bit::platform::key::v,
      bit::platform::key::w,
      bit::platform::key::x,
      bit::platform::key::y,
      bit::platform::key::z,

      // Number keys
      bit::platform::key::number_0,
      bit::platform::key::number_1,
      bit::platform::key::number_2,
      bit::platform::key::number_3,
      bit::platform::key::number_4,
      bit::platform::key::number_5,
      bit::platform::key::number_6,
      bit::platform::key::number_7,
      bit::platform::key::number_8,
      bit::platform::key::number_9,

      // Control Characters
      bit::platform::key::esc,
      bit::platform::key::l_ctrl,
      bit::platform::key::l_shift,
      bit::platform::key::l_alt,
      bit::platform::key::l_system,
      bit::platform::key::r_ctrl,
      bit::platform::key::r_shift,
      bit::platform::key::r_alt,
      bit::platform::key::r_system,

      //
      bit::platform::key::context,
      bit::platform::key::enter,
      bit::platform::key::backspace,
      bit::platform::key::tab,
      bit::platform::key::page_up,
      bit::platform::key::page_down,
      bit::platform::key::end,
      bit::platform::key::home,
      bit::platform::key::ins,
      bit::platform::key::del,
      bit::platform::key::pause,

      // Symbols
      bit::platform::key::l_bracket,
      bit::platform::key::r_bracket,
      bit::platform::key::semicolon,
      bit::platform::key::comma,
      bit::platform::key::period,
      bit::platform::key::single_quote,
      bit::platform::key::slash,
      bit::platform::key::backslash,
      bit::platform::key::tilde,
      bit::platform::key::equal,
      bit::platform::key::dash,
      bit::platform::key::space,

      // Directions
      bit::platform::key::left,
      bit::platform::key::right,
      bit::platform::key::up,
      bit::platform::key::down,

      // numpad
      bit::platform::key::numpad_add,
      bit::platform::key::numpad_minus,
      bit::platform::key::numpad_multiply,
      bit::platform::key::numpad_divide,
      bit::platform::key::numpad_0,
      bit::platform::key::numpad_1,
      bit::platform::key::numpad_2,
      bit::platform::key::numpad_3,
      bit::platform::key::numpad_4,
      bit::platform::key::numpad_5,
      bit::platform::key::numpad_6,
      bit::platform::key::numpad_7,
      bit::platform::key::numpad_8,
      bit::platform::key::numpad_9,
    };
}

//----------------------------------------------------------------------------
// to_string
//----------------------------------------------------------------------------

template<>
bit::stl::string_view
  bit::stl::enum_traits<bit::platform::key>::to_string( bit::platform::key from )
{
  switch(from) {
  // function keys
  case bit::platform::key::f1: return "f1";
  case bit::platform::key::f2: return "f2";
  case bit::platform::key::f3: return "f3";
  case bit::platform::key::f4: return "f4";
  case bit::platform::key::f5: return "f5";
  case bit::platform::key::f6: return "f6";
  case bit::platform::key::f7: return "f7";
  case bit::platform::key::f8: return "f8";
  case bit::platform::key::f9: return "f9";
  case bit::platform::key::f10: return "f10";
  case bit::platform::key::f11: return "f11";
  case bit::platform::key::f12: return "f12";

  // letters
  case bit::platform::key::a: return "a";
  case bit::platform::key::b: return "b";
  case bit::platform::key::c: return "c";
  case bit::platform::key::d: return "d";
  case bit::platform::key::e: return "e";
  case bit::platform::key::f: return "f";
  case bit::platform::key::g: return "g";
  case bit::platform::key::h: return "h";
  case bit::platform::key::i: return "i";
  case bit::platform::key::j: return "j";
  case bit::platform::key::k: return "k";
  case bit::platform::key::l: return "l";
  case bit::platform::key::m: return "m";
  case bit::platform::key::n: return "n";
  case bit::platform::key::o: return "o";
  case bit::platform::key::p: return "p";
  case bit::platform::key::q: return "q";
  case bit::platform::key::r: return "r";
  case bit::platform::key::s: return "s";
  case bit::platform::key::t: return "t";
  case bit::platform::key::u: return "u";
  case bit::platform::key::v: return "v";
  case bit::platform::key::w: return "w";
  case bit::platform::key::x: return "x";
  case bit::platform::key::y: return "y";
  case bit::platform::key::z: return "z";

  // number row
  case bit::platform::key::number_0: return "0";
  case bit::platform::key::number_1: return "1";
  case bit::platform::key::number_2: return "2";
  case bit::platform::key::number_3: return "3";
  case bit::platform::key::number_4: return "4";
  case bit::platform::key::number_5: return "5";
  case bit::platform::key::number_6: return "6";
  case bit::platform::key::number_7: return "7";
  case bit::platform::key::number_8: return "8";
  case bit::platform::key::number_9: return "9";

  // Control Characters
  case bit::platform::key::esc:      return "esc";
  case bit::platform::key::l_ctrl:   return "l_ctrl";
  case bit::platform::key::l_shift:  return "l_shift";
  case bit::platform::key::l_alt:    return "l_alt";
  case bit::platform::key::l_system: return "l_system";
  case bit::platform::key::r_ctrl:   return "r_ctrl";
  case bit::platform::key::r_shift:  return "r_shift";
  case bit::platform::key::r_alt:    return "r_alt";
  case bit::platform::key::r_system: return "r_system";

  //
  case bit::platform::key::context:   return "context";
  case bit::platform::key::enter:     return "enter";
  case bit::platform::key::backspace: return "backspace";
  case bit::platform::key::tab:       return "tab";
  case bit::platform::key::page_up:   return "page_up";
  case bit::platform::key::page_down: return "page_down";
  case bit::platform::key::end:       return "end";
  case bit::platform::key::home:      return "home";
  case bit::platform::key::ins:       return "ins";
  case bit::platform::key::del:       return "del";
  case bit::platform::key::pause:     return "pause";

  // Directions
  case bit::platform::key::left:  return "left";
  case bit::platform::key::right: return "right";
  case bit::platform::key::up:    return "up";
  case bit::platform::key::down:  return "down";

  // numpad
  case bit::platform::key::numpad_add:      return "+";
  case bit::platform::key::numpad_minus:    return "-";
  case bit::platform::key::numpad_multiply: return "*";
  case bit::platform::key::numpad_divide:   return "/";
  case bit::platform::key::numpad_0:        return "numpad_0";
  case bit::platform::key::numpad_1:        return "numpad_1";
  case bit::platform::key::numpad_2:        return "numpad_2";
  case bit::platform::key::numpad_3:        return "numpad_3";
  case bit::platform::key::numpad_4:        return "numpad_4";
  case bit::platform::key::numpad_5:        return "numpad_5";
  case bit::platform::key::numpad_6:        return "numpad_6";
  case bit::platform::key::numpad_7:        return "numpad_7";
  case bit::platform::key::numpad_8:        return "numpad_8";
  case bit::platform::key::numpad_9:        return "numpad_9";

  default: break;
  }

  BIT_ASSERT_OR_THROW(bit::stl::bad_enum_cast,false,"");
  return ""; // to silence IDE warnings
}

//----------------------------------------------------------------------------
// from_string
//----------------------------------------------------------------------------

template<>
bit::platform::key
  bit::stl::enum_traits<bit::platform::key>::from_string( bit::stl::string_view from )
{
  // TODO: Implement string -> key conversion

  return bit::platform::key::unknown;
}

//----------------------------------------------------------------------------
// Iteration
//----------------------------------------------------------------------------

template<>
const bit::platform::key* bit::stl::enum_traits<bit::platform::key>::begin()
{
  return std::begin(g_keys);
}

//----------------------------------------------------------------------------

template<>
const bit::platform::key* bit::stl::enum_traits<bit::platform::key>::end()
{
  return std::end(g_keys);
}

template struct bit::stl::enum_traits<bit::platform::key>;
