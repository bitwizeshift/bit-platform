/**
 * \file event.hpp
 *
 * \brief todo: fill in documentation
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */

#ifndef BIT_PLATFORM_SYSTEM_EVENT_HPP
#define BIT_PLATFORM_SYSTEM_EVENT_HPP

#include "keyboard.hpp"
#include "controller.hpp"

namespace bit {
  namespace platform {

    //////////////////////////////////////////////////////////////////////////
    /// \brief An exception type used for handling bad event conversions
    ///
    /// This exception is only ever thrown when exceptions are enabled, and
    /// when the base event class tries to convert to an event of the
    /// incorrect type
    //////////////////////////////////////////////////////////////////////////
    class bad_event_cast : public std::bad_cast{};

    //------------------------------------------------------------------------
    // Window Events
    //------------------------------------------------------------------------

    //////////////////////////////////////////////////////////////////////////
    /// \class bit::platform::window_event
    ///
    /// \brief This type handles all events that come from window resizing,
    ///        modifications, and interactions
    ///
    /// Events with associated data can be polled via accessor functions.
    //////////////////////////////////////////////////////////////////////////
    class window_event
    {
      //----------------------------------------------------------------------
      // Public Member Types
      //----------------------------------------------------------------------
    public:

      /// \brief The underlying type of this event
      enum class event_type
      {
        shown,        ///< window shown
        hidden,       ///< window hidden
        exposed,      ///< exposed
        moved,        ///< moved
        resized,      ///< resized
        minimized,    ///< minimized
        maximized,    ///< maximized
        restored,     ///<
        focus_gained, ///<
        focus_lost,   ///<
        close,        ///<
        mouse_enter,  ///<
        mouse_leave,  ///<
      };

      /// \brief Data for window resize events
      struct resize_data
      {
        std::size_t width;
        std::size_t height;
      };

      /// \brief Data for window moving events
      struct move_data
      {
        std::ptrdiff_t x;
        std::ptrdiff_t y;
      };

      //----------------------------------------------------------------------
      // Constructors
      //----------------------------------------------------------------------
    public:

      /// \brief Constructs a window_event given the event type
      ///
      /// \param type the type of event
      explicit window_event( event_type type ) noexcept;

      /// \brief Constructs a window_event given resize_event data
      ///
      /// \param resize_event the data for the event
      explicit window_event( resize_data const& resize_event ) noexcept;

      /// \brief Constructs a window_event given move_event data
      ///
      /// \param move_event the data for the event
      explicit window_event( move_data const& move_event ) noexcept;

      /// \brief Copy-constructs a window_event
      ///
      /// \param other the other event to copy
      window_event( const window_event& other ) noexcept;

      //----------------------------------------------------------------------
      // Element Access
      //----------------------------------------------------------------------
    public:

      /// \brief Returns a reference to the resize data if this event is
      ///        a resize event
      ///
      /// \throws bad_event_cast if exceptions are enabled and if this event is
      ///         not a resize event; otherwise asserts
      /// \return a reference to the resize data
      const resize_data& resize() const;

      /// \brief Returns a reference to the move data if this event is
      ///        a move event
      ///
      /// \throws bad_event_cast if exceptions are enabled and if this event is
      ///         not a move event; otherwise asserts
      /// \return a reference to the move data
      const move_data& move() const;

      /// \brief Returns the type of the underlying event
      ///
      /// \return the event type
      event_type type() const noexcept;

      //----------------------------------------------------------------------
      // Private Member Types
      //----------------------------------------------------------------------
    private:

      struct empty{};

      union storage_type
      {
        empty       empty;
        resize_data resize;
        move_data   move;

        storage_type() : empty(){}
        storage_type( const resize_data& data ) : resize(data){}
        storage_type( const move_data& data ) : move(data){}
      };

      //----------------------------------------------------------------------
      // Private Members
      //----------------------------------------------------------------------
    private:

      storage_type m_storage; ///< The type used for storage
      event_type   m_type;    ///< The tpye of the event
    };


    //------------------------------------------------------------------------
    // event : keyboard
    //------------------------------------------------------------------------


    //////////////////////////////////////////////////////////////////////////
    /// \class bit::platform::keyboard_event
    ///
    /// \brief This type handles all events that come from keyboard input.
    ///
    /// Events with associated data can be polled via accessor functions.
    //////////////////////////////////////////////////////////////////////////
    struct keyboard_event
    {
      //----------------------------------------------------------------------
      // Public Member Types
      //----------------------------------------------------------------------
    public:

      /// \brief The underlying type of this event
      enum class event_type
      {
        keydown, ///< Events for keypresses
        keyup,   ///< Events for key-releases
      };

      /// \brief Data for window resize events
      struct keydown_data
      {
        key  key;    ///< The key being pressed
        bool repeat; ///< Boolean indicating whether this key is repeat press signal
      };

      /// \brief Data for window moving events
      struct keyup_data
      {
        key  key; ///< The key being released
      };

      //----------------------------------------------------------------------
      // Constructor
      //----------------------------------------------------------------------
    public:

