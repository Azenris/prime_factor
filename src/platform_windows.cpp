
#include <windows.h>
#include <windowsx.h>
#undef min
#undef max
#undef near
#undef far

using PlatformDataFlags = u32;
enum PLATFORM_DATA : PlatformDataFlags
{
	PLATFORM_DATA_RUNNING = BIT( 0 ),
};

struct PlatformData
{
	HINSTANCE hInstance;
	PlatformDataFlags flags;
	Array<HANDLE, MAX_OPEN_FILES> allOpenFiles;
	Array<u32, MAX_OPEN_FILES> freeOpenFileIDs;
	HANDLE loggerFile;
	bool colouredConsole = false;
	u64 activeTimer;
	u64 tickFrequency;
};

PlatformData platformData;

// ---------------------------------------------------
// Console
BOOL WINAPI console_control_handler( DWORD signalType )
{
	switch ( signalType )
	{
	case CTRL_CLOSE_EVENT:
	case CTRL_C_EVENT:
		FreeConsole();
		return true;
	}

	return false;
}

// ---------------------------------------------------
// Messages
void show_debug_message_function( const char *message, const char *messageColoured, ... )
{
	char bufferColoured[ MAX_DEBUG_MESSAGE ];

	va_list args, copyArgs;
	va_start( args, messageColoured );
	va_copy( copyArgs, args );

	platform_logger_message( message, args );

	vsnprintf( bufferColoured, ARRAY_LENGTH( bufferColoured ), platformData.colouredConsole ? messageColoured : message, copyArgs );

	HANDLE consoleHandle = GetStdHandle( STD_OUTPUT_HANDLE );
	WriteConsoleA( consoleHandle, bufferColoured, (DWORD)string_utf8_bytes( bufferColoured ) - 1, 0, 0 );

	va_end( args );
	va_end( copyArgs );
}

void show_debug_message_function_ext( const char *message, const char *messageColoured, va_list args )
{
	char bufferColoured[ MAX_DEBUG_MESSAGE ];

	va_list copyArgs;
	va_copy( copyArgs, args );

	platform_logger_message( message, args );

	vsnprintf( bufferColoured, ARRAY_LENGTH( bufferColoured ), platformData.colouredConsole ? messageColoured : message, copyArgs );

	HANDLE consoleHandle = GetStdHandle( STD_OUTPUT_HANDLE );
	WriteConsoleA( consoleHandle, bufferColoured, (DWORD)string_utf8_bytes( bufferColoured ) - 1, 0, 0 );

	va_end( copyArgs );
}

void platform_print_error()
{
	i32 e = GetLastError();
	LPTSTR error_text = nullptr;

	FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS, 0, e, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), (LPTSTR)&error_text, 0, 0 );

	if ( error_text )
	{
		show_debug_message_function( error_text, error_text );
		LocalFree( error_text );
	}
}

// Timing
/// @desc Returns how many tick counts happen per second. Cannot change after system boot. Only need once and cache.
[[nodiscard]] inline u64 platform_get_tick_frequency()
{
	LARGE_INTEGER freq;
	QueryPerformanceFrequency( &freq );
	return freq.QuadPart;
}

[[nodiscard]] inline u64 platform_get_tick_counter()
{
	LARGE_INTEGER counter;
	QueryPerformanceCounter( &counter );
	return counter.QuadPart;
}

[[nodiscard]] inline u64 platform_get_cycle_counter()
{
	return __rdtsc();
}

inline u64 platform_timer_start()
{
	platformData.activeTimer = platform_get_tick_counter();
	return platformData.activeTimer;
}

[[nodiscard]] inline u64 platform_timer_stop()
{
	u64 elapsedMicroseconds = ( platform_get_tick_counter() - platformData.activeTimer ) * 1000000 / platformData.tickFrequency;
	platformData.activeTimer = 0;
	return elapsedMicroseconds;
}

