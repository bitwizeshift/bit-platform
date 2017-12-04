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

#include <bit/stl/utilities/types.hpp>   // stl::function_t
#include <bit/stl/utilities/delegate.hpp> // stl::delegate
#include <bit/stl/containers/string_view.hpp>

#include <memory> // std::unique_ptr

namespace bit {
  namespace platform {

    //////////////////////////////////////////////////////////////////////////
    /// \brief Indicates the type of file action performed on directory
    ///        notification
    //////////////////////////////////////////////////////////////////////////
    enum class file_action
    {
      added,    ///< The file was added
      removed,  ///< The file was removed
      renamed,  ///< The file was renamed
      modified, ///< the file was modified
    };

    //////////////////////////////////////////////////////////////////////////
    /// \brief This class manages watching directories for any file or
    ///        directory change events.
    ///
    /// \note Registered callbacks are not invoked on the same thread as the
    ///       thread that creates this watcher
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

      /// \brief Constructs a directory watcher
      directory_watcher();

      /// \brief Destructs this directory watcher, halting any background
      ///        threads
      ~directory_watcher();

      //----------------------------------------------------------------------
      // Modifiers
      //----------------------------------------------------------------------
    public:

      /// \brief Stops this directory watcher.
      ///
      /// Any further attempts to register listeners become no-ops
      void stop();

      //----------------------------------------------------------------------

      /// \brief Registers a listener for a given \p directory
      ///
      /// \note This function is not thread-safe. As such, synchronization
      ///       needs to be done if 'register_listener' is to be called on
      ///       a different thread, such as the notification thread.
      ///
      /// \tparam callback the function pointer to register
      /// \param directory the directory to view
      /// \param recursive whether to recursively check subdirectories
      template<stl::function_t<void(stl::string_view,stl::string_view,file_action)> callback>
      void register_listener( stl::string_view directory,
                              bool recursive = true );

      /// \{
      /// \brief Registers a listener for a given \p directory
      ///
      /// \note This function is not thread-safe. As such, synchronization
      ///       needs to be done if 'register_listener' is to be called on
      ///       a different thread, such as the notification thread.
      ///
      /// \tparam T the type of the underlying instance
      /// \tparam callback the member function pointer to register
      /// \param instance the instance to invoke the member function on
      /// \param directory the directory to view
      /// \param recursive whether to recursively check subdirectories
      template<typename T, stl::member_function_t<T,void(stl::string_view,stl::string_view,file_action)> callback>
      void register_listener( T* instance,
                              stl::string_view directory,
                              bool recursive = true );

      template<typename T, stl::member_function_t<const T,void(stl::string_view,stl::string_view,file_action)> callback>
      void register_listener( const T* instance,
                              stl::string_view directory,
                              bool recursive = true );

      template<typename T, stl::member_function_t<const T,void(stl::string_view,stl::string_view,file_action)> callback>
      void register_listener( const T& instance,
                              stl::string_view directory,
                              bool recursive = true );

      template<typename T, stl::member_function_t<T,void(stl::string_view,stl::string_view,file_action)> callback>
      void register_listener( T& instance,
                              stl::string_view directory,
                              bool recursive = true );
      /// \}

      template<typename T, stl::member_function_t<T,void(stl::string_view,stl::string_view,file_action)>>
      void register_listener( std::nullptr_t, stl::string_view, bool = true ) = delete;

      template<typename T, stl::member_function_t<const T,void(stl::string_view,stl::string_view,file_action)>>
      void register_listener( std::nullptr_t, stl::string_view, bool = true ) = delete;

      // TODO: Unregister directory watchers

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
