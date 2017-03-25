#include <bit/platform/system/event.hpp>

#include <bit/stl/assert.hpp>

//============================================================================
// window_event
//============================================================================

//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------

bit::platform::window_event::window_event( event_type type )
  noexcept
  : m_storage()
  , m_type(type)
{

}

bit::platform::window_event::window_event( const move_data& event )
  noexcept
  : m_storage(event)
  , m_type(event_type::moved)
{

}

bit::platform::window_event::window_event( const resize_data& event )
  noexcept
  : m_storage(event)
  , m_type(event_type::resized)
{

}

bit::platform::window_event::window_event( const window_event& other )
  noexcept
  : m_storage()
  , m_type(other.type())
{
  switch( m_type ) {
  case event_type::moved:
    new (&m_storage) storage_type(other.move());
    break;
  case event_type::resized:
    new (&m_storage) storage_type(other.resize());
    break;
  default:
    break;
  }
}

//----------------------------------------------------------------------------
// Element Access
//----------------------------------------------------------------------------

const bit::platform::window_event::resize_data&
  bit::platform::window_event::resize()
  const
{
  BIT_ASSERT_OR_THROW(bad_event_cast,m_type==event_type::resized, BIT_EMPTY);

  return m_storage.resize;
}

const bit::platform::window_event::move_data&
  bit::platform::window_event::move()
  const
{
  BIT_ASSERT_OR_THROW(bad_event_cast,m_type==event_type::moved, BIT_EMPTY);

  return m_storage.move;
}

//============================================================================
// keyboard_event
//============================================================================

//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------

bit::platform::keyboard_event::keyboard_event( const keydown_data& data )
  noexcept
  : m_storage(data)
  , m_type(event_type::keydown)
{

}

bit::platform::keyboard_event::keyboard_event( const keyup_data& data )
  noexcept
  : m_storage(data)
  , m_type(event_type::keyup)
{

}

bit::platform::keyboard_event::keyboard_event( const keyboard_event& other )
  noexcept
  : m_storage()
  , m_type(other.type())
{
  switch(m_type) {
  case event_type::keydown:
    new (&m_storage.keydown) storage_type(other.keydown());
    break;
  case event_type::keyup:
    new (&m_storage.keyup) storage_type(other.keyup());
    break;
  }
}

//----------------------------------------------------------------------------
// Element Access
//----------------------------------------------------------------------------

const bit::platform::keyboard_event::keydown_data&
  bit::platform::keyboard_event::keydown()
  const
{
  BIT_ASSERT_OR_THROW(bad_event_cast,m_type==event_type::keydown, BIT_EMPTY);

  return m_storage.keydown;
}

const bit::platform::keyboard_event::keyup_data&
  bit::platform::keyboard_event::keyup()
  const
{
  BIT_ASSERT_OR_THROW(bad_event_cast,m_type==event_type::keyup, BIT_EMPTY);

  return m_storage.keyup;
}

//============================================================================
// controller_event
//============================================================================

//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------

bit::platform::controller_event::controller_event( controller::button_state state,
                                                   const button_data& data )
  noexcept
  : m_storage(data)
  , m_type(state == controller::button_state::pressed ?
           event_type::button_press : event_type::button_release)
{

}

bit::platform::controller_event::controller_event( const joystick_data& data )
  noexcept
  : m_storage(data)
  , m_type(event_type::joystick)
{

}

bit::platform::controller_event::controller_event( const trigger_data& data )
  noexcept
  : m_storage(data)
  , m_type(event_type::trigger)
{

}

bit::platform::controller_event::controller_event( const controller_event& other )
  noexcept
  : m_storage()
  , m_type(other.type())
{
  switch(m_type) {
  case event_type::button_press:
    // [[fallthrough]];
  case event_type::button_release:
    new (&m_storage.button) storage_type(other.button());
    break;
  case event_type::joystick:
    new (&m_storage.joystick) storage_type(other.joystick());
    break;
  case event_type::trigger:
    new (&m_storage.trigger) storage_type(other.trigger());
    break;
  default:
    break;
  }
}

//----------------------------------------------------------------------------
// Element Access
//----------------------------------------------------------------------------

const bit::platform::controller_event::joystick_data&
  bit::platform::controller_event::joystick()
  const
{
  BIT_ASSERT_OR_THROW(bad_event_cast,m_type==event_type::joystick, BIT_EMPTY);

  return m_storage.joystick;
}

const bit::platform::controller_event::trigger_data&
  bit::platform::controller_event::trigger()
  const
{
  BIT_ASSERT_OR_THROW(bad_event_cast,m_type==event_type::trigger, BIT_EMPTY);

  return m_storage.trigger;
}

const bit::platform::controller_event::button_data&
  bit::platform::controller_event::button()
  const
{
  BIT_ASSERT_OR_THROW(bad_event_cast,m_type==event_type::button_press || m_type==event_type::button_release, BIT_EMPTY);

  return m_storage.button;
}

//============================================================================
// event
//============================================================================

//----------------------------------------------------------------------------
// Element Access
//----------------------------------------------------------------------------

bit::platform::event::event( const controller_event& event )
  noexcept
  : m_storage(event)
  , m_category(event_category::controller)
{

}

bit::platform::event::event( const keyboard_event& event )
  noexcept
  : m_storage(event)
  , m_category(event_category::keyboard)
{

}

bit::platform::event::event( const window_event& event )
  noexcept
  : m_storage(event)
  , m_category(event_category::window)
{

}

bit::platform::event::event( const event& other )
  noexcept
  : m_storage()
  , m_category(other.category())
{
  switch(m_category) {
  case event_category::controller:
    new (&m_storage.controller) storage_type(other.controller());
    break;
  case event_category::keyboard:
    new (&m_storage.keyboard) storage_type(other.keyboard());
    break;
  case event_category::window:
    new (&m_storage.window) storage_type(other.window());
    break;
  }
}

//----------------------------------------------------------------------------
// Element Access
//----------------------------------------------------------------------------

const bit::platform::controller_event& bit::platform::event::controller()
  const
{
  BIT_ASSERT_OR_THROW(bad_event_cast,m_category==event_category::controller, BIT_EMPTY);

  return m_storage.controller;
}

const bit::platform::keyboard_event& bit::platform::event::keyboard()
  const
{
  BIT_ASSERT_OR_THROW(bad_event_cast,m_category==event_category::keyboard, BIT_EMPTY);

  return m_storage.keyboard;
}

const bit::platform::window_event& bit::platform::event::window()
  const
{
  BIT_ASSERT_OR_THROW(bad_event_cast,m_category==event_category::window, BIT_EMPTY);

  return m_storage.window;
}
