
#pragma once

#define KB( x ) 									( (u64)1024 * x )
#define MB( x ) 									( (u64)1024 * KB( x ) )
#define GB( x ) 									( (u64)1024 * MB( x ) )

#define BIT( b ) 									( ( 1u << ( b ) ) )
#define BIT_SET( field, bit, boolean ) 				do { if ( boolean ) { ( field ) |= ( bit ); } else { ( field ) &= ~( bit ); } } while( 0 )
#define BIT_ASSIGN( field, bit, otherField )		( ( field ) = ( ( field ) & ~( bit ) ) | ( ( otherField ) & ( bit ) ) )
#define BIT_ADD( field, bit ) 						( ( field ) |= ( bit ) )
#define BIT_REM( field, bit ) 						( ( field ) &= ~( bit ) )
#define BIT_ANY( field, bits )						( ( ( field ) & ( bits ) ) != 0 )
#define BIT_ALL( field, bits )						( ( ( field ) & ( bits ) ) == ( bits ) )

#define ARRAY_LENGTH( arr ) 						( sizeof( arr ) / sizeof( arr[ 0 ] ) )

#define MEMORY_ALIGNMENT							sizeof( u64 )

#define INVALID_INDEX_UINT_16 						( UINT16_MAX )
#define INVALID_INDEX_UINT_32 						( UINT32_MAX )
#define INVALID_INDEX_UINT_64						( UINT64_MAX )

#define PRIME_NUMBER_FILE							"prime_numbers.bin"

#define INVALID_FILE_INDEX							( INVALID_INDEX_UINT_32 )
#define MAX_OPEN_FILES								( 8 )
#define MAX_DIALOG_FILENAME							( 512 )
#define MAX_WORKING_DIRECTORY_PATH					( 512 )
#define MAX_CONSOLE_INPUT							( 4096 )
#define MAX_LOG_FILES								( 16 )
#define MAX_DEBUG_MESSAGE							( 4096 )
#define MAX_CONVERT_TO_STRING_DIGITS				( 32 )

using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using f32 = float;
using f64 = double;