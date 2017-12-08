/**
 * \file tags.hpp
 *
 * \brief This implementation file is used for tags used in tag dispatching
 *        within filesystems
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */
#ifndef BIT_PLATFORM_FILESYSTEM_DETAIL_TAGS_HPP
#define BIT_PLATFORM_FILESYSTEM_DETAIL_TAGS_HPP

namespace bit {
  namespace platform {
    namespace detail {
      struct async_tag{};
      struct sync_tag{};
    } // namespace detail

    //------------------------------------------------------------------------
    // Tags
    //------------------------------------------------------------------------

    /// \brief Tag used for tag-dispatching synchronous file API calls
#ifndef BIT_DOXYGEN
    inline void sync( detail::sync_tag ){}
#else
    void sync( /* implementation defined */ );
#endif

    /// \brief Tag used for tag-dispatching asynchronous file API calls
#ifndef BIT_DOXYGEN
    inline void async( detail::async_tag ){}
#else
    void async( /* implementation defined */ );
#endif

    //------------------------------------------------------------------------
    // Tag Types
    //------------------------------------------------------------------------

    /// \brief The type of the sync tag
#ifndef BIT_DOXYGEN
    using sync_t  = void(*)(detail::sync_tag);
#else
    using sync_t = void(*)( /* implementation defined */ );
#endif

    /// \brief The type of the async tag
#ifndef BIT_DOXYGEN
    using async_t = void(*)(detail::async_tag);
#else
    using async_t = void(*)( /* implementation defined */ );
#endif
  } // namespace platform
} // namespace bit



#endif /* BIT_PLATFORM_FILESYSTEM_DETAIL_TAGS_HPP */
