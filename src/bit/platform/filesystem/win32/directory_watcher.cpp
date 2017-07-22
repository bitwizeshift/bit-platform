#include <bit/platform/filesystem/directory_watcher.hpp>

#include <bit/stl/string_view.hpp>
#include <bit/stl/delegate.hpp>

#ifndef NOMINMAX
# define NOMINMAX 1
#endif
#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>

#include <string>  // std::string
#include <map>     // std::map
#include <vector>  // std::vector
#include <codecvt> // std::wstring_convert
#include <thread>  // std::thread
#include <chrono>  // std::chrono_literals, std::duration

namespace {

  void worker_thread_function( bit::platform::directory_watcher::impl* );

  bit::platform::file_action to_file_action( ::DWORD action );

  struct object_handle
  {
    ::HANDLE    handle;
    bool        recursive;
    std::string directory;
    ::DWORD     notification_filter;
  };

  struct directory_handle
  {
    using callback_type = bit::stl::delegate<void(bit::stl::string_view,bit::stl::string_view,bit::platform::file_action)>;
    using callback_container = std::vector<callback_type>;

    ::HANDLE           handle;
    callback_container callbacks;
  };

} // anonymous namespace

struct bit::platform::directory_watcher::impl
{
  bool                                is_running;
  std::vector<object_handle>          handles;
  std::map<::HANDLE,directory_handle> directory;
  std::thread                         thread;
};

//----------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------

bit::platform::directory_watcher::directory_watcher()
  : m_impl(std::make_unique<impl>())
{
  m_impl->is_running = true;
  m_impl->thread = std::thread( &worker_thread_function, m_impl.get() );
}

bit::platform::directory_watcher::~directory_watcher()
{
  stop();

}

void bit::platform::directory_watcher::stop()
{
  if( m_impl->is_running ) {
    m_impl->is_running = false;
    m_impl->thread.join();

    // Close the open notification handles
    for( auto const& handle : m_impl->handles ) {
      ::FindCloseChangeNotification( handle.handle );
    }

    // Close the open directory handles
    for( auto const& pair : m_impl->directory ) {
      ::CloseHandle( pair.second.handle );
    }
  }
}

void bit::platform::directory_watcher::register_listener( stl::delegate<void(stl::string_view,stl::string_view,file_action)> callback,
                                                          stl::string_view directory,
                                                          bool recursive )
{
  auto filter = FILE_NOTIFY_CHANGE_FILE_NAME  | \
                FILE_NOTIFY_CHANGE_DIR_NAME   | \
                FILE_NOTIFY_CHANGE_SIZE       | \
                FILE_NOTIFY_CHANGE_LAST_WRITE | \
                FILE_NOTIFY_CHANGE_CREATION;
  auto handle = ::FindFirstChangeNotificationA( directory.data(), recursive, filter );

  const auto object = object_handle{handle,recursive,static_cast<std::string>(directory),static_cast<::DWORD>(filter) };
  m_impl->handles.emplace_back( std::move(object) );

  auto& dir = m_impl->directory[handle];
  if( !dir.handle ) {
    dir.handle = ::CreateFileA(directory.data(),
                               FILE_LIST_DIRECTORY,
                               FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
                               0,
                               OPEN_EXISTING,
                               FILE_FLAG_BACKUP_SEMANTICS,
                               nullptr);
  }
  dir.callbacks.push_back(callback);
}

namespace {

  void worker_thread_function( bit::platform::directory_watcher::impl* impl )
  {
    using namespace std::chrono_literals;
    static constexpr auto file_count = 128;

    using storage_type = std::aligned_storage_t<256,alignof(::FILE_NOTIFY_INFORMATION)>;
    storage_type buffer;

    while(impl->is_running)
    {
      if( impl->handles.empty() ) {
        std::this_thread::yield();
        continue;
      }

      auto result = ::WaitForMultipleObjects( impl->handles.size(), (void**) &impl->handles[0], false, (1000ms).count() );

      // If we timed out, yield this processor's time slice and try again later
      if( result == WAIT_TIMEOUT ) {
        std::this_thread::yield();
        continue;
      }

      auto index = result - WAIT_OBJECT_0;
      auto& handle    = impl->handles[index];
      auto& directory = impl->directory[handle.handle];

      auto bytes_read = ::DWORD{};

      ::ReadDirectoryChangesW( directory.handle,
                               (void*) &buffer,
                               file_count,
                               static_cast<::WINBOOL>(handle.recursive),
                               handle.notification_filter,
                               &bytes_read,
                               nullptr,
                               nullptr );

      // TODO: Manage error (?)
      if( bytes_read <= 0 ) continue;

      using converter_type = std::codecvt_utf8<wchar_t>;
      std::wstring_convert<converter_type, wchar_t> converter;

      auto file = reinterpret_cast<::FILE_NOTIFY_INFORMATION*>(&buffer);
      auto wfilename = std::wstring( file->FileName, file->FileNameLength / sizeof(::WCHAR) );
      auto filename = converter.to_bytes(wfilename);

      auto action = to_file_action(file->Action);

      // Invoke each callback, supplying the filename
      for( auto const& callback : directory.callbacks ) {
        callback( handle.directory, filename, action );
      }

      // Keep waiting for the next notification
      ::FindNextChangeNotification( handle.handle );
    }
  }

  bit::platform::file_action to_file_action( ::DWORD action )
  {
    switch( action ) {
    case FILE_ACTION_ADDED:
      return bit::platform::file_action::added;
    case FILE_ACTION_REMOVED:
      return bit::platform::file_action::removed;
    case FILE_ACTION_MODIFIED:
      return bit::platform::file_action::modified;
    case FILE_ACTION_RENAMED_OLD_NAME:
      return bit::platform::file_action::renamed;
    case FILE_ACTION_RENAMED_NEW_NAME:
      return bit::platform::file_action::renamed;
    default: break;
    }
    return {};
  }

} // anonymous namespace
