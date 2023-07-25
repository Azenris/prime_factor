
#pragma once

#define ID_NAME( id )	id, #id

[[nodiscard ]] inline bool is_prime( u64 value )
{
	u64 sqrRoot = (u64)sqrt( value );

	for ( u64 i = 3; i <= sqrRoot; i += 2 )
		if ( value % i == 0 )
			return false;

	return true;
}

[[nodiscard ]] inline bool bytes_compare( const void *rhs, const void *lhs, u64 bytes )
{
	return memcmp( rhs, lhs, bytes );
}

[[nodiscard]] inline i32 compare_value( const char *lhs, const char *rhs );

[[nodiscard]] constexpr inline i32 compare_value( u16 lhs, u16 rhs )
{
	return lhs - rhs;
}

[[nodiscard]] constexpr inline i32 compare_value( u32 lhs, u32 rhs )
{
	return lhs - rhs;
}

[[nodiscard]] constexpr inline i32 compare_value( u64 lhs, u64 rhs )
{
	return lhs > rhs ? 1 : ( lhs < rhs ? - 1 : 0 );
}

[[nodiscard]] constexpr inline i32 compare_value( i16 lhs, i16 rhs )
{
	return lhs - rhs;
}

[[nodiscard]] constexpr inline i32 compare_value( i32 lhs, i32 rhs )
{
	return lhs - rhs;
}

[[nodiscard]] constexpr inline i32 compare_value( i64 lhs, i64 rhs )
{
	return lhs > rhs ? 1 : ( lhs < rhs ? - 1 : 0 );
}

[[nodiscard]] inline bool time_alternate( f32 globalTime, f32 time )
{
	if ( time <= 0 ) return 0;
	return ( static_cast<i32>( globalTime / time ) & 1 ) == 0;
}

inline void read_data( void *dst, u8 *&src, u64 size, u8 *srcBarrier )
{
	massert( src + size <= srcBarrier );
	memcpy( dst, src, size );
	src += size;
}

inline void write_data( u8 *&dst, const void *src, u64 size, u8 *dstBarrier )
{
	massert( dst + size <= dstBarrier );
	memcpy( dst, src, size );
	dst += size;
}

[[nodiscard]] u64 convert_to_u64( const char *input, const char **output = nullptr );

[[nodiscard]] inline u32 convert_to_u32( const char *input, const char **output = nullptr )
{
	return static_cast<u32>( convert_to_u64( input, output ) );
}

[[nodiscard]] inline u16 convert_to_u16( const char *input, const char **output = nullptr )
{
	return static_cast<u16>( convert_to_u64( input, output ) );
}

[[nodiscard]] inline u8 convert_to_u8( const char *input, const char **output = nullptr )
{
	return static_cast<u8>( convert_to_u64( input, output ) );
}

[[nodiscard]] i64 convert_to_i64( const char *input, const char **output = nullptr );

[[nodiscard]] inline i32 convert_to_i32( const char *input, const char **output = nullptr )
{
	return static_cast<i32>( convert_to_i64( input, output ) );
}

[[nodiscard]] inline i16 convert_to_i16( const char *input, const char **output = nullptr )
{
	return static_cast<i16>( convert_to_i64( input, output ) );
}

[[nodiscard]] inline i8 convert_to_i8( const char *input, const char **output = nullptr )
{
	return static_cast<i8>( convert_to_i64( input, output ) );
}

[[nodiscard]] inline i32 convert_to_int( const char *input, const char **output = nullptr )
{
	return convert_to_i32( input, output );
}

[[nodiscard]] f32 convert_to_float( const char *input, const char **output = nullptr );
[[nodiscard]] bool convert_to_bool( const char *input, const char **output = nullptr );
[[nodiscard]] bool is_floating_point( const char *input );

u64 convert_to_string( char *dest, u64 destSize, u64 value, i32 radix = 10, i32 trailing = 0 );

inline u64 convert_to_string( char *dest, u64 destSize, u32 value, i32 radix = 10, i32 trailing = 0 )
{
	return convert_to_string( dest, destSize, static_cast<u64>( value ), radix, trailing );
}

inline u64 convert_to_string( char *dest, u64 destSize, u16 value, i32 radix = 10, i32 trailing = 0 )
{
	return convert_to_string( dest, destSize, static_cast<u64>( value ), radix, trailing );
}

inline u64 convert_to_string( char *dest, u64 destSize, u8 value, i32 radix = 10, i32 trailing = 0 )
{
	return convert_to_string( dest, destSize, static_cast<u64>( value ), radix, trailing );
}

u64 convert_to_string( char *dest, u64 destSize, i64 value, i32 radix = 10, i32 trailing = 0 );

inline u64 convert_to_string( char *dest, u64 destSize, i32 value, i32 radix = 10, i32 trailing = 0 )
{
	return convert_to_string( dest, destSize, static_cast<i64>( value ), radix, trailing );
}

inline u64 convert_to_string( char *dest, u64 destSize, i16 value, i32 radix = 10, i32 trailing = 0 )
{
	return convert_to_string( dest, destSize, static_cast<i64>( value ), radix, trailing );
}

inline u64 convert_to_string( char *dest, u64 destSize, i8 value, i32 radix = 10, i32 trailing = 0 )
{
	return convert_to_string( dest, destSize, static_cast<i64>( value ), radix, trailing );
}

u64 convert_to_string( char *dest, u64 destSize, f32 value, i32 fracDigits = 2 );

inline u64 convert_to_string( char *dest, u64 destSize, bool value );

[[nodiscard]] const char *convert_to_string( u64 value, i32 radix = 10, i32 trailing = 0 );
[[nodiscard]] const char *convert_to_string( u32 value, i32 radix = 10, i32 trailing = 0 );
[[nodiscard]] const char *convert_to_string( u16 value, i32 radix = 10, i32 trailing = 0 );
[[nodiscard]] const char *convert_to_string( u8 value, i32 radix = 10, i32 trailing = 0 );

[[nodiscard]] const char *convert_to_string( i64 value, i32 radix = 10, i32 trailing = 0 );
[[nodiscard]] const char *convert_to_string( i32 value, i32 radix = 10, i32 trailing = 0 );
[[nodiscard]] const char *convert_to_string( i16 value, i32 radix = 10, i32 trailing = 0 );
[[nodiscard]] const char *convert_to_string( i8 value, i32 radix = 10, i32 trailing = 0 );

[[nodiscard]] const char *convert_to_string( f32 value, i32 fracDigits );
[[nodiscard]] const char *convert_to_string( bool value );