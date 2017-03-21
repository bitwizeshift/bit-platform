/**
 * \file scope_clock.hpp
 *
 * \brief This header contains a small wrapper around a clock to time
 *        a given scope using RAII
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */

#ifndef BIT_PLATFORM_TIMING_SCOPE_CLOCK_HPP
#define BIT_PLATFORM_TIMING_SCOPE_CLOCK_HPP

#include "clock.hpp"

namespace bit {
  namespace platform {

    //////////////////////////////////////////////////////////////////////////
    /// \brief This class is an RAII wrapper around a clock for timing a
    ///        given scope.
    //////////////////////////////////////////////////////////////////////////
    class scope_clock
    {
      //----------------------------------------------------------------------
      // Public Member Types
      //----------------------------------------------------------------------
    public:

      using time_unit = clock::time_unit; ///< The unit of measure for timing

      //----------------------------------------------------------------------
      // Constructors / Destructor
      //----------------------------------------------------------------------
    public:

      /// \brief Constructs a scoped_lock from a given output time
      ///
      /// \param time pointer to the time_unit to write to
      explicit scope_clock( time_unit* time ) noexcept;

      /// Deleted copy constructor
      scope_clock( const scope_clock& ) = delete;

      /// Deleted move constructor
      scope_clock( scope_clock&& ) = delete;

      /// \brief Destructs the scope_clock and writes the result to the stored
      ///        time_point
      ~scope_clock() noexcept;

      //----------------------------------------------------------------------
      // Private Members
      //----------------------------------------------------------------------
    private:

      time_unit* const m_time;
      clock            m_clock;

    };

  } // namespace platform
} // namespace bit

//----------------------------------------------------------------------------
// Inline Definitions
//----------------------------------------------------------------------------

inline bit::platform::scope_clock::scope_clock( time_unit* time )
  noexcept
  : m_time(time),
    m_clock()
{
  m_clock.start();
}

//----------------------------------------------------------------------------

inline bit::platform::scope_clock::~scope_clock()
  noexcept
{
  *m_time = m_clock.elapsed();
  m_clock.stop();
}
#endif /* BIT_PLATFORM_TIMING_SCOPE_CLOCK_HPP */
