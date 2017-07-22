/**
 * \file directory_watcher.hpp
 *
 * \brief A class that watches activity in a given directory, and fires
 *        callbacks on changes.
 *
 * \author Matthew Rodusek (matthew.rodusek@gmail.com)
 */

#ifndef BIT_PLATFORM_FILESYSTEM_DIRECTORY_WATCHER_HPP
#define BIT_PLATFORM_FILESYSTEM_DIRECTORY_WATCHER_HPP

#include <bit/stl/stddef.hpp> // stl::function_t
#include <bit/stl/delegate.hpp>

#include <memory> // std::unique_ptr

namespace bit {
  namespace platform {

    enum class file_action
    {
      added,    ///< The file was added
      removed,  ///< The file was removed
      renamed,  ///< The file was renamed
      modified, ///< the file was modified
    };

    //////////////////////////////////////////////////////////////////////////
    /// \brief This class manages watching changes from directories,
    ///
    //////////////////////////////////////////////////////////////////////////
    class directory_watcher
    {
      //----------------------------------------------------------------------
      // Public Member Types
      //----------------------------------------------------------------------
    public:

      struct impl;

      //----------------------------------------------------------------------
      // Constructors
      //----------------------------------------------------------------------
    public:

      directory_watcher();

      ~directory_watcher();

      //----------------------------------------------------------------------
      // Modifiers
      //----------------------------------------------------------------------
    public:

      void stop();

      template<stl::function_t<void(stl::string_view,stl::string_view,file_action)> callback>
      void register_listener( stl::string_view directory,
                              bool recursive = true );

      //----------------------------------------------------------------------

      template<typename T, stl::member_function_t<T,void(stl::string_view,stl::string_view,file_action)> callback>
      void register_listener( T* instance,
                              stl::string_view directory,
                              bool recursive = true );

      template<typename T, stl::member_function_t<const T,void(stl::string_view,stl::string_view,file_action)> callback>
      void register_listener( const T* instance,
                              stl::string_view directory,
                              bool recursive = true );

      //----------------------------------------------------------------------

      template<typename T, stl::member_function_t<const T,void(stl::string_view,stl::string_view,file_action)> callback>
      void register_listener( const T& instance,
                              stl::string_view directory,
                              bool recursive = true );

      template<typename T, stl::member_function_t<T,void(stl::string_view,stl::string_view,file_action)> callback>
      void register_listener( T& instance,
                              stl::string_view directory,
                              bool recursive = true );

      //----------------------------------------------------------------------
      // Private Modifiers
      //----------------------------------------------------------------------
    private:

      void register_listener( stl::delegate<void(stl::string_view,stl::string_view,file_action)> callback,
                              stl::string_view directory,
                              bool recursive );

      //----------------------------------------------------------------------
      // Private Members
      //----------------------------------------------------------------------
    private:

      std::unique_ptr<impl> m_impl;
    };

  } // namespace platform
} // namespace bit

//============================================================================
// Inline Definitions
//============================================================================

//----------------------------------------------------------------------------
// Modifiers
//----------------------------------------------------------------------------

template<bit::stl::function_t<void(bit::stl::string_view,bit::stl::string_view,bit::platform::file_action)> callback>
void bit::platform::directory_watcher::register_listener( stl::string_view directory,
                                                          bool recursive )
{
  auto delegate = stl::delegate<void(stl::string_view,stl::string_view,file_action)>{};
  delegate.bind<callback>();

  register_listener( delegate, directory, recursive );
}

//----------------------------------------------------------------------------

template<typename T, bit::stl::member_function_t<T,void(bit::stl::string_view,bit::stl::string_view,bit::platform::file_action)> callback>
void bit::platform::directory_watcher::register_listener( T* instance,
                                                          stl::string_view directory,
                                                          bool recursive )
{
  auto delegate = stl::delegate<void(stl::string_view,stl::string_view,file_action)>{};
  delegate.bind<T,callback>( instance );

  register_listener( delegate, directory, recursive );
}

template<typename T, bit::stl::member_function_t<const T,void(bit::stl::string_view,bit::stl::string_view,bit::platform::file_action)> callback>
void bit::platform::directory_watcher::register_listener( const T* instance,
                                                          stl::string_view directory,
                                                          bool recursive )
{
  auto delegate = stl::delegate<void(stl::string_view,stl::string_view,file_action)>{};
  delegate.bind<T,callback>( instance );

  register_listener( delegate, directory, recursive );
}

//----------------------------------------------------------------------------

template<typename T, bit::stl::member_function_t<T,void(bit::stl::string_view,bit::stl::string_view,bit::platform::file_action)> callback>
void bit::platform::directory_watcher::register_listener( T& instance,
                                                          stl::string_view directory,
                                                          bool recursive )
{
  auto delegate = stl::delegate<void(stl::string_view,stl::string_view,file_action)>{};
  delegate.bind<T,callback>( instance );

  register_listener( delegate, directory, recursive );
}

template<typename T, bit::stl::member_function_t<const T,void(bit::stl::string_view,bit::stl::string_view,bit::platform::file_action)> callback>
void bit::platform::directory_watcher::register_listener( const T& instance,
                                                          stl::string_view directory,
                                                          bool recursive )
{
  auto delegate = stl::delegate<void(stl::string_view,stl::string_view,file_action)>{};
  delegate.bind<T,callback>( instance );

  register_listener( delegate, directory, recursive );
}


#endif /* BIT_PLATFORM_FILESYSTEM_DIRECTORY_WATCHER_HPP */