// ---------------------------------------------------
// Date
[[nodiscard]] Date platform_get_system_date()
{
	SYSTEMTIME st;
	GetSystemTime( &st );

	return Date
	{
		.dayOfWeek = static_cast<u16>( ( st.wDayOfWeek + 6 ) % 7 ), // change from sunday 0, to monday 0
		.day = st.wDay,
		.month = st.wMonth,
		.year = st.wYear,
		.hour = st.wHour,
		.minute = st.wMinute,
		.second = st.wSecond,
		.milliSecond = st.wMilliseconds,
	};
}

[[nodiscard]] Date platform_get_local_date()
{
	SYSTEMTIME st;
	GetLocalTime( &st );

	return Date
	{
		.dayOfWeek = static_cast<u16>( ( st.wDayOfWeek + 6 ) % 7 ), // change from sunday 0, to monday 0
		.day = st.wDay,
		.month = st.wMonth,
		.year = st.wYear,
		.hour = st.wHour,
		.minute = st.wMinute,
		.second = st.wSecond,
		.milliSecond = st.wMilliseconds,
	};
}

// ---------------------------------------------------
// Directory
bool platform_create_directory( const char *directory )
{
	char pathMem[ 4096 ];
	if ( string_utf8_copy( pathMem, directory ) == 0 )
		return false;

	char *path = pathMem;
	const char *token;
	const char *delimiters = "./\\";
	char delim;
	char dir[ 4096 ] = "\0";

	if ( *path == '.' )
	{
		++path;

		if ( *path == '/' || *path == '\\' )
		{
			// ./ (current directory)
			string_utf8_append( dir, "./" );
		}
		else if ( *path == '.' )
		{
			++path;

			// ../ (moving up from current directory)
			if ( *path == '/' || *path == '\\' )
			{
				// ./ (current directory)
				string_utf8_append( dir, "../" );
			}
		}
	}

	path = string_utf8_tokenise( path, delimiters, &token, &delim );

	while ( token )
	{
		if ( delim == '.' )
		{
			// file ext
			return true;
		}

		string_utf8_append( dir, token );
		string_utf8_append( dir, "/" );

		if ( !CreateDirectory( dir, nullptr ) )
			return false;

		path = string_utf8_tokenise( path, delimiters, &token, &delim );
	}

	return true;
}

inline void platform_set_current_directory( const char *workingDirectory )
{
	if ( SetCurrentDirectory( workingDirectory ) )
		string_utf8_copy( program->workingDirectory, workingDirectory );
}

[[nodiscard]] char *platform_get_current_directory( MemoryArena *arena )
{
	// Determine the size needed (including the null terminator)
	DWORD size = GetCurrentDirectory( 0, NULL );
	if ( size == 0 )
		return nullptr;

	// Allocate some memory for it
	char *buffer = (char *)memory_arena_transient_allocate( arena, size );

	// Get the current directory and return it
	if ( GetCurrentDirectory( size, buffer ) )
		return buffer;

	// Failed
	platform_print_error();
	return nullptr;
}

// ---------------------------------------------------
// Files
[[nodiscard]] char *platform_get_relative_filename( char *path )
{
	path = string_utf8_past_start_case_insensitive( path, program->workingDirectory );
	while ( *path == '\\' || *path == '/' )
		++path;
	return path;
}

[[nodiscard]] const char *platform_get_relative_filename( const char *path )
{
	path = string_utf8_past_start_case_insensitive( path, program->workingDirectory );
	while ( *path == '\\' || *path == '/' )
		++path;
	return path;
}

