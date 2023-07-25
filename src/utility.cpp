
[[nodiscard]] i32 compare_value( const char *lhs, const char *rhs )
{
	return string_utf8_compare_value( lhs, rhs );
}

[[nodiscard]] u64 convert_to_u64( const char *input, const char **output )
{
	const char *start = input;

	// remove leading whitespace
	while ( *input == ' ' )
		++input;

	// check it's a number or invalid input
	if ( ( *input < '0' || *input > '9' ) && *input != '+' )
	{
		show_log_warning( "Invalid data. Not an u64." );
		if ( output )
			*output = start;
		return 0;
	}

	u64 base = 0;

	bool overflow = false;

	while ( ( *input >= '0' && *input <= '9' ) )
	{
		overflow = overflow || base > ( SIZE_MAX / 10 ) || ( base == ( SIZE_MAX / 10 ) && ( *input - '0' ) > 7 );

		base = 10 * base + ( *input++ - '0' );
	}

	if ( output )
		*output = input;

	return !overflow ? base : SIZE_MAX;
}

[[nodiscard]] i64 convert_to_i64( const char *input, const char **output )
{
	const char *start = input;

	// remove leading whitespace
	while ( *input == ' ' )
		++input;

	// check it's a number or invalid input
	if ( ( *input < '0' || *input > '9' ) && *input != '-' && *input != '+' )
	{
		show_log_warning( "Invalid data. Not an i64." );
		if ( output )
			*output = start;
		return 0;
	}

	i64 sign = 1;
	i64 base = 0;

	// sign
	if ( *input == '-' || *input == '+' )
		sign = 1 - 2 * ( *input++ == '-' );

	bool overflow = false;

	while ( ( *input >= '0' && *input <= '9' ) )
	{
		overflow = overflow || base > ( INT_MAX / 10 ) || ( base == ( INT_MAX / 10 ) && ( *input - '0' ) > 7 );

		base = 10 * base + ( *input++ - '0' );
	}

	if ( output )
		*output = input;

	return !overflow ? ( base * sign ) : ( sign >= 0 ? INT_MAX : INT_MIN );
}

[[nodiscard]] f32 convert_to_float( const char *input, const char **output )
{
	const char *start = input;

	// remove leading whitespace
	while ( *input == ' ' )
		++input;

	f32 sign = 1.0f;
	f32 base = 0.0f;
	bool overflow = false;
	bool foundPoint = false;
	char c = *input;

	// check it's a number or invalid input
	if ( ( c < '0' || c > '9' ) && c != '.' && c != '-' && c != '+' )
	{
		show_log_warning( "Invalid data. Not a f32: %s", start );
		if ( output )
			*output = start;
		return 0.f;
	}

	// sign
	if ( c == '-' || c == '+' )
		sign = 1.0f - 2.0f * ( *input++ == '-' );

	// starting . :eg: .5
	if ( *input == '.' )
	{
		++input;
		foundPoint = true;
	}

	c = *input++;

	while ( ( c >= '0' && c <= '9' ) || c == '.' )
	{
		if ( c == '.' )
		{
			c = *input++;

			if ( foundPoint )
			{
				show_log_warning( "Invalid data. Not a f32: %s", start );
				return 0.f;
			}
			foundPoint = true;
			continue;
		}

		if ( foundPoint )
			sign /= 10.0f;

		overflow = overflow || base > ( FLT_MAX / 10.0f ) || ( base == ( FLT_MAX / 10.0f ) && ( c - '0' ) > 7 );

		base = 10.0f * base + ( c - '0' );

		c = *input++;
	}

	if ( output )
		*output = input - 1;

	return !overflow ? ( base * sign ) : ( sign >= 0 ? FLT_MAX : -FLT_MAX );
}

[[nodiscard]] bool convert_to_bool( const char *input, const char **output )
{
	const char *start = input;

	// remove leading whitespace
	while ( *input == ' ' )
		++input;

	const char *trueStr = "true";
	const char *falseStr = "false";
	bool value = ( ascii_char_lower( *input ) == *trueStr );
	const char *checkStr = value ? trueStr : falseStr;

	while ( *checkStr )
	{
		// If a character is different, return false, but don't advance output
		if ( ascii_char_lower( *input++ ) != *checkStr++ )
		{
			if ( output )
				*output = start;
			return false;
		}
	}

	// Advance the output if required
	if ( output )
		*output = input;

	return value;
}

