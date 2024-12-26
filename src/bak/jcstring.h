typedef struct {
	unsigned int capacity;
	unsigned int length;
	char* string;
} STRBUF;

typedef struct {
	unsigned int length;
	char* string;
} SVALUE;

STRBUF* stringCreateBuffer();
STRBUF* stringCreateBuffer_capacity( unsigned int initial_capacity );
STRBUF* stringCreateBuffer_value( char* initial_value );
STRBUF* stringCreateBuffer_valcap( char* initial_value, unsigned int initial_capacity );
SVALUE* stringCreateValue( char* value );

void stringInsert( STRBUF* buffer, char* text_to_insert );
void stringAppend( STRBUF* buffer, char* text_to_append );
bool stringMatchCharArray( SVALUE* svalue, char* char_array, size_t offset, size_t length );
bool stringContains( STRBUF* buffer, char c );
SVALUE** stringSplitAndTrim( STRBUF* buffer, char c );

