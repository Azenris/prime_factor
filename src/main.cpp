
// System includes
#include <stdarg.h>
#include <cmath>
#include <vector>
#include <algorithm>

// Includes
#include "defines.h"
#include "logging.h"
#include "memory_arena.h"
#include "array.h"
#include "strings.h"
#include "map.h"
#include "utility.h"
#include "platform.h"
#include "result_code.h"

using ProgramFlags = u32;
enum PROGRAM_FLAGS : ProgramFlags
{
	PROGRAM_FLAG_VERBOSE = BIT( 0 ),
};
constexpr const ProgramFlags DEFAULT_PROGRAM_FLAGS = 0;

struct Program
{
	ProgramFlags flags;
	MemoryArena memoryArena;
	char workingDirectory[ MAX_WORKING_DIRECTORY_PATH ];
	char consoleInput[ MAX_CONSOLE_INPUT ];
};

Program *program;

// PLATFORM
#ifdef PLATFORM_WINDOWS
#	include "platform_windows.cpp"
#else
#	warning No Platform selected.
#endif

// Implements
#include "utility.cpp"

// -------------------------------------------------------------------------

void timer_start()
{
	platform_timer_start();
	show_log_info( "Timer Started" );
}

void timer_stop()
{
	u64 microSeconds = platform_timer_stop();
	u64 seconds = microSeconds / 1000000;

	show_log_info( "Timer Stopped: %llu seconds (%llu microseconds)", seconds, microSeconds );
}

const char *get_input()
{
	fgets( program->consoleInput, sizeof( program->consoleInput ), stdin );
	return program->consoleInput;
}

// -------------------------------------------------------------------------

static RESULT_CODE usage_message( RESULT_CODE code )
{
	show_log_warning( "\nERROR_CODE: %s\n", error_code_string( code ) );
	show_log_info( ":: USAGE ::" );
	show_log_message( "Expects %s <commands>", "prime_factor.exe" );
	show_log_message( "Eg. %s %s\n", "prime_factor.exe", "-v -memory 65535 65535" );
	show_log_info( "COMMANDS" );
	show_log_message( "[-v]                         EG. -v                               (enable verbose outputs)" );
	show_log_message( "[-ra]                        EG. -ra                              (outputs received arguments)" );
	show_log_message( "[-wd] <path>                 EG. -wd TEMP\\                        (override the default working directory)" );
	show_log_message( "[-memory] <bytes> <bytes>    EG. -memory 1024 2048                (specify memory allocation - perma, transient)" );

	return code;
}