[[nodiscard]] u8 *platform_read_file( const char *path, u64 *fileSize, bool addNullTerminator, MemoryArena *arena )
{
	HANDLE file = CreateFile( path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0 );

	if ( file == INVALID_HANDLE_VALUE )
	{
		show_log_warning( "Failed to open file: %s", path );
		return nullptr;
	}

	LARGE_INTEGER size;

	if ( !GetFileSizeEx( file, &size ) )
	{
		CloseHandle( file );
		show_log_warning( "Failed to get size for file: %s", path );
		return nullptr;
	}

	DWORD fileSizeInBytes = (DWORD)size.QuadPart;
	DWORD bytesRead;

	u8 *buffer = memory_arena_transient_allocate( arena, fileSizeInBytes + ( addNullTerminator ? 1 : 0 ) );

	if ( !buffer )
	{
		CloseHandle( file );
		show_log_warning( "Failed to allocate memory ( %d bytes )", fileSizeInBytes );
		return nullptr;
	}

	if ( !ReadFile( file, buffer, fileSizeInBytes, &bytesRead, 0 ) || fileSizeInBytes != bytesRead )
	{
		CloseHandle( file );
		memory_arena_transient_free( arena, buffer );
		show_log_warning( "Failed to read file: %s", path );
		return nullptr;
	}

	if ( addNullTerminator )
	{
		buffer[ bytesRead ] = '\0';
		bytesRead += 1;
	}

	if ( fileSize )
		*fileSize = bytesRead;

	CloseHandle( file );

	return buffer;
}

u64 platform_write_file( const char *path, const u8 *buffer, u64 size, bool append )
{
	HANDLE file = CreateFile( path, append ? FILE_APPEND_DATA : GENERIC_WRITE, FILE_SHARE_WRITE, 0, append ? OPEN_ALWAYS : CREATE_ALWAYS, 0, 0 );

	if ( file == INVALID_HANDLE_VALUE )
	{
		show_log_warning( "Failed to open file: %s", path );
		return 0;
	}

	if ( append )
	{
		DWORD result = SetFilePointer( file, 0, 0, FILE_END );

		if ( result == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR )
		{
			CloseHandle( file );
			show_log_warning( "Failed to set pointer to end of file: %s", path );
			return 0;
		}
	}

	massert( size < UINT32_MAX );

	DWORD bytesWritten;

	BOOL result = WriteFile( file, buffer, (u32)size, &bytesWritten, 0 );

	CloseHandle( file );

	if ( !result || size != bytesWritten )
	{
		show_log_warning( "Failed to write file: %s", path );
		return 0;
	}

	return bytesWritten;
}

u32 platform_open_file( const char *path, FileOptions options )
{
	DWORD access = 0;
	DWORD flags = OPEN_EXISTING;
	DWORD shareMode = 0;

	if ( options & FILE_OPTION_READ )	{ access |= GENERIC_READ; shareMode |= FILE_SHARE_READ; }
	if ( options & FILE_OPTION_WRITE )	{ access |= GENERIC_WRITE; shareMode |= FILE_SHARE_WRITE; }
	if ( options & FILE_OPTION_CREATE )	{ flags = OPEN_ALWAYS; }
	if ( options & FILE_OPTION_CLEAR )	{ flags = CREATE_ALWAYS; }

	HANDLE file = CreateFile( path, access, shareMode, 0, flags, 0, 0 );

	if ( file == INVALID_HANDLE_VALUE )
	{
		show_log_warning( "Failed to open file: %s", path );
		return INVALID_FILE_INDEX;
	}

	if ( options & FILE_OPTION_APPEND )
	{
		DWORD result = SetFilePointer( file, 0, 0, FILE_END );

		if ( result == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR )
		{
			CloseHandle( file );
			show_log_warning( "Failed to set pointer to end of file: %s", path );
			return INVALID_FILE_INDEX;
		}
	}

	u32 fileID = platformData.freeOpenFileIDs.pop();
	platformData.allOpenFiles[ fileID ] = file;
	return fileID;
}

void platform_close_file( u32 fileID )
{
	CloseHandle( platformData.allOpenFiles[ fileID ] );
	platformData.freeOpenFileIDs.add( fileID );
}

u64 platform_get_file_size( u32 fileID )
{
	LARGE_INTEGER size;

	if ( !GetFileSizeEx( platformData.allOpenFiles[ fileID ], &size ) )
	{
		show_log_warning( "Failed to get size for fileID: %d", fileID );
		return 0;
	}

	return size.QuadPart;
}

