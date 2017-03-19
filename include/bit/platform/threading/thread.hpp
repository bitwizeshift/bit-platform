/**
 * \file thread.hpp
 *
 * \brief A small API for adding additional functionality to the base
 *        std::thread api
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */
#ifndef BIT_PLATFORM_THREADING_THREAD_HPP
#define BIT_PLATFORM_THREADING_THREAD_HPP

#include <thread>

namespace bit {
  namespace platform {

    //------------------------------------------------------------------------
    // Affinity
    //------------------------------------------------------------------------

    /// \brief Sets this thread's affinity to a specific core id
    ///
    /// \brief This may not be implemented in some architectures that do not
    ///        support thread affinity
    ///
    /// \param core_id the id of the core
    void set_affinity( std::size_t core_id );

    /// \brief Sets the specified thread's affinity to a specific core id
    ///
    /// \brief This may not be implemented in some architectures that do not
    ///        support thread affinity
    ///
    /// \param thread the thread to set the affinity of
    /// \param core_id the id of the core
    void set_affinity( std::thread& thread, std::size_t core_id );

    //------------------------------------------------------------------------

    /// \brief Accesses the current thread's affinity
    ///
    /// \return the affinity of the thread
    std::size_t affinity();

    /// \brief Accesses the specified thread's affinity
    ///
    /// \param thread the thread to query the affinity of
    /// \return the affinity of the thread
    std::size_t affinity( std::thread& thread );

  } // namespace platform
} // namespace bit

#endif /* BIT_PLATFORM_THREADING_THREAD_HPP */