      /// \brief Constructs a keyboard event from a keydown state
      ///
      /// \param data the keydown state
      explicit keyboard_event( const keydown_data& data ) noexcept;

      /// \brief Constructs a keyboard event from a keyup state
      ///
      /// \param data the keyup state
      explicit keyboard_event( const keyup_data& data ) noexcept;

      /// \brief Copy-constructs a keyboard_event
      ///
      /// \param other the other event to copy
      keyboard_event( const keyboard_event& other ) noexcept;

      //----------------------------------------------------------------------
      // Observers
      //----------------------------------------------------------------------
    public:

      /// \brief Returns a reference to the keydown data if this event is
      ///        a keydown event
      ///
      /// \throws bad_event_cast if exceptions are enabled and if this event is
      ///         not a keydown event; otherwise asserts
      /// \return a reference to the keydown data
      const keydown_data& keydown() const;

      /// \brief Returns a reference to the keyup data if this event is
      ///        a keyup event
      ///
      /// \throws bad_event_cast if exceptions are enabled and if this event is
      ///         not a keyup event; otherwise asserts
      /// \return a reference to the keyup data
      const keyup_data& keyup() const;

      /// \brief Returns the type of the underlying event
      ///
      /// \return the event type
      event_type type() const noexcept;

      //----------------------------------------------------------------------
      // Private Member Types
      //----------------------------------------------------------------------
    private:

      struct empty{};

      union storage_type
      {
        empty        empty;
        keydown_data keydown;
        keyup_data   keyup;

        storage_type() : empty(){}
        storage_type( const keydown_data& data ) : keydown(data){}
        storage_type( const keyup_data& data ) : keyup(data){}
      };

      //----------------------------------------------------------------------
      // Private Members
      //----------------------------------------------------------------------
    private:

      storage_type m_storage; ///< The type used for storage
      event_type   m_type;    ///< The tpye of the event
    };

    //------------------------------------------------------------------------
    // event : controller
    //------------------------------------------------------------------------

    //////////////////////////////////////////////////////////////////////////
    /// \class bit::platform::controller_event
    ///
    /// \brief This type handles all events that come from controller input,
    ///        such as button presses, trigger press, and analog input
    ///
    /// Events with associated data can be polled via accessor functions.
    //////////////////////////////////////////////////////////////////////////
    class controller_event
    {
      //----------------------------------------------------------------------
      // Public Member Types
      //----------------------------------------------------------------------
    public:

      /// \brief The underlying type of this event
      enum class event_type
      {
        joystick,       ///< joystick events
        trigger,        ///< trigger button events
        button_press,   ///< button press events
        button_release, ///< Button up events
        gyroscope,      ///< Not implemented yet. Will be for 6-axis control
      };

      /// \brief Data associated with joystick motion events
      ///
      /// This event data keeps track of the x/y axis of a given analog stick
      struct joystick_data
      {
        float x_axis; ///< The direction in the x-axis (-1.0 to 1.0)
        float y_axis; ///< The direction in the y-axis (-1.0 to 1.0)
        controller::joystick_side side; ///< The side of this analog
      };

      /// \brief Data associated with trigger events
      ///
      /// This event data keeps track of the pressure of a given trigger
      struct trigger_data
      {
        float pressure; ///< The amount of pressure applied (0.0 to 1.0)
        controller::trigger_side side; ///< The side of the trigger
      };

      /// \brief Data associated with button events
      struct button_data
      {
        controller::button button; ///< The button that changed
      };

      struct gyroscope_data
      {
        float roll;  ///<
        float pitch; ///<
        float yaw;   ///<
      };

      //----------------------------------------------------------------------
      // Constructors
      //----------------------------------------------------------------------
    public:

      /// \brief Constructs a controller_event with a given button state
      ///
      /// \param state the state of the button (up or down)
      /// \param data the data for the button
      explicit controller_event( controller::button_state state,
                                 const button_data& data ) noexcept;

      /// \brief Constructs a controller_event with a given joystick data
      ///
      /// \param data the joystick data
      explicit controller_event( const joystick_data& data ) noexcept;

      /// \brief Constructs a controller_event for a trigger event
      ///
      /// \param data the data for the trigger event
      explicit controller_event( const trigger_data& data ) noexcept;

      /// \brief Copy-constructs a controller_event
      ///
      /// \param other the other event to copy
      controller_event( const controller_event& other ) noexcept;

      //----------------------------------------------------------------------
      // Observers
      //----------------------------------------------------------------------
    public:

      /// \brief Returns a reference to the joystick data if this event is
      ///        a joystick event
      ///
      /// \throws bad_event_cast if exceptions are enabled and if this event is
      ///         not a joystick event; otherwise asserts
      /// \return a reference to the joystick data
      const joystick_data& joystick() const;

      /// \brief Returns a reference to the trigger data if this event is
      ///        a trigger event
      ///
      /// \throws bad_event_cast if exceptions are enabled and if this event is
      ///         not a trigger event; otherwise asserts
      /// \return a reference to the trigger data
      const trigger_data& trigger() const;

