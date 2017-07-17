/**
 * \file processor.hpp
 *
 * \brief todo: fill in documentation
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */

#ifndef BIT_PLATFORM_SYSTEM_PROCESSOR_HPP
#define BIT_PLATFORM_SYSTEM_PROCESSOR_HPP

#include <bit/stl/string_view.hpp>

#include <cstdlib> // std::size_t

namespace bit {
  namespace platform {

    /////////////////////////////////////////////////////////////////////////////
    /// \brief Wrapper around processor information, allowing easy queries for
    ///        supported instruction sets at compile time.
    /////////////////////////////////////////////////////////////////////////////
    class processor
    {
      //------------------------------------------------------------------------
      // Constructor
      //------------------------------------------------------------------------
    public:

      /// \brief Constructs a Processor object by calling the cpuid instruction
      processor() noexcept;

      /// \brief Copy-constructs a Processor
      ///
      /// \param other the other processor to copy
      processor( const processor& other ) noexcept = default;

      /// \brief Move-constructs a Processor
      ///
      /// \param other the other processor to move
      processor( processor&& other ) noexcept = default;

      //------------------------------------------------------------------------
      // CPU Info
      //------------------------------------------------------------------------
    public:

      /// \brief Returns the CPU Vendor string
      ///
      /// \return the CPU Vendor string
      stl::string_view cpu_vendor() const noexcept;

      /// \brief Returns the CPU Brand string
      ///
      /// \return the CPU brand string
      stl::string_view cpu_brand() const noexcept;

      /// \brief Returns whether or not RDTSC instructions are supported
      ///
      /// \return true if RDTSC is supported
      bool is_RDTSC_supported() const noexcept;

      /// \brief Returns whether or not CMOV instructions are supported
      ///
      /// \return true if CMOV is supported
      bool is_CMOV_supported() const noexcept;

      /// \brief Returns whether or not FCMOV instructions are supported
      ///
      /// \return true if FCMOV is supported
      bool is_FCMOV_supported() const noexcept;

      /// \brief Returns whether or not MMX instructions are supported
      ///
      /// \return true if MMX is supported
      bool is_mmx_supported() const noexcept;

      /// \brief Returns whether or not SSE instructions are supported
      ///
      /// \return true if SSE is supported
      bool is_sse_supported() const noexcept;

      /// \brief Returns whether or not SSE2 instructions are supported
      ///
      /// \return true if SSE2 is supported
      bool is_sse2_supported() const noexcept;

      /// \brief Returns whether or not SSE3 instructions are supported
      ///
      /// \return true if SSE3 is supported
      bool is_sse3_supported() const noexcept;

      /// \brief Returns whether or not SSE4.1 instructions are supported
      ///
      /// \return true if SSE4.1 is supported
      bool is_sse4_1_supported() const noexcept;

      /// \brief Returns whether or not SSE4.2 instructions are supported
      ///
      /// \return true if SSE4.2 is supported
      bool is_sse4_2_supported() const noexcept;

      /// \brief Returns whether or not AVX instructions are supported
      ///
      /// \return true if AVX is supported
      bool is_avx_supported() const noexcept;

      /// \brief Returns whether or not f16c instructions are supported
      ///
      /// \return true if f216c is supported
      bool is_f16c_supported() const noexcept;

      /// \brief Returns whether or not 3D-Now! instructions are supported
      ///
      /// \return true if 3D-Now! is supported
      bool is_3dnow_supported() const noexcept;

      /// \brief Returns whether or not the CPU supports on-chip random number
      ///        generation (RDRAND).
      ///
      /// \return true if RDRAND is supported
      bool is_rdrand_supported() const noexcept;

      /// \brief Returns whether or not hyper threading is supported
      ///
      /// \return true if hyper threading is supported
      bool is_hyper_threading_supported() const noexcept;

      /// \brief Returns the number of logical processors
      ///
      /// \return the number of logical processors
      std::size_t logical_processors() const noexcept;

      //-------------------------------------------------------------------------
      // Private Members
      //-------------------------------------------------------------------------
    private:

      char m_vendor_string[13];    ///< The processor's vender string
      char m_brand_string[49];     ///< The processor's brand string
      std::size_t m_processors;    ///< Number of physical processors in the computer.
      std::size_t m_logical_cores; ///< Number of logical processor cores.

      bool m_is_intel   : 1; ///< Is the processor an intel processor?
      bool m_is_amd     : 1; ///< Is the processor an AMD processor?
      bool m_has_RDTSC  : 1; ///< Is RDTSC supported?
      bool m_has_CMOV   : 1; ///< Is CMOV supported?
      bool m_has_FCMOV  : 1; ///< Is FCMOV supported?
      bool m_has_MMX    : 1; ///< Is MMX supported?
      bool m_has_SSE    : 1; ///< Is SSE supported?
      bool m_has_SSE2   : 1; ///< Is SSE2 supported?
      bool m_has_SSE3   : 1; ///< Is SSE3 supported?
      bool m_has_SSSE3  : 1; ///< Is supplementary SSE3 supported?
      bool m_has_SSE4_1 : 1; ///< Is SSE4.1 supported?
      bool m_has_SSE4_2 : 1; ///< Is SSE4.2 supported?
      bool m_has_AVX    : 1; ///< IS AVX supported?
      bool m_has_f16c   : 1; ///< Is F16 floats supported?
      bool m_has_3DNow  : 1; ///< Is 3DNow supported?
      bool m_has_rdrand : 1; ///< Is RDRAND supported?
      bool m_has_HT     : 1; ///< Is HyperThreading supported?

    };

  } // namespace platform
} // namespace bit

#endif /* BIT_PLATFORM_SYSTEM_PROCESSOR_HPP */
