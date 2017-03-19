/**
 * \file crypto_file.hpp
 *
 * \brief This header contains an implementation of the cryptographic
 *        file device adapter.
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */

#ifndef SRC_BIT_PLATFORM_FILESYSTEM_CRYPTO_FILE_HPP
#define SRC_BIT_PLATFORM_FILESYSTEM_CRYPTO_FILE_HPP

#include <bit/platform/filesystem/filesystem.hpp>

namespace bit {
  namespace platform {

    //////////////////////////////////////////////////////////////////////////
    /// \brief The crypto_file class is an adapter around any abstract_file
    ///        that encrypts and decrypts files using a stream-cypher
    ///        algorithm.
    ///
    /// The crypto_file can only be created through piggybacking, as it
    /// requires an instance of a file to work.
    //////////////////////////////////////////////////////////////////////////
    class crypto_file final : public abstract_file
    {
      //----------------------------------------------------------------------
      // Public Member Types
      //----------------------------------------------------------------------
    public:

      using size_type  = abstract_file::size_type;
      using index_type = abstract_file::index_type;

      //----------------------------------------------------------------------
      // Constructor
      //----------------------------------------------------------------------
    public:

      /// \brief Constructs a crypto file from an underlying \p file pointer
      ///
      /// \param file
      explicit crypto_file( abstract_file* file );

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

      abstract_file* m_file;
    };

  } // namespace platform
} // namespace bit

#endif /* SRC_BIT_PLATFORM_FILESYSTEM_CRYPTO_FILE_HPP */