      /// \brief Returns a reference to the button data if this event is
      ///        a button event
      ///
      /// \throws bad_event_cast if exceptions are enabled and if this event is
      ///         not a button event; otherwise asserts
      /// \return a reference to the button data
      const button_data& button() const;

      /// \brief Returns the type of the underlying event
      ///
      /// \return the event type
      event_type type() const noexcept;

      //----------------------------------------------------------------------
      // Private Member Types
      //----------------------------------------------------------------------
    private:

      struct empty{};

      union storage_type
      {
        empty         empty;
        joystick_data joystick; ///< Data associated with a joystick event
        trigger_data  trigger;  ///< Data associated with a trigger event
        button_data   button;   ///< Data associated with a button event

        storage_type() : empty(){}
        storage_type( const joystick_data& data ) : joystick(data){}
        storage_type( const trigger_data& data ) : trigger(data){}
        storage_type( const button_data& data ) : button(data){}
      };

      //----------------------------------------------------------------------
      // Private Members
      //----------------------------------------------------------------------
    private:

      storage_type m_storage; ///< The type used for storage
      event_type   m_type;    ///< The tpye of the event
    };

    //------------------------------------------------------------------------
    // event : base class
    //------------------------------------------------------------------------

    //////////////////////////////////////////////////////////////////////////
    /// \brief
    ///
    //////////////////////////////////////////////////////////////////////////
    class event
    {
      //----------------------------------------------------------------------
      // Public Member Types
      //----------------------------------------------------------------------
    public:

      enum class event_category
      {
        window,     ///< category for window events
        keyboard,   ///< category for keyboard events
        controller, ///< category for controller events
      };

      //----------------------------------------------------------------------
      // Constructors
      //----------------------------------------------------------------------
    public:

      /// \brief Constructs this event out of a controller event
      ///
      /// \param event the controller event
      event( const controller_event& event ) noexcept;

      /// \brief Constructs this event out of a keyboard event
      ///
      /// \param event the keyboard event
      event( const keyboard_event& event ) noexcept;

      /// \brief Constructs this event out of a window_event
      ///
      /// \param event the window event
      event( const window_event& event ) noexcept;

      /// \brief Copy constructs this event out of another event
      ///
      /// \param other the other event to copy
      event( const event& other ) noexcept;

      //----------------------------------------------------------------------
      // Element Access
      //----------------------------------------------------------------------
    public:

      /// \brief Accesses the underlying controller event
      ///
      /// \throws bad_event_cast if exceptions are enabled and if this event is
      ///         not a controller event, otherwise asserts
      /// \return a reference to the controller event
      const controller_event& controller() const;

      /// \brief Accesses the underlying keyboard event
      ///
      /// \throws bad_event_cast if exceptions are enabled and if this event is
      ///         not a keyboard event, otherwise asserts
      /// \return a reference to the keyboard event
      const keyboard_event& keyboard() const;

      /// \brief Accesses the underlying window event
      ///
      /// \throws bad_event_cast if exceptions are enabled and if this event is
      ///         not a window event, otherwise asserts
      /// \return a reference to the window event
      const window_event& window() const;

      /// \brief Returns the underlying category of this event
      ///
      /// \return the category
      event_category category() const noexcept;

      //----------------------------------------------------------------------
      // Private Member Types
      //----------------------------------------------------------------------
    private:

      struct empty{};

      union storage_type
      {
        empty            empty;
        controller_event controller; ///< Event for controller updates
        keyboard_event   keyboard;   ///< Event for keyboard updates
        window_event     window;     ///< Event for window updates

        storage_type() : empty(){}
        storage_type( const controller_event& event ) : controller(event){}
        storage_type( const keyboard_event& event ) : keyboard(event){}
        storage_type( const window_event& event ) : window(event){}
      };

      //----------------------------------------------------------------------
      // Private Members
      //----------------------------------------------------------------------
    private:

      storage_type   m_storage;  ///< The type used for storage
      event_category m_category; ///< The tpye of the event
    };

  } // namespace platform
} // namespace bit

//============================================================================
// controller_event
//============================================================================

//----------------------------------------------------------------------------
// Observers
//----------------------------------------------------------------------------

inline bit::platform::controller_event::event_type
  bit::platform::controller_event::type()
  const noexcept
{
  return m_type;
}

//============================================================================
// keyboard_event
//============================================================================

//----------------------------------------------------------------------------
// Element Access
//----------------------------------------------------------------------------

inline bit::platform::keyboard_event::event_type
  bit::platform::keyboard_event::type()
  const noexcept
{
  return m_type;
}

//============================================================================
// window_event
//============================================================================

//----------------------------------------------------------------------------
// Element Access
//----------------------------------------------------------------------------

inline bit::platform::window_event::event_type bit::platform::window_event::type()
  const noexcept
{
  return m_type;
}

//============================================================================
// event
//============================================================================

//----------------------------------------------------------------------------
// Element Access
//----------------------------------------------------------------------------

inline bit::platform::event::event_category bit::platform::event::category()
  const noexcept
{
  return m_category;
}

#endif /* BIT_PLATFORM_SYSTEM_EVENT_HPP */
