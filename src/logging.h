
#pragma once

#define CONSOLE_COLOR_BLACK					"\x1b[30m"
#define CONSOLE_COLOR_RED					"\x1b[31m"
#define CONSOLE_COLOR_GREEN					"\x1b[32m"
#define CONSOLE_COLOR_YELLOW				"\x1b[33m"
#define CONSOLE_COLOR_BLUE					"\x1b[34m"
#define CONSOLE_COLOR_MAGENTA				"\x1b[35m"
#define CONSOLE_COLOR_CYAN					"\x1b[36m"
#define CONSOLE_COLOR_WHITE					"\x1b[37m"
#define CONSOLE_COLOR_BRIGHT_BLACK			"\x1b[90m"
#define CONSOLE_COLOR_BRIGHT_RED			"\x1b[91m"
#define CONSOLE_COLOR_BRIGHT_GREEN			"\x1b[92m"
#define CONSOLE_COLOR_BRIGHT_YELLOW			"\x1b[93m"
#define CONSOLE_COLOR_BRIGHT_BLUE			"\x1b[94m"
#define CONSOLE_COLOR_BRIGHT_MAGENTA		"\x1b[95m"
#define CONSOLE_COLOR_BRIGHT_CYAN			"\x1b[96m"
#define CONSOLE_COLOR_BRIGHT_WHITE			"\x1b[97m"

#define CONSOLE_BG_COLOR_BLACK				"\x1b[40m"
#define CONSOLE_BG_COLOR_RED				"\x1b[41m"
#define CONSOLE_BG_COLOR_GREEN				"\x1b[42m"
#define CONSOLE_BG_COLOR_YELLOW				"\x1b[43m"
#define CONSOLE_BG_COLOR_BLUE				"\x1b[44m"
#define CONSOLE_BG_COLOR_MAGENTA			"\x1b[45m"
#define CONSOLE_BG_COLOR_CYAN				"\x1b[46m"
#define CONSOLE_BG_COLOR_WHITE				"\x1b[47m"
#define CONSOLE_BG_COLOR_BRIGHT_BLACK		"\x1b[100m"
#define CONSOLE_BG_COLOR_BRIGHT_RED			"\x1b[101m"
#define CONSOLE_BG_COLOR_BRIGHT_GREEN		"\x1b[102m"
#define CONSOLE_BG_COLOR_BRIGHT_YELLOW		"\x1b[103m"
#define CONSOLE_BG_COLOR_BRIGHT_BLUE		"\x1b[104m"
#define CONSOLE_BG_COLOR_BRIGHT_MAGENTA		"\x1b[105m"
#define CONSOLE_BG_COLOR_BRIGHT_CYAN		"\x1b[106m"
#define CONSOLE_BG_COLOR_BRIGHT_WHITE		"\x1b[107m"

#define LOG_MSG_PREFIX						"[   MSG]: "
#define LOG_INFO_PREFIX						"[  INFO]: "
#define LOG_WARN_PREFIX						"[  WARN]: "
#define LOG_ERROR_PREFIX					"[ ERROR]: "
#define LOG_MSG_PREFIX_COLOURED				"[\x1b[37m   MSG\033[0m]: "
#define LOG_INFO_PREFIX_COLOURED			"[\x1b[94m  INFO\033[0m]: "
#define LOG_WARN_PREFIX_COLOURED			"[\x1b[33m  WARN\033[0m]: "
#define LOG_ERROR_PREFIX_COLOURED			"[\x1b[31m ERROR\033[0m]: "
#define LOG_ASSERT_PREFIX_COLOURED			"[\x1b[31;4;103mASSERT\033[0m]: "

#ifdef DEBUG