RESULT_CODE initialise( int argc, const char *argv[] )
{
	if ( !platform_logger_initialisation() )
		return RESULT_CODE_FAILED_TO_OPEN_LOGGER_FILE;

	platform_initialise();

	struct Options
	{
		u64 permanentSize = MB( 2 );
		u64 transientSize = MB( 2 );
		const char *workingDirectory = nullptr;
		bool verbose = false;
	};

	Options options;

	// Exe Argument Commands
	{
		Map<const char *, RESULT_CODE(*)( Options &options, int &, int, const char ** ), 256> commands;

		commands.insert( "-v", [] ( Options &options, int &index, int argc, const char *argv[] )
			{
				options.verbose = true;

				return RESULT_CODE_SUCCESS;
			} );

		commands.insert( "-ra", [] ( Options &options, int &index, int argc, const char *argv[] )
			{
				show_log_message( "Arguments received [#%d]", argc );
				for ( int i = 0; i < argc; ++i )
					show_log_info( " [%d] = %s", i, argv[ i ] );

				return RESULT_CODE_SUCCESS;
			} );

		commands.insert( "-wd", [] ( Options &options, int &index, int argc, const char *argv[] )
			{
				options.workingDirectory = argv[ ++index ];

				return RESULT_CODE_SUCCESS;
			} );

		commands.insert( "-memory", [] ( Options &options, int &index, int argc, const char *argv[] )
			{
				options.permanentSize = convert_to_u64( argv[ ++index ] );
				options.transientSize = convert_to_u64( argv[ ++index ] );

				return RESULT_CODE_SUCCESS;
			} );

		// Process the option commands
		for ( int i = 1; i < argc; ++i )
		{
			auto f = commands.find( argv[ i ] );

			if ( f )
			{
				RESULT_CODE code = f->value( options, i, argc, &argv[ 0 ] );
				if ( code != RESULT_CODE_SUCCESS )
					return code;
			}
			else
			{
				show_log_warning( "Unknown command: %s", argv[ i ] );
				return RESULT_CODE_UNKNOWN_OPTIONAL_COMMAND;
			}
		}
	}

	// Memory
	MemoryArena memory;

	if ( !memory_arena_initialise( &memory, options.permanentSize, options.transientSize, true ) )
	{
		show_log_error( "Failed to initialise memory arena." );
		return RESULT_CODE_FAILED_TO_INITIALISE_MEMORY_ARENA;
	}

	// Allocate room for the program
	program = memory_arena_permanent_allocate<Program>( &memory );

	program->flags = DEFAULT_PROGRAM_FLAGS;
	if ( options.verbose )
		program->flags |= PROGRAM_FLAG_VERBOSE;

	// Give the memory to the program
	program->memoryArena = memory;

	// Platform
	if ( !platform_initialise() )
	{
		show_log_error( "Failed to initialise the platform." );
		return RESULT_CODE_FAILED_TO_INITIALISE_PLATFORM;
	}
	else
	{
		verbose_log_message( "Platform initialised." );
	}

	// Working Directory
	platform_set_current_directory( options.workingDirectory ? options.workingDirectory : platform_get_current_directory( &program->memoryArena ) );

	return RESULT_CODE_SUCCESS;
}

int shutdown()
{
	platform_cleanup();

	// Unbind the memory from program before freeing (since it contains the program itself)
	MemoryArena memory = program->memoryArena;
	program->memoryArena.flags &= ~MEMORY_FLAGS_INITIALISED;
	memory_arena_free( &memory );

	platform_logger_close();

	return RESULT_CODE_SUCCESS;
}

u64 generate_prime_numbers( u64 to )
{
	u64 primeNumbersFound = 0;

	u32 file = platform_open_file( PRIME_NUMBER_FILE, FILE_OPTION_CREATE | FILE_OPTION_WRITE | FILE_OPTION_CLEAR );
	if ( file == INVALID_FILE_INDEX )
	{
		show_log_warning( "Failed to create or open file: %s", PRIME_NUMBER_FILE );
		return 0;
	}

	// Write a blank entry at the start for the total number of primes found
	u64 value = 0;
	platform_write_to_file( file, &value, sizeof( u64 ) );

	value = 2;
	platform_write_to_file( file, &value, sizeof( u64 ) );
	++primeNumbersFound;

	{
		timer_start();

		for ( u64 i = 3; i < to; i += 2 )
		{
			if ( is_prime( i ) )
			{
				platform_write_to_file( file, &i, sizeof( u64 ) );
				++primeNumbersFound;
			}
		}

		timer_stop();
	}

	show_message( "\n%llu Prime Numbers found (0 - %llu).", primeNumbersFound, to );

	// Write the number at the start
	platform_seek_in_file( file, FILE_SEEK_START, 0 );
	platform_write_to_file( file, &primeNumbersFound, sizeof( u64 ) );

	platform_close_file( file );

	return primeNumbersFound;
}

const char *get_positional_ending( u64 value )
{
	switch ( value % 10 )
	{
	case 1: return "st";
	case 2: return "nd";
	case 3: return "rd";
	}

	return "th";
}

// -------------------------------------------------------------------------
void single_prime_finding( u64 &rem, std::vector<u64> &primeFactor, const std::vector<u64> &primesUsed )
{
	u64 primeFactorValue = primesUsed[ 0 ];

	do
	{
		primeFactor.push_back( primeFactorValue );
		rem = rem / primeFactorValue;

	} while ( rem != 1 );
}

