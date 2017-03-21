/**
 * \file clock.hpp
 *
 * \brief This header contains the definition for a high-resolution timer
 *        to keep track of elapsed times.
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */

#ifndef BIT_PLATFORM_TIMING_CLOCK_HPP
#define BIT_PLATFORM_TIMING_CLOCK_HPP

#include <chrono>

namespace bit {
  namespace platform {

    //////////////////////////////////////////////////////////////////////////
    /// \brief A small high-definition timer class for keeping track of
    ///        elapsed time.
    //////////////////////////////////////////////////////////////////////////
    class clock final
    {
      //----------------------------------------------------------------------
      // Public Member Types
      //----------------------------------------------------------------------
    public:

      using time_point = std::chrono::high_resolution_clock::time_point;
      using time_unit  = time_point::duration;

      //----------------------------------------------------------------------
      // Constructors
      //----------------------------------------------------------------------
    public:

      /// \brief Constructs a clock with the given \p scale
      ///
      /// \param scale the scale for the clock
      explicit clock( double scale = 1.0 ) noexcept;

      /// \brief Move-constructs a clock from another clock
      ///
      /// \param other the other clock to move
      clock( clock&& other ) noexcept = default;

      /// Deleted copy constructor
      clock( const clock& other ) = delete;

      //----------------------------------------------------------------------

      /// \brief Move-assigns a clock from another clock
      ///
      /// \param other the other clock to move
      /// \return a reference to \c (*this)
      clock& operator=( clock&& other ) noexcept = default;

      /// Deleted copy assignment
      clock& operator=( const clock& other ) = delete;

      //----------------------------------------------------------------------
      // Modifiers
      //----------------------------------------------------------------------
    public:

      /// \brief Sets the scale for this clock
      ///
      /// \param scale the scale to set
      void set_scale( double scale ) noexcept;

      //----------------------------------------------------------------------
      // Timing
      //----------------------------------------------------------------------
    public:

      /// \brief Starts the timing on the clock
      void start() noexcept;

      /// \brief Stops a running clock
      void stop() noexcept;

      /// \brief Resets the start timer on the clock
      void reset() noexcept;

      /// Get the split time in microseconds
      ///
      /// \return the time in microseconds since the last split
      time_unit split();

      /// Get the elapsed time in microseconds
      ///
      /// \return the time in microseconds since the clock started
      time_unit elapsed();

      //----------------------------------------------------------------------
      // Private Member Functions
      //----------------------------------------------------------------------
    private:

      double    m_scale;      ///< Scale the time moves at
      bool      m_is_running; ///< Is the clock running?
      time_point m_start;      ///< The start timer (epoch) to calculate elapsed times
      time_point m_split;      ///< The split timer for frequent resets
    };

  } // namespace platform
} // namespace bit

#endif /* BIT_PLATFORM_TIMING_CLOCK_HPP */