#define massert( condition, ... ) do { if ( !( condition ) ) { show_debug_message_function( LOG_ERROR_PREFIX #condition "\n", LOG_ASSERT_PREFIX_COLOURED "\x1b[33:41m" #condition "\033[0m\n" __VA_OPT__(" : ") __VA_ARGS__ ); __debugbreak(); } } while (0)
#define massert_static( condition, ... ) do { static_assert( condition __VA_OPT__(,) __VA_ARGS__ ); } while (0)
#define show_debug_message( message, ... ) do { show_debug_message_function( LOG_MSG_PREFIX message "\n", LOG_MSG_PREFIX_COLOURED message "\n" __VA_OPT__(,) __VA_ARGS__ ); } while (0)
#define show_debug_info( message, ... ) do { show_debug_message_function( LOG_INFO_PREFIX message "\n", LOG_INFO_PREFIX_COLOURED "\x1b[96m" message "\033[0m\n" __VA_OPT__(,) __VA_ARGS__ ); } while (0)
#define show_debug_warning( message, ... ) do { show_debug_message_function( LOG_WARN_PREFIX message "\n", LOG_WARN_PREFIX_COLOURED "\x1b[93m" message "\033[0m\n" __VA_OPT__(,) __VA_ARGS__ ); } while (0)
#define show_debug_error( message, ... ) do { show_debug_message_function( LOG_ERROR_PREFIX message "\n", LOG_ERROR_PREFIX_COLOURED "\x1b[91m" message "\033[0m\n" __VA_OPT__(,) __VA_ARGS__ ); } while (0)

#else

#define massert( condition, ... ) do {} while (0)
#define massert_static( condition, ... ) do {} while (0)
#define show_debug_message( message, ... ) do {} while (0)
#define show_debug_info( message, ... ) do {} while (0)
#define show_debug_warning( message, ... ) do {} while (0)
#define show_debug_error( message, ... ) do {} while (0)

#endif

#define show_message( message, ... ) do { show_debug_message_function( message "\n", message "\n" __VA_OPT__(,) __VA_ARGS__ ); } while (0)
#define show_message_same_line( message, ... ) do { show_debug_message_function( message, message __VA_OPT__(,) __VA_ARGS__ ); } while (0)

#define show_log_message( message, ... ) do { show_debug_message_function( LOG_MSG_PREFIX message "\n", LOG_MSG_PREFIX_COLOURED message "\n" __VA_OPT__(,) __VA_ARGS__ ); } while (0)
#define show_log_info( message, ... ) do { show_debug_message_function( LOG_INFO_PREFIX message "\n", LOG_INFO_PREFIX_COLOURED "\x1b[96m" message "\033[0m\n" __VA_OPT__(,) __VA_ARGS__ ); } while (0)
#define show_log_warning( message, ... ) do { show_debug_message_function( LOG_WARN_PREFIX message "\n", LOG_WARN_PREFIX_COLOURED "\x1b[93m" message "\033[0m\n" __VA_OPT__(,) __VA_ARGS__ ); } while (0)
#define show_log_error( message, ... ) do { show_debug_message_function( LOG_ERROR_PREFIX message "\n", LOG_ERROR_PREFIX_COLOURED "\x1b[101m" message "\033[0m\n" __VA_OPT__(,) __VA_ARGS__ ); } while (0)

#define verbose_log_message( message, ... ) do { if ( !( program->flags & PROGRAM_FLAG_VERBOSE ) ) break; show_debug_message_function( LOG_MSG_PREFIX message "\n", LOG_MSG_PREFIX_COLOURED message "\n" __VA_OPT__(,) __VA_ARGS__ ); } while (0)
#define verbose_log_info( message, ... ) do { if ( !( program->flags & PROGRAM_FLAG_VERBOSE ) ) break; show_debug_message_function( LOG_INFO_PREFIX message "\n", LOG_INFO_PREFIX_COLOURED "\x1b[96m" message "\033[0m\n" __VA_OPT__(,) __VA_ARGS__ ); } while (0)
#define verbose_log_warning( message, ... ) do { if ( !( program->flags & PROGRAM_FLAG_VERBOSE ) ) break; show_debug_message_function( LOG_WARN_PREFIX message "\n", LOG_WARN_PREFIX_COLOURED "\x1b[93m" message "\033[0m\n" __VA_OPT__(,) __VA_ARGS__ ); } while (0)
#define verbose_log_error( message, ... ) do { if ( !( program->flags & PROGRAM_FLAG_VERBOSE ) ) break; show_debug_message_function( LOG_ERROR_PREFIX message "\n", LOG_ERROR_PREFIX_COLOURED "\x1b[101m" message "\033[0m\n" __VA_OPT__(,) __VA_ARGS__ ); } while (0)

void show_debug_message_function( const char *message, const char *messageColoured, ... );
void show_debug_message_function_ext( const char *message, const char *messageColoured, va_list args );