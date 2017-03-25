#include <bit/platform/system/keyboard.hpp>

#ifdef _WIN32_WINDOWS
# undef _WIN32_WINDOWS
#endif
#ifdef _WIN32_WINNT
# undef _WIN32_WINNT
#endif
#ifndef NOMINMAX
# define NOMINMAX 1
#endif
#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN 1
#endif
#define _WIN32_WINDOWS 0x0501
#define _WIN32_WINNT   0x0501
#include <windows.h>

bool bit::platform::keyboard::is_key_pressed( key k )
  noexcept
{
  int vkey;

  switch(k)
  {
  // function keys
  case key::f1:  vkey = VK_F1;  break;
  case key::f2:  vkey = VK_F2;  break;
  case key::f3:  vkey = VK_F3;  break;
  case key::f4:  vkey = VK_F4;  break;
  case key::f5:  vkey = VK_F5;  break;
  case key::f6:  vkey = VK_F6;  break;
  case key::f7:  vkey = VK_F7;  break;
  case key::f8:  vkey = VK_F8;  break;
  case key::f9:  vkey = VK_F9;  break;
  case key::f10: vkey = VK_F10; break;
  case key::f11: vkey = VK_F11; break;
  case key::f12: vkey = VK_F12; break;

  // letters
  case key::a: vkey = 'A'; break;
  case key::b: vkey = 'B'; break;
  case key::c: vkey = 'C'; break;
  case key::d: vkey = 'D'; break;
  case key::e: vkey = 'E'; break;
  case key::f: vkey = 'F'; break;
  case key::g: vkey = 'G'; break;
  case key::h: vkey = 'H'; break;
  case key::i: vkey = 'I'; break;
  case key::j: vkey = 'J'; break;
  case key::k: vkey = 'K'; break;
  case key::l: vkey = 'L'; break;
  case key::m: vkey = 'M'; break;
  case key::n: vkey = 'N'; break;
  case key::o: vkey = 'O'; break;
  case key::p: vkey = 'P'; break;
  case key::q: vkey = 'Q'; break;
  case key::r: vkey = 'R'; break;
  case key::s: vkey = 'S'; break;
  case key::t: vkey = 'T'; break;
  case key::u: vkey = 'U'; break;
  case key::v: vkey = 'V'; break;
  case key::w: vkey = 'W'; break;
  case key::x: vkey = 'X'; break;
  case key::y: vkey = 'Y'; break;
  case key::z: vkey = 'Z'; break;

  // number keys
  case key::number_0: vkey = '0'; break;
  case key::number_1: vkey = '1'; break;
  case key::number_2: vkey = '2'; break;
  case key::number_3: vkey = '3'; break;
  case key::number_4: vkey = '4'; break;
  case key::number_5: vkey = '5'; break;
  case key::number_6: vkey = '6'; break;
  case key::number_7: vkey = '7'; break;
  case key::number_8: vkey = '8'; break;
  case key::number_9: vkey = '9'; break;

  // Control Characters
  case key::esc:      vkey = VK_ESCAPE; break;
  case key::l_ctrl:   vkey = VK_LCONTROL; break;
  case key::l_shift:  vkey = VK_LSHIFT; break;
  case key::l_alt:    vkey = VK_LMENU; break;
  case key::l_system: vkey = VK_LWIN; break;
  case key::r_ctrl:   vkey = VK_RCONTROL; break;
  case key::r_shift:  vkey = VK_RSHIFT; break;
  case key::r_alt:    vkey = VK_RMENU; break;
  case key::r_system: vkey = VK_RWIN; break;

  // Directions
  case key::left:  vkey = VK_LEFT; break;
  case key::right: vkey = VK_RIGHT; break;
  case key::up:    vkey = VK_UP; break;
  case key::down:  vkey = VK_DOWN; break;

  default: vkey = 0;
  }

  return (::GetAsyncKeyState(vkey) & 0x8000) != 0;
}
