#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "pjl.h"

#define ESC "\x1B"

#define UNIVERSAL_EXIT_LANGUAGE ESC "%%-12345X"
#define PJL_END_OF_JOB "@PJL EOJ\n"
#define PCL_DELIMITER      ESC "E"
#define PCL_JOB_SEPARATION ESC "&l1T"

const int HP_DPI[] = { 96, 100, 120, 144, 150, 160, 180, 200, 225, 240, 288, 300, 360, 400, 450, 480, 600, 720, 800, 900, 1200, 1440, 1800, 2400, 3600, 7200 };

const char *FontFamilyNames[33] = {
	"Line Printer",
	"Albertus",
	"Antique Olive",
	"Arial",
	"ITC Avant Garde Gothic",
	"ITC Bookman",
	"ITC Zapf Chancery",
	"Clarendon",
	"Coronet",
	"Courier",
	"CourierPS",
	"ITC Zapf Dingbats",
	"ITC Zapf Dingbats MS",
	"Garamond Antiqua",
	"MS Gothic",
	"Helvetica",
	"Helvetica Narrow",
	"GW-Kai",
	"Letter Gothic",
	"Marigold",
	"MS Mincho",
	"New Century Schoolbook",
	"CG Omega",
	"Palatino",
	"SimHei",
	"SimSun",
	"Symbol",
	"SymbolPS",
	"CG Times",
	"Times New",
	"Times Roman",
	"Univers",
	"Wingdings"
};

const char *FontFamily_Number[33] = {
	"0",
	"4362",
	"4168",
	"16602",
	"24607",
	"24623",
	"45099",
	"4140",
	"4116",
	"4099",
	"24579",
	"4141",
	"45101",
	"4197",
	"28825",
	"24580",
	"24580",
	"37357",
	"4102",
	"4297",
	"28752",
	"24703",
	"4113",
	"24591",
	"37110",
	"37058",
	"16686",
	"45358",
	"4101",
	"16901",
	"25093",
	"4148",
	"31402"
};

const char *SymbolSetCode[1] = {
	ESC "(8U"
};

static PCL_Element* createNewElement( PCL_Form* pcl );
static bool zValidateJobText( char* sJobName );
static void printReadablePCL( char* sPCL );
static char* createFontPCL( FONT_DEFINITION* font );
static char* createJob_PCL( char* sJobName, char* sDisplayText, PAGE_SIZE ePageSize, PAPER_SOURCE ePaperSource, ORIENTATION eOrientation, unsigned int dpi, char** listCommands_PCL );

char* createQuery();

/* TODO support custom page size
ESC &l10101P      // Enable custom page size mode
ESC &l612A        // Set width to 8.5 inches (612 decipoints)
ESC &l828B        // Set length to 11.5 inches (828 decipoints)
ESC &l0O          // Portrait orientation
ESC &l1H          // Enable paper handling (optional)
*/

char* createQuery(){
	char* sPCL_MemoryFreeSpace            = ESC "*s1M";
	char* sPCL_StatusReadBackLocationType = ESC "*s2T";   // all locations
	char* sPCL_StatusReadBackLocationUnit = ESC "*s2U";   // all units (3, 0) all internal (4, 0) all downloaded
	char* sPCL_StatusReadBackEntity       = ESC "*x0I";   // 0-font 1-macro 2-userdefined 3-symbol set 4-font extended
	return NULL;   	
}

PCL_Form* createPCL( char* sJobName, char* sDisplayText, PAGE_SIZE ePageSize, PAPER_SOURCE ePaperSource, ORIENTATION eOrientation, unsigned int dpi, FONT_DEFINITION* default_font ){
	PCL_Form* pcl = (PCL_Form*) malloc( sizeof( PCL_Form ) );
	if( pcl == NULL ){
		printf( "failed to allocate memory for PCL form\n" );
		return NULL;
	}
	pcl->dpi          = dpi;
	pcl->ePageSize    = ePageSize;
	pcl->ePaperSource = ePaperSource;
	pcl->eOrientation = eOrientation;
	pcl->sJobName     = sJobName;
	pcl->sDisplayText = sDisplayText;
	pcl->fontDefault  = default_font;
	
	PCL_Element** listElements = (PCL_Element**) malloc( sizeof( void* ) * 1001 ); // default to 1000 elements
	if( listElements == NULL ){
		printf( "Failed to allocate memory for PCL elements\n" );
		return NULL;
	}
	pcl->szElement_capacity = 1000;
	pcl->szElement_count    = 0;
	for( size_t xElement = 0; xElement < pcl->szElement_capacity; xElement++ ) listElements[ xElement ] = NULL;
	pcl->listElements = listElements;
	
	return pcl;
}