void multi_prime_finding( u64 &rem, std::vector<u64> &primeFactor, const std::vector<u64> &possiblePrimesUsed )
{
	if ( rem == 1 )
		return;

	std::vector<u64> primesUsed;

	u64 pos = 0;

	while ( pos < possiblePrimesUsed.size() && possiblePrimesUsed[ pos ] <= rem )
	{
		if ( rem % possiblePrimesUsed[ pos ] == 0 )
			primesUsed.push_back( possiblePrimesUsed[ pos ] );

		++pos;
	}

	if ( primesUsed.size() == 1 )
	{
		single_prime_finding( rem, primeFactor, primesUsed );
	}
	else
	{
		for ( u64 i = 0; i < primesUsed.size(); ++i)
		{
			rem = rem / primesUsed[ i ];
			primeFactor.push_back( primesUsed[ i ] );
		}
	}

	multi_prime_finding( rem, primeFactor, primesUsed );
}

void prime_factorisation()
{
	while ( true )
	{
		u64 minValue = 2;
		u64 maxValue = 0;

		{
			u32 file = platform_open_file( PRIME_NUMBER_FILE, FILE_OPTION_READ );
			if ( file == INVALID_FILE_INDEX )
			{
				show_log_warning( "Generate some prime numbers first." );
				return;
			}

			u64 bytesRead = platform_read_from_file( file, &maxValue, sizeof( u64 ) );
			platform_close_file( file );

			if ( bytesRead != sizeof( u64 ) || maxValue == 0 )
			{
				show_log_warning( "Prime number file corrupted. Regeneerate." );
				return;
			}
		}

		bool invalid = true;
		u64 inputValue = 0;

		// Get the input
		while ( invalid )
		{
			show_message( "\nEnter 0 to exit." );
			show_message_same_line( "Enter a number between %llu and %llu (inclusive):", minValue, maxValue );

			const char *input = get_input();

			if ( !input || input[ 0 ] == '0' || input[ 0 ] == '-' || input[ 0 ] == '\0' )
				return;

			inputValue = convert_to_u64( input );

			invalid = ( inputValue == 0 || inputValue < minValue || inputValue > maxValue );

			if ( invalid )
				show_log_warning( "An error has occured, ensure the input is numeric and within the boundaries." );
		}

		bool even = ( inputValue & 1 ) == 0;
		bool single = true;
		u64 combinedOccurence = 0;

		std::vector<u64> primesUsed;
		std::vector<u64> primeFactor;
		std::vector<u64> occurence;

		primesUsed.reserve( 65536 );
		primeFactor.reserve( 65536 );
		occurence.reserve( 65536 );

		// Process
		show_message( "\nProcessing..." );
		{
			int primeNumber = 0;

			u32 file = platform_open_file( PRIME_NUMBER_FILE, FILE_OPTION_READ );
			if ( file == INVALID_FILE_INDEX )
			{
				show_log_warning( "Failed to open prime numbers file." );
				return;
			}

			// Read the count first
			u64 count;
			platform_read_from_file( file, &count, sizeof( u64 ) );

			timer_start();

			for ( u64 i = 0; i < count; ++i )
			{
				platform_read_from_file( file, &primeNumber, sizeof( u64 ) );

				if ( primeNumber > inputValue )
					break;

				if ( inputValue % primeNumber == 0 )
					primesUsed.push_back( primeNumber );
			}

			platform_close_file( file );

			if ( primesUsed.empty() )
			{
				show_log_warning( "Prime number file may be corrupt. Regenerate." );
				return;
			}

			single = ( primesUsed.size() == 1 );

			if ( single )
			{
				u64 rem = inputValue;

				single_prime_finding( rem, primeFactor, primesUsed );

				combinedOccurence = inputValue / primeFactor[ 0 ];
			}
			else
			{
				u64 jumpValue = primesUsed[ 0 ];

				for ( u64 i = 1, primeCount = primesUsed.size(); i < primeCount; ++i )
				{
					occurence.push_back( inputValue / primesUsed[ i ] );
					jumpValue = jumpValue * primesUsed[ i ];
				}

				combinedOccurence = inputValue / jumpValue;

				u64 rem = inputValue;

				for ( u64 i = 0, primeCount = primesUsed.size(); i < primeCount; ++i )
				{
					rem = rem / primesUsed[ i ];
					primeFactor.push_back( primesUsed[ i ] );
				}

				multi_prime_finding( rem, primeFactor, primesUsed );
			}

			timer_stop();
		}

		// Output Findings
		if ( primesUsed.size() < 1 || primeFactor.size() < 1 )
		{
			show_log_warning( "An error has occured while processing the primes involved." );
			return;
		}

		show_message( "\n> Data:" );
		show_message( "> Input Value: %llu (%s)", inputValue, even ? "even" : "odd" );
		show_message( "> Type: %s (%llu)", single ? "Single" : "Multi", primesUsed.size() );
		show_message_same_line( "> Primes used: %llu", primesUsed[ 0 ] );

		for ( u64 i = 1, primeCount = primesUsed.size(); i < primeCount; ++i )
			show_message_same_line( ", %llu", primesUsed[ i ] );

		if ( single )
		{
			show_message( "\n> Occurence [%llu]: %llu%s", primesUsed[ 0 ], combinedOccurence, get_positional_ending( combinedOccurence ) );
		}
		else
		{
			show_message( "\n> Occurences:" );

			u64 occur = 0;

			for ( u64 i = 0, primeCount = primesUsed.size(); i < primeCount; ++i )
			{
				occur = inputValue / primesUsed[ i ];
				show_message( " > [%llu]: %llu%s", primesUsed[ i ], occur, get_positional_ending( occur ) );
			}

			show_message_same_line( "> Combined occurence [%llu", primesUsed[ 0 ] );

			for ( u64 i = 1, primeCount = primesUsed.size(); i < primeCount; ++i )
				show_message_same_line( ".%llu", primesUsed[ i ] );

			show_message( "]: %llu%s", combinedOccurence, get_positional_ending( combinedOccurence ) );
		}

		show_message_same_line( "> Prime factors [raw find]: %llu", primeFactor[ 0 ] );

		for ( u64 i = 1, primeFactorCount = primeFactor.size(); i < primeFactorCount; ++i )
			show_message_same_line( ".%llu", primeFactor[ i ] );

		std::sort( primeFactor.begin(), primeFactor.end() );

		show_message_same_line( "\n> Prime factors: %llu", primeFactor[ 0 ] );

		for ( u64 i = 1, primeFactorCount = primeFactor.size(); i < primeFactorCount; ++i )
			show_message_same_line( ".%llu", primeFactor[ i ] );

		show_message( "" );
	}
}