[[nodiscard]] bool is_floating_point( const char *input )
{
	// remove leading whitespace
	while ( *input == ' ' )
		++input;

	// ignore leading +/-
	if ( *input == '-' || *input == '+' )
		++input;

	// search for non number
	while ( *input >= '0' && *input <= '9' )
		++input;

	// if it ended with a . , its a floating point number
	return *input == '.';
}

u64 convert_to_string( char *dest, u64 destSize, u64 value, i32 radix, i32 trailing )
{
	massert( radix <= 256 );
	massert( trailing <= 32 );

	char tmp[ 64 ];
	char *tp = tmp;

	char i;

	while ( value || tp == tmp )
	{
		i = static_cast<char>( value % radix );
		value /= radix;

		if ( i < 10 )
			*tp++ = i + '0';
		else
			*tp++ = i + 'a' - 10;
	}

	u64 len = tp - tmp;

	// Check if trailing 0's are needed
	while ( len < trailing )
	{
		*tp++ = '0';
		++len;
	}

	massert( destSize + 1 >= len );

	while ( tp > tmp )
		*dest++ = *--tp;

	*dest = '\0';

	// Digits of string
	return len;
}

u64 convert_to_string( char *dest, u64 destSize, i64 value, i32 radix, i32 trailing )
{
	massert( radix <= 256 );
	massert( trailing <= 32 );

	char tmp[ 64 ];
	char *tp = tmp;

	char i;
	u64 v;

	i64 sign = ( radix == 10 && value < 0 );
	if ( sign )
		v = -value;
	else
		v = static_cast<u64>( value );

	while ( v || tp == tmp )
	{
		i = static_cast<char>( v % radix );
		v /= radix;

		if ( i < 10 )
			*tp++ = i + '0';
		else
			*tp++ = i + 'a' - 10;
	}

	u64 len = tp - tmp;

	// Check if trailing 0's are needed
	while ( len < trailing )
	{
		*tp++ = '0';
		++len;
	}

	if ( sign )
	{
		*dest++ = '-';
		++len;
	}

	massert( destSize + 1 >= len );

	while ( tp > tmp )
		*dest++ = *--tp;

	*dest = '\0';

	// Digits of string
	return len;
}

u64 convert_to_string( char *dest, u64 destSize, f32 value, i32 fracDigits )
{
	char *start = dest;
	i32 integer = static_cast<i32>( value );
	f32 fraction = value - static_cast<f32>( integer );

	u64 size = convert_to_string( dest, destSize, integer, 10 );

	dest += size;

	if ( fracDigits != 0 )
	{
		*dest++ = '.';

		i32 fd = abs( fracDigits );

		u64 fracSize = convert_to_string( dest, destSize - ( size + 1 ), static_cast<i32>( fraction * pow( 10, fd ) ), 10, fd );
		dest += fracSize;

		// Negative fracDigits means to cut trailing zeros (2.0000000 > 2.0)
		if ( fracDigits < 0 )
		{
			--dest;
			while ( *dest == '0' )
				--dest;
			if ( *dest == '.' )
				++dest;
			*++dest = '\0';
		}
	}

	return dest - start;
}

u64 convert_to_string( char *dest, u64 destSize, bool value )
{
	return string_utf8_copy( dest, destSize, value ? "true" : "false" );
}

[[nodiscard]] const char *convert_to_string( u8 value, i32 radix, i32 trailing )
{
	char *text = (char *)memory_arena_transient_allocate( &program->memoryArena, MAX_CONVERT_TO_STRING_DIGITS );
	convert_to_string( text, MAX_CONVERT_TO_STRING_DIGITS, value, radix, trailing );
	text = (char *)memory_arena_transient_reallocate( &program->memoryArena, text, string_utf8_bytes( text ) );
	return text;
}

