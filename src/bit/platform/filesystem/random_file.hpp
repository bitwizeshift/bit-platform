/**
 * \file random_file_device.hpp
 *
 * \brief This header contains an abstract_file implementation
 *        of file that only returns random data
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */

#ifndef SRC_BIT_PLATFORM_FILESYSTEM_RANDOM_FILE_HPP
#define SRC_BIT_PLATFORM_FILESYSTEM_RANDOM_FILE_HPP

#include <bit/platform/filesystem/filesystem.hpp>

#include <random>

namespace bit {
  namespace platform {

    //////////////////////////////////////////////////////////////////////////
    /// \brief An implementation of abstract_file that only ever returns
    ///        random data.
    ///
    /// Seeking and writes perform no operation.
    //////////////////////////////////////////////////////////////////////////
    class random_file : public abstract_file
    {
      //----------------------------------------------------------------------
      // Public Member Types
      //----------------------------------------------------------------------
    public:

      using size_type  = abstract_file::size_type;
      using index_type = abstract_file::index_type;

      //----------------------------------------------------------------------
      // Constructors
      //----------------------------------------------------------------------
    public:

      /// \brief Constructs a random_file from a given \p seed
      ///
      /// \param seed the seed for the random generation
      random_file( unsigned int seed );

      //----------------------------------------------------------------------
      // File API
      //----------------------------------------------------------------------
    public:

      /// \brief Closes this null_file if it's currently open
      void close() override;

      /// \brief Reads data into the specified \p buffer
      ///
      /// \note a null_file never reads any data
      ///
      /// \param buffer a span of bytes to write into
      /// \return a span containing the bytes read
      size_type read( stl::span<stl::byte> buffer ) override;

      /// \brief Writes data into the specified \p buffer
      ///
      /// \note a null_file never writes any data
      ///
      /// \param buffer a span of bytes to write to the file
      /// \return a span containing the bytes written
      size_type write( stl::span<const stl::byte> buffer ) override;

      //----------------------------------------------------------------------
      // Observers
      //----------------------------------------------------------------------
    public:

      /// \brief Returns the current position in the file
      ///
      /// \return the current position in the file
      index_type tell() const override;

      //----------------------------------------------------------------------
      // Seeking
      //----------------------------------------------------------------------
    public:

      /// \brief Seeks to the specified position in the file
      ///
      /// \param pos the position to seek to
      void seek( index_type pos ) override;

      /// \brief Seeks to the end of the file
      void seek_to_end() override;

      /// \brief Skips the number of bytes from the current position
      ///
      /// \param bytes the number of bytes to skip
      void skip( index_type bytes ) override;

      //----------------------------------------------------------------------
      // Private Members
      //----------------------------------------------------------------------
    private:

      std::mt19937                            m_generator;
      std::uniform_int_distribution<unsigned> m_distribution;
    };

  } // namespace platform
} // namespace bit

#endif /* SRC_BIT_PLATFORM_FILESYSTEM_RANDOM_FILE_HPP */