// -------------------------------------------------------------------------
// ENTRY
// -------------------------------------------------------------------------
int main( int argc, const char *argv[] )
{
	{
		RESULT_CODE result = initialise( argc, argv );
		if ( result != RESULT_CODE_SUCCESS )
			return usage_message( result );
	}

	verbose_log_message( "Starting..." );

	// -------------------------------------------------------------------------

	while ( platform_update() )
	{
		show_message( "\n:: Prime Stuff\n:: By Azenris\n:: morleyx22@hotmail.com\n:: only 8 byte numbers." );
		show_message_same_line( "] 0: Exit Program.\n] 1: Prime Factorisation.\n] 2: Generate Prime Numbers.\n] Selection: " );

		int inputValue = convert_to_int( get_input() );

		switch ( inputValue )
		{
		case 0:
			platform_shutdown();
			break;

		case 1:
			prime_factorisation();
			break;

		case 2:
			// Generate Prime Numbers
			show_message_same_line( "\nPlease enter the highest number to check: " );

			inputValue = convert_to_int( get_input() );

			if ( inputValue > 0 )
			{
				generate_prime_numbers( inputValue );
			}
			else
			{
				show_log_warning( "An error has occured. Invalid number." );
				platform_shutdown();
			}
			break;
		}

		memory_arena_update( &program->memoryArena );
	}

	// -------------------------------------------------------------------------

	verbose_log_message( "Ending..." );

	return shutdown();
}