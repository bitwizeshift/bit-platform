/**
 * \file async_file.hpp
 *
 * \brief This header contains a non-owning 'async file' type
 *
 * \note This is an internal header file, included by other library headers.
 *       Do not attempt to use it directly.
 */
#ifndef BIT_PLATFORM_FILESYSTEM_DETAIL_FILESYSTEM_ASYNC_FILE_HPP
#define BIT_PLATFORM_FILESYSTEM_DETAIL_FILESYSTEM_ASYNC_FILE_HPP

namespace bit {
  namespace platform {

    class async_file;

    /// \brief Type-trait to determine whether a given type is a valid
    ///        async-file-implementation
    ///
    /// The result is aliased as \c ::value
    template<typename T, typename = void>
    struct is_async_file_implementation : std::false_type{};

    template<typename T>
    struct is_async_file_implementation<T,stl::void_t<
      decltype( std::declval<T&>().close() ),
      decltype( (std::size_t) std::declval<T&>().write( std::declval<stl::span<const stl::byte>>() ) ),
      decltype( (std::size_t) std::declval<T&>().read( std::declval<stl::span<stl::byte>>() ) ),
      decltype( (std::size_t) std::declval<T&>().size() ),
      decltype( (std::size_t) std::declval<T&>().tell() ),
      decltype( (stl::index_t) std::declval<T&>().seek( std::declval<stl::index_t>() ) ),
      decltype( (stl::index_t) std::declval<T&>().skip( std::declval<stl::index_t>() ) )
    >> : stl::negation<std::is_same<T,async_file>>{};

    class async_file_operation final
    {
      //----------------------------------------------------------------------
      // Public Member Types
      //----------------------------------------------------------------------
    public:

      //----------------------------------------------------------------------
      // Async Operation
      //----------------------------------------------------------------------
    public:
      bool completed() const;

      stl::span<const stl::byte> wait() const;

      void cancel();

    private:
    };

    class async_file final
    {
      //----------------------------------------------------------------------
      // Public Member Types
      //----------------------------------------------------------------------
    public:

      using operation = async_file_operation;
      using index_type = stl::index_t;
      using size_type  = std::size_t;

      void close();

      operation read( index_type pos, stl::span<stl::byte> buffer );
      operation write( index_type pos, stl::span<const stl::byte> buffer );
    };

    template<typename T>
    struct is_async_file : std::false_type{};

    template<>
    struct is_async_file<async_file> : std::true_type{};

  } // namespace platform
} // namespace bit

#endif /* BIT_PLATFORM_FILESYSTEM_DETAIL_FILESYSTEM_ASYNC_FILE_HPP */