FONT_DEFINITION* createFontDefinition( FONT_SymbolSet eSymbolSet, FONT_Spacing eSpacing, int iHeight, FONT_Style eStyle, FONT_Stroke eStroke, FONT_Family eFamily ){
	FONT_DEFINITION* font = (FONT_DEFINITION*) malloc( sizeof( FONT_DEFINITION ) );
	if( font == NULL ){
		printf( "Failed to allocate memory for font definition\n" );
	   	return NULL;
	}
	font->eSymbolSet = eSymbolSet;  //  Roman_8
	font->eSpacing   = eSpacing;    //  SPACING_Proportional
	font->iHeight    = iHeight;     //  1200
	font->eStyle     = eStyle;      //  Upright
	font->eStroke    = eStroke;     //  Bold
	font->eFamily    = eFamily;     //  CG_Omega
	return font;
}

static PCL_Element* createNewElement( PCL_Form* pcl ){
	if( pcl == NULL ){
		printf( "no PCL structure supplied\n" );
		return NULL;
	}
	if( pcl->szElement_count >= pcl->szElement_capacity ){  // need to increase the size of the array
		size_t new_capacity = pcl->szElement_capacity * 10 / 4 + 1;
		PCL_Element** expanded_array = (PCL_Element**) realloc( pcl->listElements, new_capacity );
		if( expanded_array == NULL ){
			printf( "failed to expand list to accomodate more text elements\n" );
			return false;
		}
		pcl->listElements = expanded_array;
		pcl->szElement_capacity = new_capacity;
	}
	size_t xNewElement = pcl->szElement_count;
	PCL_Element* new_element = (PCL_Element*) malloc( sizeof( PCL_Element ) );
	if( new_element == NULL ){
		printf( "failed to allocate memory for new PCL element\n" );
		return NULL;
	}
	pcl->listElements[ pcl->szElement_count ] = new_element;
	pcl->szElement_count++;
	return new_element;
}
	
bool addTextElement( PCL_Form* pcl, int x, int y, char* string, FONT_DEFINITION* font ){ 

	// create new text element
	PCL_Text* text = (PCL_Text*) malloc( sizeof( PCL_Text ) );
	if( text == NULL ){
		printf( "failed to allocate memory for new text element\n" );
		return false;
	}
	text->x      = x;  // TODO validate
	text->y      = y;
	text->string = string;
	text->font   = font;

	// create new text element type and add it to the list
	PCL_Element* new_element = createNewElement( pcl );
	new_element->type = Element_Text;
	new_element->element = text;
	return true;
}

bool setFont( PCL_Form* pcl, FONT_DEFINITION* font ){
	PCL_Element* new_element = createNewElement( pcl );
	new_element->type = Element_Font;
	new_element->element = font;
	return true;
}

static char* createJob_PCL( char* sJobName, char* sDisplayText, PAGE_SIZE ePageSize, PAPER_SOURCE ePaperSource, ORIENTATION eOrientation, unsigned int dpi, char** listCommands_PCL ){
	int ctCommands = 0;
	char** pCurrentCommand = listCommands_PCL;
	size_t szCommandList = 0;
	while( true ){
		if( *pCurrentCommand == NULL ) break;
		szCommandList += strlen( *pCurrentCommand );
		pCurrentCommand++;
	}
	size_t szJob = strlen( UNIVERSAL_EXIT_LANGUAGE ) * 2 + strlen( sJobName ) + 500;
	char* buffer = (char*) calloc( szJob, sizeof( char ) );
	if( buffer == NULL ) return NULL;
	size_t position = 0;

	// Begin Writing Job Buffer
	position += snprintf( buffer + position, szJob - position, UNIVERSAL_EXIT_LANGUAGE );
	position += snprintf( buffer + position, szJob - position, "@PJL \n" );
	position += snprintf( buffer + position, szJob - position, "@PJL JOB " );
	
	// JOB NAME (optional argument)
	if( zValidateJobText( sJobName ) ){
		position += snprintf( buffer + position, szJob - position, "NAME = \"" );
		position += snprintf( buffer + position, szJob - position, "%s\" ", sJobName );
	}

	// DISPLAY TEXT (optional argument)
	if( zValidateJobText( sDisplayText ) ){
		position += snprintf( buffer + position, szJob - position, "DISPLAY = \"" );
		position += snprintf( buffer + position, szJob - position, "%s\" ", sDisplayText );
	}
	
	// Terminate JOB command with a newline
	position += snprintf( buffer + position, szJob - position, "\n" );

	// Change language to PCL
	position += snprintf( buffer + position, szJob - position, "@PJL ENTER LANGUAGE = PCL \n" );
	position += snprintf( buffer + position, szJob - position, PCL_DELIMITER );
	
	// initialize PCL printer environment
	position += snprintf( buffer + position, szJob - position, "%c&u%dD", 27, dpi);
	position += snprintf( buffer + position, szJob - position, "%c&l%dA", 27, ePageSize );
	position += snprintf( buffer + position, szJob - position, "%c&l%dH", 27, ePaperSource );
	position += snprintf( buffer + position, szJob - position, "%c&l%dO", 27, eOrientation );

	// insert PCL commands
	unsigned long xCommand = 0;
	pCurrentCommand = listCommands_PCL;
	while( true ){
		if( *pCurrentCommand == NULL ) break;
		position += snprintf( buffer + position, szJob - position, "%s", *pCurrentCommand );
		pCurrentCommand++;
	}

	// insert PCL job separation command (recommended by HP)
	position += snprintf( buffer + position, szJob - position, PCL_JOB_SEPARATION );

	position += snprintf( buffer + position, szJob - position, PCL_DELIMITER );
	position += snprintf( buffer + position, szJob - position, UNIVERSAL_EXIT_LANGUAGE );
	position += snprintf( buffer + position, szJob - position, PJL_END_OF_JOB );
	position += snprintf( buffer + position, szJob - position, UNIVERSAL_EXIT_LANGUAGE );
	buffer[ position ] = '\0';
	return buffer;

}

