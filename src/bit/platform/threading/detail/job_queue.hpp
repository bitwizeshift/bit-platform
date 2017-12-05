/**
 * \file job_queue.hpp
 *
 * \brief TODO: Add description
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */
#ifndef SRC_BIT_PLATFORM_THREADING_DETAIL_JOB_QUEUE_HPP
#define SRC_BIT_PLATFORM_THREADING_DETAIL_JOB_QUEUE_HPP

#include <bit/platform/threading/job.hpp> // job

#include <array>   // std::array
#include <mutex>   // std::mutex
#include <array>   // std::array
#include <cstddef> // std::ptrdiff_t

namespace bit {
  namespace platform {
    namespace detail {

    //=========================================================================
    // job_queue
    //=========================================================================

    ///////////////////////////////////////////////////////////////////////////
    ///
    ///
    ///////////////////////////////////////////////////////////////////////////
    class job_queue
    {
      //-----------------------------------------------------------------------
      // Constructor
      //-----------------------------------------------------------------------
    public:

      job_queue();

      //-----------------------------------------------------------------------
      // Modifiers
      //-----------------------------------------------------------------------
    public:

      /// \brief Pushes a new job into the queue
      ///
      /// \param j the job to push
      void push( job j );

      /// \brief Pops a job from the front of this job_queue
      ///
      /// \return the popped job, or nullptr on failure
      job pop();

      /// \brief Steals a job from the back of this job_queue
      ///
      /// \return the stolen job, or nullptr on failure
      job steal();

      //-----------------------------------------------------------------------
      // Capacity
      //-----------------------------------------------------------------------
    public:

      /// \brief Queries whether this job_queue is empty
      ///
      /// \return \c true when empty
      bool empty() const noexcept;

      //-----------------------------------------------------------------------
      // Private Members
      //-----------------------------------------------------------------------
    private:

      static auto constexpr max_jobs = job::max_jobs;

      std::array<job,max_jobs> m_jobs;
      std::ptrdiff_t           m_bottom;
      std::ptrdiff_t           m_top;
      mutable std::mutex       m_lock;
    };

    } // namespace detail
  } // namespace platform
} // namespace bit

#endif /* SRC_BIT_PLATFORM_THREADING_DETAIL_JOB_QUEUE_HPP */