bool platform_seek_in_file( u32 fileID, FileSeek seek, u64 offset )
{
	DWORD move;

	switch ( seek )
	{
		case FILE_SEEK_START:	move = FILE_BEGIN;		break;
		case FILE_SEEK_CURRENT:	move = FILE_CURRENT;	break;
		case FILE_SEEK_END:		move = FILE_END;		break;
		default:
			show_log_warning( "Function received unknown seek: %d", seek );
			return false;
	}

	LARGE_INTEGER li;

	li.QuadPart = offset;

	DWORD result = SetFilePointer( platformData.allOpenFiles[ fileID ], li.LowPart, &li.HighPart, move );

	if ( result == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR )
	{
		show_log_warning( "Failed to seek pointer in fileID: %d", fileID );
		return false;
	}

	return true;
}

u8 *platform_read_whole_file( u32 fileID, MemoryArena *arena, bool addNullTerminator )
{
	u64 fileSize = platform_get_file_size( fileID );

	if ( fileSize == 0 )
		return nullptr;

	u8 *buffer = memory_arena_transient_allocate( arena, fileSize + addNullTerminator );

	platform_seek_in_file( fileID, FILE_SEEK_START, 0 );

	u64 bytesRead = platform_read_from_file( fileID, buffer, fileSize );

	if ( bytesRead != fileSize )
	{
		memory_arena_transient_free( arena, buffer );
		return nullptr;
	}

	if ( addNullTerminator )
		buffer[ fileSize ] = '\0';

	return buffer;
}

u64 platform_read_from_file( u32 fileID, void *buffer, u64 size )
{
	HANDLE file = platformData.allOpenFiles[ fileID ];

	massert( size < UINT32_MAX );

	DWORD bytesRead;

	if ( !ReadFile( file, buffer, (u32)size, &bytesRead, 0 ) )
	{
		show_log_warning( "Failed to read from fileID: %d", fileID );
		return 0;
	}

	return bytesRead;
}

u64 platform_write_to_file( u32 fileID, void *buffer, u64 size )
{
	HANDLE file = platformData.allOpenFiles[ fileID ];

	massert( size < UINT32_MAX );

	DWORD bytesWritten;

	BOOL result = WriteFile( file, buffer, (u32)size, &bytesWritten, 0 );

	if ( !result || size != bytesWritten )
	{
		show_log_warning( "Failed to write to fileID: %d", fileID );
		return 0;
	}

	return bytesWritten;
}

[[nodiscard]] inline bool platform_file_exists( const char *path )
{
	DWORD attributes = GetFileAttributes( path );
	return ( attributes != INVALID_FILE_ATTRIBUTES && !( attributes & FILE_ATTRIBUTE_DIRECTORY ) );
}

inline bool platform_delete_file( const char *path )
{
	return DeleteFile( path ) != 0;
}

[[nodiscard]] u64 platform_last_edit_timestamp( const char *path )
{
	HANDLE file = CreateFile( path, GENERIC_READ, FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0 );

	if ( file == INVALID_HANDLE_VALUE )
	{
		return 0;
	}

	FILETIME writeTime;
	if ( !GetFileTime( file, 0, 0, &writeTime ) )
	{
		CloseHandle( file );
		return 0;
	}

	CloseHandle( file );

	return ULARGE_INTEGER{ writeTime.dwLowDateTime, writeTime.dwHighDateTime }.QuadPart;
}

inline void platform_copy_file( const char *from, const char *to )
{
	while ( !CopyFile( from, to, false ) )
	{
		Sleep( 10 );
	}
}