static char* createFontPCL( FONT_DEFINITION* font ){
	if( font == NULL ) return "";
	int iFontHeight = font->iHeight;
	if( iFontHeight < 25 || iFontHeight > 99975 ) iFontHeight = 1200;
	int iFontHeight_integer = iFontHeight / 100;
	int iFontHeight_remainder = iFontHeight - ( iFontHeight_integer * 100 );
	int iFontHeight_decimal = 0;
	if( iFontHeight_remainder < 13 ) iFontHeight_decimal = 0;
	else if( iFontHeight_remainder < 38 ) iFontHeight_decimal = 25;
	else if( iFontHeight_remainder < 63 ) iFontHeight_decimal = 50;
	else if( iFontHeight_remainder < 88 ) iFontHeight_decimal = 75;
	else iFontHeight_integer += 1;                                      // round height fraction to nearest .25	
	const char* sFontFamilyID = FontFamily_Number[ font->eFamily ];
	
	size_t lenSymbolSet  = snprintf(NULL, 0, ESC "(%dU", 0 );
	size_t lenSpacing    = snprintf(NULL, 0, ESC "(s1P" );
	size_t lenHeight     = snprintf(NULL, 0, ESC "(s%d.%02dV", iFontHeight_integer, iFontHeight_decimal );
	size_t lenStyle      = snprintf(NULL, 0, ESC "(s%dS", font->eStyle );
	size_t lenStroke     = snprintf(NULL, 0, ESC "(s%dB", font->eStroke );
	size_t lenFamily     = snprintf(NULL, 0, ESC "(s%sT", sFontFamilyID );
	size_t lenFontString = lenSymbolSet + lenSpacing + lenHeight + lenStyle + lenStroke + lenFamily + 1;
	char* sFontString = (char*) malloc( lenFontString * sizeof( char ) );

	size_t sz = lenFontString;
	size_t position = 0;
	position += snprintf( sFontString + position, sz - position, ESC "(%dU", 0 );
	position += snprintf( sFontString + position, sz - position, ESC "(s1P" );
	position += snprintf( sFontString + position, sz - position, ESC "(s%d.%02dV", iFontHeight_integer, iFontHeight_decimal );
	position += snprintf( sFontString + position, sz - position, ESC "(s%dS", font->eStyle );
	position += snprintf( sFontString + position, sz - position, ESC "(s%dB", font->eStroke );
	position += snprintf( sFontString + position, sz - position, ESC "(s%sT", sFontFamilyID );
	return sFontString;
}

