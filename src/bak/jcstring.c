#include "jcstring.h"

STRBUF* stringCreateBuffer(){
	return stringCreateBuffer_capacity( 255 );
}
	
STRBUF* stringCreateBuffer_capacity( unsigned int initial_capacity ){
	STRBUF* sbNew = malloc( sizeof( STRBUF ) );
	if( sbNew = NULL ) return NULL;
	char* sNew = malloc( initial_capacity + 1 );
	if( sNew = NULL ) return NULL;
	int xString = 0;
	for( ; xString <= initial_capacity; xString++ ) sNew[ xString ] = 0;
	sbNew.capacity = initial_capacity;
	sbNew.length   = 0;
	sbNew.string   = sNew;
}

STRBUF* stringCreateBuffer_value( char* initial_value ){
	int xString = 0;
	for( ;; xString++ ) if( initial_value[ xString ] == 0 ) break; /* reached end of string */
	return stringCreateBuffer_valcap( initial_value, xString );
}

STRBUF* stringCreateBuffer_valcap( char* initial_value, unsigned int initial_capacity ){
	STRBUF* sbNew = stringCreateBuffer_capacity( initial_capacity );
	if( sbNew == NULL ) return NULL;
	int xString = 0;
	for( ; xString < initial_capacity; xString++ ){
		if( initial_value[ xString ] == 0 ) break; /* reached end of string */
		sbNew.string[ xString ] = initial_value[ xString ];
	}
	sbNew.length = xString;
}