[[nodiscard]] const char *convert_to_string( u16 value, i32 radix, i32 trailing )
{
	char *text = (char *)memory_arena_transient_allocate( &program->memoryArena, MAX_CONVERT_TO_STRING_DIGITS );
	convert_to_string( text, MAX_CONVERT_TO_STRING_DIGITS, value, radix, trailing );
	text = (char *)memory_arena_transient_reallocate( &program->memoryArena, text, string_utf8_bytes( text ) );
	return text;
}

[[nodiscard]] const char *convert_to_string( u32 value, i32 radix, i32 trailing )
{
	char *text = (char *)memory_arena_transient_allocate( &program->memoryArena, MAX_CONVERT_TO_STRING_DIGITS );
	convert_to_string( text, MAX_CONVERT_TO_STRING_DIGITS, value, radix, trailing );
	text = (char *)memory_arena_transient_reallocate( &program->memoryArena, text, string_utf8_bytes( text ) );
	return text;
}

[[nodiscard]] const char *convert_to_string( u64 value, i32 radix, i32 trailing )
{
	char *text = (char *)memory_arena_transient_allocate( &program->memoryArena, MAX_CONVERT_TO_STRING_DIGITS );
	convert_to_string( text, MAX_CONVERT_TO_STRING_DIGITS, value, radix, trailing );
	text = (char *)memory_arena_transient_reallocate( &program->memoryArena, text, string_utf8_bytes( text ) );
	return text;
}

[[nodiscard]] const char *convert_to_string( i8 value, i32 radix, i32 trailing )
{
	char *text = (char *)memory_arena_transient_allocate( &program->memoryArena, MAX_CONVERT_TO_STRING_DIGITS );
	convert_to_string( text, MAX_CONVERT_TO_STRING_DIGITS, value, radix, trailing );
	text = (char *)memory_arena_transient_reallocate( &program->memoryArena, text, string_utf8_bytes( text ) );
	return text;
}

[[nodiscard]] const char *convert_to_string( i16 value, i32 radix, i32 trailing )
{
	char *text = (char *)memory_arena_transient_allocate( &program->memoryArena, MAX_CONVERT_TO_STRING_DIGITS );
	convert_to_string( text, MAX_CONVERT_TO_STRING_DIGITS, value, radix, trailing );
	text = (char *)memory_arena_transient_reallocate( &program->memoryArena, text, string_utf8_bytes( text ) );
	return text;
}

[[nodiscard]] const char *convert_to_string( i32 value, i32 radix, i32 trailing )
{
	char *text = (char *)memory_arena_transient_allocate( &program->memoryArena, MAX_CONVERT_TO_STRING_DIGITS );
	convert_to_string( text, MAX_CONVERT_TO_STRING_DIGITS, value, radix, trailing );
	text = (char *)memory_arena_transient_reallocate( &program->memoryArena, text, string_utf8_bytes( text ) );
	return text;
}

[[nodiscard]] const char *convert_to_string( i64 value, i32 radix, i32 trailing )
{
	char *text = (char *)memory_arena_transient_allocate( &program->memoryArena, MAX_CONVERT_TO_STRING_DIGITS );
	convert_to_string( text, MAX_CONVERT_TO_STRING_DIGITS, value, radix, trailing );
	text = (char *)memory_arena_transient_reallocate( &program->memoryArena, text, string_utf8_bytes( text ) );
	return text;
}

[[nodiscard]] const char *convert_to_string( f32 value, i32 fracDigits )
{
	char *text = (char *)memory_arena_transient_allocate( &program->memoryArena, MAX_CONVERT_TO_STRING_DIGITS );
	convert_to_string( text, MAX_CONVERT_TO_STRING_DIGITS, value, fracDigits );
	text = (char *)memory_arena_transient_reallocate( &program->memoryArena, text, string_utf8_bytes( text ) );
	return text;
}

[[nodiscard]] const char *convert_to_string( bool value )
{
	char *text = (char *)memory_arena_transient_allocate( &program->memoryArena, MAX_CONVERT_TO_STRING_DIGITS );
	convert_to_string( text, MAX_CONVERT_TO_STRING_DIGITS, value );
	text = (char *)memory_arena_transient_reallocate( &program->memoryArena, text, string_utf8_bytes( text ) );
	return text;
}