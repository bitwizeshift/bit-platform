/**
 * \file controller.hpp
 *
 * \brief todo: fill in documentation
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */

#ifndef BIT_PLATFORM_SYSTEM_CONTROLLER_HPP
#define BIT_PLATFORM_SYSTEM_CONTROLLER_HPP

#include <cstddef> // std::size_t

namespace bit {
  namespace platform {

    //////////////////////////////////////////////////////////////////////////
    ///
    ///
    //////////////////////////////////////////////////////////////////////////
    class controller
    {
      //----------------------------------------------------------------------
      // Public Static Members
      //----------------------------------------------------------------------
    public:

      /// The maximum number of controllers
      static constexpr std::size_t max_connections = 4;

      /// The maximum number of buttons a controller may have
      static constexpr std::size_t max_buttons = 24;

      /// The maximum number of axes supported per controller
      static constexpr std::size_t max_axes = 6;

      //----------------------------------------------------------------------
      // Public Member Types
      //----------------------------------------------------------------------
    public:

      /// \brief Enumeration of all the available buttons for a given
      ///        controller
      ///
      /// Semantic names are given to common buttons in controllers
      enum class button
      {
        button0,  button1,  button2,  button3,  button4,  button5,
        button6,  button7,  button8,  button9,  button10, button11,
        button12, button13, button14, button15, button16, button17,
        button18, button19, button20, button21, button22, button23,
        button24, button25, button26, button27, button28, button29,
        button30, button31,

        // Direction buttons
        up    = button0, ///< d-pad up
        left  = button1, ///< d-pad left
        right = button2, ///< d-pad right
        down  = button3, ///< d-pad down

        // Basic control
        control0 = button4, ///< control-flow button, conventionally 'select'
        control1 = button5, ///< control-flow button, conventionally 'start'
        control2 = button6, ///< additional control-flow 0
        control3 = button7, ///< additional control-flow 1

        // Basic input
        input0 = button8,  ///< input button 1 (a, b, x, y, etc)
        input1 = button9,  ///< input button 2 (a, b, x, y, etc)
        input2 = button10, ///< input button 3 (a, b, x, y, etc)
        input3 = button11, ///< input button 4 (a, b, x, y, etc)
        input4 = button12, ///< input button 5 (a, b, x, y, etc)
        input5 = button13, ///< input button 6 (a, b, x, y, etc)
        input6 = button14, ///< input button 7 (a, b, x, y, etc)
        input7 = button15, ///< input button 8 (a, b, x, y, etc)

        // Sides
        left0  = button16, ///< left-bumper
        left1  = button17, ///< left-trigger
        left2  = button18, ///< left-analog button
        left3  = button19, ///< additional left-button
        right0 = button20, ///< right-bumper
        right1 = button21, ///< right-trigger
        right2 = button22, ///< right-analog button
        right3 = button23, ///< additional right-button

        // Controller input
        system0 = button24, ///< system button (ps-button,x-button,etc)
        system1 = button25, ///< additional system button
      };

      ///
      enum class button_state
      {
        pressed, ///<
        released ///<
      };

      ///
      enum class joystick_side
      {
        left,  ///<
        right, ///<
      };

      ///
      enum class trigger_side
      {
        left,  ///<
        right, ///<
      };

      ///
      enum class gyroscope_axis
      {
        roll,  ///< The roll axis
        pitch, ///< The pitch axis
        yaw    ///< The yaw axis
      };
    };

  } // namespace platform
} // namespace bit

#endif /* BIT_PLATFORM_SYSTEM_CONTROLLER_HPP */