SVALUE* stringCreateBlank( unsigned int iSize ){
	SVALUE* svNew = malloc( sizeof( SVALUE );
	if( svNew == NULL ) return NULL;
	svNew.string = malloc( iSize );
	if( svNew.string == NULL ) return NULL;
	int xString = 0;
	for( ; xString < iSize; xString++ ) svNew.string[ xString ] = 0;
	svNew.length = iSize;
}

SVALUE* stringCreateValue( char* value ){
	SVALUE* svNew = malloc( sizeof( SVALUE ) );
	if( svNew == NULL ) return NULL;
	int xString = 0;
	for( ;; xString++ ) if( initial_value[ xString ] == 0 ) break; /* reached end of string */
	svNew.length = xString;
	svNew.string = value;
}

void stringSetValue( SVALUE sv, char* string ){
	sv.string = string;
	for( sv.length = 0;; sv.length++ ) if( sv.string[ sv.length ] == 0 ) break; /* reached end of string */
}

void stringInsert( STRBUF* buffer, char* text_to_insert ){
	int lenInsert = 0;
	for( ;; lenInsert++ ) if( text_to_insert[ lenInsert ] == 0 ) break; /* reached end of string */
	if( buffer.length + lenInsert > buffer.capacity ){
		char* sbNew = malloc( buffer.length + lenInsert + 1 ); /* extra char is for null terminator */
		if( sbNew == NULL ) return NULL;
		for( int xNewBuffer = 0; xNewBuffer < lenInsert; xNewBuffer++ ) sbNew[ xNewBuffer ] = text_to_insert[ xNewBuffer ];
		for( int xOldBuffer = 0; xOldBuffer < buffer.length; xOldBuffer++ ) sbNew[ xOldBuffer + lenInsert ] = buffer.string[ xOldBuffer ];
		free( buffer.string );
		buffer.string = sbNew;
		buffer.length += lenInsert;
		buffer.capacity = buffer.length;
		buffer.string[ buffer.length ] = 0; /* make sure buffer string is null terminated */ 	
	} else { /* buffer already big enough to contain additional text */
		for( int xOldBuffer = buffer.length - 1; xOldBuffer >= 0; xOldBuffer-- ) buffer.string[ xOldBuffer + lenInsert ] = buffer.string[ xOldBuffer ];
		for( int xNewBuffer = 0; xNewBuffer < lenInsert; xNewBuffer++ ) buffer.string[ xNewBuffer ] = text_to_insert[ xNewBuffer ];
		buffer.length += lenInsert;
		buffer.string[ buffer.length ] = 0; /* make sure buffer string is null terminated */ 	
	}
} 

void stringAppend( STRBUF* buffer, char* text_to_append ){
	int lenInsert = 0;
	for( ;; lenInsert++ ) if( text_to_insert[ lenInsert ] == 0 ) break; /* reached end of string */
	if( buffer.length + lenInsert > buffer.capacity ){
		char* sbNew = malloc( buffer.length + lenInsert + 1 ); /* extra char is for null terminator */
		if( sbNew == NULL ) return NULL;
		for( int xOldBuffer = 0; xOldBuffer < buffer.length; xOldBuffer++ ) sbNew[ xOldBuffer ] = buffer.string[ xOldBuffer ];
		for( int xNewBuffer = 0; xNewBuffer < lenInsert; xNewBuffer++ ) sbNew[ buffer.length + xNewBuffer ] = text_to_insert[ xNewBuffer ];
		free( buffer.string );
		buffer.string = sbNew;
		buffer.length += lenInsert;
		buffer.capacity = buffer.length;
		buffer.string[ buffer.length ] = 0; /* make sure buffer string is null terminated */ 	
	} else { /* buffer already big enough to contain additional text */
		for( int xAppendText = 0; xAppendText < lenInsert; xAppendText++ ) buffer.string[ buffer.length + xAppendText ] = text_to_insert[ xAppendText ];
		buffer.length += lenInsert;
		buffer.string[ buffer.length ] = 0; /* make sure buffer string is null terminated */ 	
	}
} 

void stringAppendChar( STRBUF* buffer, char char_to_append ){
	if( buffer.length + 1 > buffer.capacity ){
		char* sbNew = malloc( buffer.length + 1 );
		if( sbNew == NULL ) return NULL;
		for( int xOldBuffer = 0; xOldBuffer < buffer.length; xOldBuffer++ ) sbNew[ xOldBuffer ] = buffer.string[ xOldBuffer ];
		sbNew[ buffer.length ] = char_to_append;
		free( buffer.string );
		buffer.string = sbNew;
		buffer.length += 1;
		buffer.capacity = buffer.length;
		buffer.string[ buffer.length ] = 0; /* make sure buffer string is null terminated */ 	
	} else { /* buffer already big enough to contain additional text */
		buffer.string[ buffer.length ] = char_to_append;
		buffer.length += lenInsert;
		buffer.string[ buffer.length ] = 0; /* make sure buffer string is null terminated */ 	
	}
}
/* match EXACT char array to beginning of SVALUE */
bool stringMatchCharArray( SVALUE* svalue, char* char_array, size_t offset, size_t length ){
	size_t xSValue = 0;
	size_t offsetEnd = offset + length - 1;
	while( true ){
		if( offset == offsetEnd ) return true;
		if( xSValue == svalue->length ) return false;
		if( svalue->string[ xSValue ] != char_array[ offset ] ) return false;
		xSValue++;
		offset++;
	}
}

bool stringContains( STRBUF* buffer, char c ){
	if( buffer == NULL ) return FALSE;
	for( pos = 0; pos < buffer->length; pos++ ){
		if( buffer->value[ pos ] == c ) return TRUE;
	}
	return FALSE;
}

SVALUE** stringSplitAndTrim( STRBUF* buffer, char c ){
	STRBUF* sb = stringCreateBuffer( buffer.length + 1 );
	unsigned int posField = 0;
	unsigned int lenField = 0;
	unsigned int posBuffer = 0;
	unsigned int posBuffer_reverse = 0;
	unsigned int ctFields = 1;
	unsigned int xField = 0;
	for( posBuffer = 0; posBuffer < buffer.length; posBuffer++ ){
		if( buffer[ posBuffer ] == c ) ctFields++;
	}
	posBuffer = 0;
	SVALUE** aFields = malloc( sizeof( SVALUE* ) * (ctFields + 1) ); // null terminated array
	if( aFields == NULL ) return NULL;
	posBuffer = 0;
	for( xField = 0; xField < ctFields; xField++ ){
		while( posBuffer < buffer.length && buffer[ posBuffer ] != c ) lenField++; // determine max length of field (before trimming)
		SVALUE* svField = malloc( sizeof( SVALUE ) );
		if( svField == NULL ) return NULL;
		aFields[ xField ] = svField;
		posBuffer -= lenField;
		posField = 0;
		while( buffer[ posBuffer ] == ' ' || buffer[ posBuffer ] == '\t' ) posBuffer++; // skip leading spaces and tabs
		while( posBuffer < buffer.length && buffer[ posBuffer ] != c ){
			aFields[ xField ].buffer[ posField++ ] = buffer[ posBuffer++ ];
		}
		posBuffer_reverse = posBuffer;
		while( posBuffer_reverse > 0 && posField > 0 ){ // go backwards through the field and remove any trailing spaces and tabs
			posBuffer_reverse--;
			if( buffer[ posBuffer ] == c ) break;
			if( buffer[ posBuffer ] != ' ' && buffer[ posBuffer ] != '\t' ) break;
			posField--;
		}
	}
	return aFields;
}
	