char* createJob_PCL_Form( PCL_Form* form ){

	// count of elements and commands required
	size_t ctCommands = 0;
	size_t ctElements = form->szElement_count;
	if( form->fontDefault == NULL ){  // then the font only changes when explicitly set by a text element
		for( size_t xElement = 0; xElement < ctElements; xElement++ ){
			if( form->listElements[ xElement ]->type == Element_Text ){
				PCL_Text* pText = (PCL_Text*)(form->listElements[ xElement ]->element);
				if( pText->font != NULL ) ctCommands++; // additional command required to set the font
			}
			ctCommands++;
		}
	} else {
		bool zUsingDefaultFont = false;
		for( size_t xElement = 0; xElement < ctElements; xElement++ ){
			if( form->listElements[ xElement ]->type == Element_Text ){
				PCL_Text* pText = (PCL_Text*)(form->listElements[ xElement ]->element);
				if( pText->font == NULL ){
					if( zUsingDefaultFont ){
						// nothing to do, font is already set to default
					} else {
						ctCommands++; // set to default font
						zUsingDefaultFont = true;
					}
				} else {
					ctCommands++; // set to text font
					zUsingDefaultFont = false;
				}
			}
	   		ctCommands++; // set to text font
		}
	}

	// allocate memory for the PCL commands
	char** listCommands_PCL = (char**) malloc( sizeof( char* ) * ctCommands + 1 );
	if( listCommands_PCL == NULL ) return NULL;

	// generate default font string, if present
	char* sDefaultFontCommand = NULL;
	if( form->fontDefault != NULL ){
		sDefaultFontCommand = createFontPCL( form->fontDefault );
	}

	size_t xCommand = 0;
	bool zUsingDefaultFont = false;
	for( size_t xElement = 0; xElement < ctElements; xElement++ ){
		if( xCommand >= ctCommands ){
			printf( "Internal error, command buffer count mismatch.\n" );
			return NULL;
		}
		FONT_DEFINITION* font = NULL;
		switch( form->listElements[ xElement ]->type ){
			case Element_Font:
				font = (FONT_DEFINITION*)( form->listElements[ xElement ]->element );
				listCommands_PCL[ xCommand ] = createFontPCL( font );
				xCommand++;
				break;
			case Element_Text:{
				PCL_Text* text = (PCL_Text*)(form->listElements[ xElement ]->element);
				if( text->font == NULL ){
					if( sDefaultFontCommand == NULL ){
						// do nothing, whatever the existing font is will be used
					} else {
						if( zUsingDefaultFont ){
							// we are already using the default font
						} else {
							listCommands_PCL[ xCommand ] = sDefaultFontCommand;
							xCommand++;
							zUsingDefaultFont = true;
						}
					}
				} else {
					listCommands_PCL[ xCommand ] = createFontPCL( text->font );
					xCommand++;
					zUsingDefaultFont = false;
				}
				int x   = text->x;
				int y   = text->y;
				char* s = text->string;
				size_t lenX = snprintf( NULL, 0, "%d", x );
				size_t lenY = snprintf( NULL, 0, "%d", y );
				size_t lenS = snprintf( NULL, 0, "%s", s );
				size_t lenCommand = 5 + lenX + lenY + lenS;
				char* sCommand = (char*) malloc( sizeof( char* ) * (lenCommand + 1) );
				if( sCommand == NULL ) return NULL;
				size_t bytes_written = snprintf( sCommand, lenCommand + 1, "%c*p%dx%dY%s%c", 27, x, y, s, 0  );
				if( bytes_written != lenCommand + 1 ) return NULL;
				listCommands_PCL[ xCommand ] = sCommand;
				xCommand++;
				break;
			}
			default:
				printf( "unsupported element type\n" );
				return false;
		}
	}
	listCommands_PCL[ ctCommands ] = NULL;   // the command list is NULL terminated

	char* sJob = createJob_PCL( form->sJobName, form->sDisplayText, form->ePageSize, form->ePaperSource, form->eOrientation, form->dpi, listCommands_PCL );
	return sJob;
}

static bool zValidateJobText( char* sJobName ){
	if( sJobName == NULL ) return false;
	size_t szName = strlen( sJobName );
	if( szName < 1 ) return false;
	if( szName > 80 ) return false;
	for( size_t xName = 0; xName < szName; xName++ ){
		char c = sJobName[ xName ];
		if( c == '"' ){
			c = '\'';
			sJobName[ xName ] = c; // convert quotation marks to single quotes
		}
		if( c >= 0x20 && c < 0x7F ) continue; // printable characters not including DEL 0x7F
	}
	return true;
}

static void printReadablePCL( char* sPCL ){
	size_t xPCL = 0;
	while( true ){
		char c = sPCL[ xPCL ];
		if( c == '\0' ) break;
		if( c >= ' ' && c <= '~' ){
			printf( "%c", c );
		} else if( c == 27 ){
			printf( "<ESC>" );
		} else if( c == 9 ){
			printf( "<TAB>" );
		} else if( c == 10 ){
			printf( "<N>\n" );
		} else if( c == 13 ){
			printf( "<CR>" );
		} else printf( "<%d>", c );
		xPCL++;
	}
	printf( "\n" );
}

