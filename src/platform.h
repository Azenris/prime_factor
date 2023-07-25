
#pragma once

void platform_print_error();

struct MemoryArena;

// Timings
[[nodiscard]] inline u64 platform_get_tick_frequency();
[[nodiscard]] inline u64 platform_get_tick_counter();
[[nodiscard]] inline u64 platform_get_cycle_counter();

inline u64 platform_timer_start();
[[nodiscard]] inline u64 platform_timer_stop();

// Date
struct Date
{
	u16 dayOfWeek;							// 0 - 6 (monday, tuesday, wednesday, thursday, friday, saturday, sunday)
	u16 day;								// 1 - 31
	u16 month;								// 1 - 12
	u16 year;								// 
	u16 hour;								// 0 - 23
	u16 minute;								// 0 - 59
	u16 second;								// 0 - 59
	u16 milliSecond;						// 0 - 999
};

[[nodiscard]] Date platform_get_system_date();
[[nodiscard]] Date platform_get_local_date();

// Directory
bool platform_create_directory( const char *directory );
inline void platform_set_current_directory( const char *workingDirectory );
[[nodiscard]] char *platform_get_current_directory( MemoryArena *arena );

// Files
using FileOptions = u32;
enum FILE_OPTION : FileOptions
{
	FILE_OPTION_READ	= BIT( 0 ),			// open the file for reading
	FILE_OPTION_WRITE	= BIT( 1 ),			// open the file for writing
	FILE_OPTION_CREATE	= BIT( 2 ),			// create the file if it doesnt exist
	FILE_OPTION_APPEND	= BIT( 3 ),			// start at the end of the file
	FILE_OPTION_CLEAR	= BIT( 4 ),			// the file is cleared
};

using FileSeek = u32;
enum FILE_SEEK : FileSeek
{
	FILE_SEEK_START,
	FILE_SEEK_CURRENT,
	FILE_SEEK_END,
};

[[nodiscard]] const char *platform_get_relative_filename( const char *path );
[[nodiscard]] u8 *platform_read_file( const char *path, u64 *fileSize, bool addNullTerminator, MemoryArena *arena );
u64 platform_write_file( const char *path, const u8 *buffer, u64 size, bool append );
u32 platform_open_file( const char *path, FileOptions options );
void platform_close_file( u32 fileID );
u64 platform_get_file_size( u32 fileID );
bool platform_seek_in_file( u32 fileID, FileSeek seek, u64 offset );
u8 *platform_read_whole_file( u32 fileID, MemoryArena *arena, bool addNullTerminator );
u64 platform_read_from_file( u32 fileID, void *buffer, u64 size );
u64 platform_write_to_file( u32 fileID, void *buffer, u64 size );
[[nodiscard]] inline bool platform_file_exists( const char *path );
inline bool platform_delete_file( const char *path );
[[nodiscard]] u64 platform_last_edit_timestamp( const char *path );
inline void platform_copy_file( const char *from, const char *to );

// Logger File
bool platform_logger_initialisation();
void platform_logger_message( const char *message, ... );
void platform_logger_message( const char *message, va_list args );
void platform_logger_close();

// System
bool platform_initialise();
void platform_shutdown();
void platform_cleanup();