bool platform_logger_initialisation()
{
	platform_create_directory( "logs" );

	// Check there arn't too many logs (if there is, delete the oldest one)
	WIN32_FIND_DATA data;
	HANDLE hFind = FindFirstFile( "logs/*", &data );

	if ( hFind != INVALID_HANDLE_VALUE )
	{
		u32 fileCount = 0;
		u64 oldestCreationDate = INVALID_INDEX_UINT_64;
		char oldestPath[ 2048 ];

		do
		{
			if ( !( data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
			{
				u64 creationDate = ULARGE_INTEGER{ data.ftCreationTime.dwLowDateTime, data.ftCreationTime.dwHighDateTime }.QuadPart;

				if ( creationDate < oldestCreationDate )
				{
					oldestCreationDate = creationDate;
					string_utf8_copy( oldestPath, data.cFileName );
				}

				++fileCount;
			}
		} while ( FindNextFile( hFind, &data ) != 0 );

		FindClose( hFind );

		if ( fileCount >= MAX_LOG_FILES )
		{
			char path[ 2048 ];
			string_utf8_format( path, "logs/%s", oldestPath );
			platform_delete_file( path );
		}
	}

	// Create the new log file
	Date date = platform_get_local_date();

	char path[ 2048 ];
	string_utf8_format( path, "logs/log_file__%d_%d_%d__%d_%d_%d.txt", date.day, date.month, date.year, date.hour, date.minute, date.second );

	HANDLE file = CreateFile( path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, 0 );

	if ( file == INVALID_HANDLE_VALUE )
	{
		show_log_warning( "Failed to open log file: %s", path );
		return false;
	}

	platformData.loggerFile = file;

	return true;
}

void platform_logger_message( const char *message, ... )
{
	char buffer[ MAX_DEBUG_MESSAGE ];
	va_list args;
	va_start( args, message );
	vsnprintf( buffer, ARRAY_LENGTH( buffer ), message, args );
	va_end( args );

	DWORD bytesWritten;
	WriteFile( platformData.loggerFile, buffer, (DWORD)string_utf8_bytes( buffer ) - 1, &bytesWritten, 0 );
	FlushFileBuffers( platformData.loggerFile );
}

void platform_logger_message( const char *message, va_list args )
{
	char buffer[ MAX_DEBUG_MESSAGE ];
	vsnprintf( buffer, ARRAY_LENGTH( buffer ), message, args );

	DWORD bytesWritten;
	WriteFile( platformData.loggerFile, buffer, (DWORD)string_utf8_bytes( buffer ) - 1, &bytesWritten, 0 );
	FlushFileBuffers( platformData.loggerFile );
}

void platform_logger_close()
{
	CloseHandle( platformData.loggerFile );
	platformData.loggerFile = INVALID_HANDLE_VALUE;
}

// ---------------------------------------------------
// System
inline bool platform_update()
{
	return ( platformData.flags & PLATFORM_DATA_RUNNING );
}

bool platform_initialise()
{
	platformData.hInstance = GetModuleHandle( 0 );
	platformData.allOpenFiles.clear();
	platformData.freeOpenFileIDs.clear();
	platformData.activeTimer = 0;
	platformData.tickFrequency = platform_get_tick_frequency();

	// Files
	platformData.allOpenFiles.set_full();
	for ( i32 i = static_cast<i32>( platformData.freeOpenFileIDs.capacity() ) - 1; i >= 0; --i )
		platformData.freeOpenFileIDs.add( i );

	// Enable virtual terminal processing to use cescape sequences to alter olours of the logging
	HANDLE consoleHandle = GetStdHandle( STD_OUTPUT_HANDLE );
	if ( consoleHandle != INVALID_HANDLE_VALUE )
	{
		DWORD dwMode = 0;
		if ( GetConsoleMode( consoleHandle, &dwMode ) )
		{
			dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

			if ( SetConsoleMode( consoleHandle, dwMode ) )
			{
				platformData.colouredConsole = true;
			}
			else
			{
				show_debug_warning( "SetConsoleMode failed" );
			}
		}
		else
		{
			show_debug_warning( "GetConsoleMode failed" );
		}
	}
	else
	{
		show_debug_warning( "GetStdHandle failed" );
	}

	platformData.flags |= PLATFORM_DATA_RUNNING;

	return true;
}

void platform_shutdown()
{
	platformData.flags &= ~PLATFORM_DATA_RUNNING;
}

void platform_cleanup()
{
}