#include <stdio.h>      /* snprintf  */
#include <stddef.h>
#include <stdlib.h>      /* atoi() */
#include <time.h>       
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>         /* inet_aton */
#include <fcntl.h>             /* open */
#include <sys/stat.h>          /* open permission constants */
#include <sys/mman.h>          /* pointer check */
#include <unistd.h>            /* pointer check */

#include "pjl.h"
#include "jcshared.h"

#define PROGRAM_TITLE "HP PJL Printing Program - 2024-12-13\n"

char* sStatus = NULL;
static struct termios saved_tty_mode;

STRBUF* getWordsForAmount( SVALUE* amount, STRBUF* sbError );
bool writeCheckRecord( char *sDataFilePath, SVALUE* sCheckNumber, SVALUE* sDate, SVALUE* sPayee, SVALUE* sAmount, STRBUF* sbAmountWords, SVALUE* sMemo, STRBUF* sbError );
void getCurrentTimeString( char* time_string );
void getCurrentDate_SpelledOut( char* time_string );
void interact();
void interact_test();
void interact_print_file( SVALUE* sPrinterIP );
void interact_print_envelope( SVALUE* sPrinterIP );
void interact_print_check( SVALUE* sPrinterIP );
void interact_set_printer( SVALUE** sPrinterIP );

void testWordsForAmount();
static void printPCL( char* sPCL );

const char* DEFAULT_PRINTER_IP = "10.1.10.190";
const char* digit_names[] = { "ZERO", "ONE", "TWO", "THREE", "FOUR", "FIVE", "SIX", "SEVEN", "EIGHT", "NINE" };
const char* tens_names[]  = { "", "TEN", "TWENTY", "THIRTY", "FORTY", "FIFTY", "SIXTY", "SEVENTY", "EIGHTY", "NINETY" };
const char* teens_names[] = { "TEN", "ELEVEN", "TWELVE", "THIRTEEN", "FOURTEEN", "FIFTEEN", "SIXTEEN", "SEVENTEEN", "EIGHTEEN", "NINETEEN" };

char* const log_location_Aeolus = "/home/jsc/aeolus/aeolus_checks.log";
char* const log_location_NAMC = "/home/jsc/namc/NAMC_checks.log";

int main( int arg_count, char* args[] ){
	tty_save( &saved_tty_mode );                 /* save current terminal mode */
	if( arg_count > 1 ){
		SVALUE* sPrinterIP = stringCreateValue( args[1] );
		if( sPrinterIP == NULL ){
			printf( "failed to get printer IP\n" );
			return 0;
		}
		SVALUE* sFilePath  = stringCreateValue( args[2] );
		if( sFilePath == NULL ){
			printf( "failed to get file path\n" );
			return 0;
		}
		printf( "printing %s to %s\n", sFilePath->string, sPrinterIP->string );
		STRBUF* sbError = stringCreateBuffer();
		if( printFile2IP( sPrinterIP, sFilePath, sbError ) ){
			printf( "printed %s to %s\n", sFilePath->string, sPrinterIP->string );
		} else {
			printf( "failed to print %s to %s: %s\n", sFilePath->string, sPrinterIP->string, sbError->string );
		}
	} else {
		// testWordsForAmount();
		interact();                 /* interact with user */
	}
	tty_restore( &saved_tty_mode );              /* restore terminal to the way it was */
	return 0;
}

void interact(){
	SVALUE* sPrinterIP = stringCreateValue( DEFAULT_PRINTER_IP );
	set_terminal_raw();
	while( true ){
		printf( PROGRAM_TITLE );
		if( sStatus != NULL ) printf( "%s\n", sStatus );
		printf( "Print to: %s\n", sPrinterIP->string );
		printf( ": (F)ile (E)nvelope (C)heck (P)rinter (Q) Quit" );
		char cInput = getchar();
		if( cInput != EOF ) printf( "   :: %c\n", cInput );
		cInput = cInput & '_'; // convert letter to upper case if necessary
		switch( cInput ){
			case 'F': interact_print_file( sPrinterIP ); break;
			case 'E': interact_print_envelope( sPrinterIP ); break;
			case 'C': interact_print_check( sPrinterIP ); break;
			case 'P': interact_set_printer( &sPrinterIP ); break;
			case 'X': interact_test(); break;
			case 'Q': return;
			case EOF: return;
		}
	}
	set_terminal_buffered();
}

void interact_test(){
	printf( "test key presses:\n" );
	while( true ){
		int c = getchar();
		if( c == '\n' ) return;
		printf( "%d\n", c );
	}
}

void interact_set_printer( SVALUE** sPrinterIP ){
	STRBUF* sbError = stringCreateBuffer();
	bool skip = false;
	bool cancel = false;
	SVALUE* sNewPrinterIP = getTextInput( "Enter new printer IP", &skip, &cancel, sbError );
	if( cancel || skip ){
		return;
	}
	if( sNewPrinterIP == NULL ){
		printf( "Error getting new printer IP: %s\n", sbError->string );
		return;
	}
	stringFree( *sPrinterIP );
	*sPrinterIP = sNewPrinterIP;
	return;
}

void interact_print_file( SVALUE* sPrinterIP ){
	SVALUE* sFilePath;

	// get file path from user
	STRBUF* sbError = stringCreateBuffer();
	bool skip = false;
	bool cancel = false;
GetPath:
	sFilePath = getTextInput( "Enter File Path", &skip, &cancel, sbError );
	if( cancel ){
		return;
	}
	if( skip ){
		printf( "skipped" );
		goto GetPath;
	}
	if( sFilePath == NULL ){
		printf( "Error getting file path input: %s\n", sbError->string );
		return;
	}

	printf( "File to be printed: %s\n", sFilePath->string );
	printf( "Printer: %s\n", sPrinterIP->string );
	bool zPrint = getBooleanInput( "Ok to print?", NULL, NULL, sbError );
	if( ! zPrint ) goto GetPath;

	// print file
	if( printFile2IP( sPrinterIP, sFilePath, sbError ) ){
		printf( "file content sent to printer\n" );
	} else {
		printf( "failed to print file: %s\n", sbError->string );
	}
	stringFreeBuffer( sbError );
	stringFree( sFilePath );
	
	return;
}

void interact_print_envelope( SVALUE* sPrinterIP ){
	STRBUF* sbError = stringCreateBuffer();
	SVALUE* sAddress;
	SVALUE* sReturnAddress;
	size_t xLine = 0;
	bool skip = false;
	bool cancel = false;
	if( is_valid_pointer( sPrinterIP ) ){
		// valid
	} else {
		printf( "unable to print envelope, pointer to printer address is invalid: %p\n", sPrinterIP );
		return;
	}

	// get return address
	STRBUF* bufferReturnAddress = stringCreateBuffer_capacity( 10000 );
GetReturnAddress:
	printf( "Return Address: (A)eolus (N) NAMC (D)umpling JSC (P)O Box JSC (X) none (C)ustom (Q) Quit" );
	char cInput = getchar();
	if( cInput != EOF ) printf( "   :: %c\n", cInput );
	cInput = cInput & '_'; // convert letter to upper case if necessary
	switch( cInput ){
		case 'A':
			stringAppend( bufferReturnAddress, "Aeolus Limited\n" );
			stringAppend( bufferReturnAddress, "29 Dumpling Cove Road\n" );
			stringAppend( bufferReturnAddress, "Newington, NH 03801\n" );
			sReturnAddress = stringCreateFromSTRBUF( bufferReturnAddress );
			break;
		case 'N':
			stringAppend( bufferReturnAddress, "Newington Asset Management Company\n" );
			stringAppend( bufferReturnAddress, "29 Dumpling Cove Road\n" );
			stringAppend( bufferReturnAddress, "Newington, NH 03801\n" );
			sReturnAddress = stringCreateFromSTRBUF( bufferReturnAddress );
			break;
		case 'D':
			stringAppend( bufferReturnAddress, "John S. Chamberlain\n" );
			stringAppend( bufferReturnAddress, "29 Dumpling Cove Road\n" );
			stringAppend( bufferReturnAddress, "Newington, NH 03801\n" );
			sReturnAddress = stringCreateFromSTRBUF( bufferReturnAddress );
			break;
		case 'P':
			stringAppend( bufferReturnAddress, "John S. Chamberlain\n" );
			stringAppend( bufferReturnAddress, "PO Box 1229\n" );
			stringAppend( bufferReturnAddress, "Portsmouth, NH 03802\n" );
			sReturnAddress = stringCreateFromSTRBUF( bufferReturnAddress );
			break;
		case 'X': break;
		case 'C':
			sReturnAddress = getTextInput( "Enter Return Address (use \\ for line breaks):\n", &skip, &cancel, sbError );
			if( cancel ){
				printf( "[user cancelled]\n" );
				return;
			}
			if( skip ){
				goto GetReturnAddress;
			}
			if( sAddress == NULL ){
				printf( "Error getting address input: %s\n", sbError->string );
				return;
			}

			break;
		case 'Q': return;
		case EOF: return;
		default: goto GetReturnAddress;
	}
	
	// split address into individual lines of text
	SVALUE** linesReturnAddress = stringSplit( sReturnAddress, '\n' );

	// get envelope address from user
GetAddress:
	sAddress = getMultilineInput( "Enter Addressee ", &skip, &cancel, sbError );
	if( cancel ){
		return;
	}
	if( skip ){
		printf( "Cannot print a blank envelope.\n" );
		goto GetAddress;
	}
	if( sAddress == NULL ){
		printf( "Error getting address input: %s\n", sbError->string );
		return;
	}

	// split address into individual lines of text
	SVALUE** linesDeliveryAddress = stringSplit( sAddress, '\n' );
	size_t xDeliveryAddress = 0; 
	while( true ){
		if( linesDeliveryAddress[ xDeliveryAddress ] != NULL ) break;
		xDeliveryAddress++;
	}
	size_t ctLines_DeliveryAddress = xDeliveryAddress;
	if( ctLines_DeliveryAddress > 0 && linesDeliveryAddress[ ctLines_DeliveryAddress - 1 ]->length == 0 ){
		stringFree( linesDeliveryAddress[ ctLines_DeliveryAddress - 1 ] );
		linesDeliveryAddress[ ctLines_DeliveryAddress - 1 ] = NULL;  // there is probably a blank line at the end of the list, delete it
	}

   	printf( "Envelope will be printed as follows:\n" );
	while( linesDeliveryAddress[ xLine ] != NULL ){
		printf( "%s\n", linesDeliveryAddress[ xLine ]->string );
		xLine++;
	}
	bool zPrint = getBooleanInput( "Ok to print?", NULL, &cancel, sbError );
	if( cancel ) return;
	if( ! zPrint ) goto GetAddress;

	// create the return address font for the envelope
	FONT_SymbolSet eSymbolSet = Roman_8;
	FONT_Spacing eSpacing     = SPACING_Proportional;
	int iFontHeight           = 1200;
	FONT_Style eStyle         = Upright;
	FONT_Stroke eStroke       = Bold;
	FONT_Family eFamily       = Times_New;
	FONT_DEFINITION* fontReturnAddress = createFontDefinition( eSymbolSet, eSpacing, iFontHeight, eStyle, eStroke, eFamily );
	if( fontReturnAddress == NULL ){
		printf( "Failed to allocate memory for return address font definition\n" );
	   	return;
	}

	// create the delivery address font for the envelope
	eSymbolSet   = Roman_8;
	eSpacing     = SPACING_Proportional;
	iFontHeight  = 1400;
	eStyle       = Upright;
	eStroke      = Bold;
	eFamily      = Times_New;
	FONT_DEFINITION* fontDeliveryAddress = createFontDefinition( eSymbolSet, eSpacing, iFontHeight, eStyle, eStroke, eFamily );
	if( fontDeliveryAddress == NULL ){
		printf( "Failed to allocate memory for delivery address font definition\n" );
	   	return;
	}

	// create PCL job form
	PAPER_SOURCE ePaperSource = FEED_feeder;
	PAGE_SIZE ePageSize = Com_10;
	ORIENTATION eOrientation = ORIENTATION_Landscape;
   	unsigned int dpi = 600;
	char* sJobName = "Print Envelope";
	char* sDisplayText = "Printing Envelope...";
	PCL_Form* pcl = createPCL( sJobName, sDisplayText, ePageSize, ePaperSource, eOrientation, dpi, NULL );
	if( pcl == NULL ){
		printf( "failed to create PCL object for envelope\n" );
		return;
	}
	
	// set return address font
	if( ! setFont( pcl, fontReturnAddress ) ){
		printf( "failed to set font for return address\n" );
		return;
	}
	
	// draw return address
	xLine = 0;
	while( linesReturnAddress[ xLine ] != NULL ){
		int x = 0;
		int y = xLine * 100;
		addTextElement( pcl, x, y, linesReturnAddress[ xLine ]->string, NULL ); 
		xLine++;
	}
	
	// set delivery address font
	if( ! setFont( pcl, fontDeliveryAddress ) ){
		printf( "failed to set font for delivery address\n" );
		return;
	}
	
	// draw delivery address
	xLine = 0;
	while( linesDeliveryAddress[ xLine ] != NULL ){
		int x = 2000;
		int y = 820 + xLine * 140;
		addTextElement( pcl, x, y, linesDeliveryAddress[ xLine ]->string, NULL ); 
		xLine++;
	}
	
	char* sOutputPCL = createJob_PCL_Form( pcl );
	// printPCL( sOutputPCL );
	if( printBuffer2IP( sPrinterIP, sOutputPCL, strlen( sOutputPCL ), sbError ) ){
		printf( "envelope content sent to printer\n" );
	} else {
		printf( "failed to print envelope: %s\n", sbError->string );
	}
	stringFreeBuffer( sbError );
	stringFreeBuffer( bufferReturnAddress );
	
	return;
}

void interact_print_check( SVALUE* sPrinterIP ){

	if( is_valid_pointer( sPrinterIP ) ){
		// printer is there
	} else {	
		printf( "unable to print check, printer IP is missing or invalid: %p\n", sPrinterIP );
		return;
	}	

	// get check information from user
	STRBUF* sbError = stringCreateBuffer();
	bool skip = false;
	bool cancel = false;
	
	// get the desired checkbook
	char* sDataFilePath;
	set_terminal_raw();
	printf( "Checkbook: (A)eolus Limited (N)AMC (Q) Quit" );
	char cInput = getchar();
	if( cInput != EOF ) printf( "   :: %c\n", cInput );
	cInput = cInput & '_'; // convert letter to upper case if necessary
	switch( cInput ){
		case 'A': case 'a': sDataFilePath = log_location_Aeolus; break;
		case 'N': case 'n': sDataFilePath = log_location_NAMC; break;
		case 'Q': return;
		case EOF: return;
	}
	set_terminal_buffered();

	SVALUE* sCheckNumber = NULL;
	SVALUE* sDate = NULL;
	SVALUE* sPayee = NULL;
	SVALUE* sAmount = NULL;
	SVALUE* sMemo = NULL;
GET_NUMBER:
	sCheckNumber = getTextInput( "Enter Check Number (for log)", NULL, &cancel, sbError ); 
	if( cancel ){
		return;
	}
	if( sCheckNumber == NULL ){
		printf( "Error getting check number input: %s\n", sbError->string );
		goto GET_NUMBER;
	}

	char sCurrentDate[ 100 ];
	getCurrentDate_SpelledOut( sCurrentDate );

GET_DATE:
	sDate = getTextInput_Default( "Enter Date", sCurrentDate, &cancel, sbError );
	if( cancel ) return;
	if( sDate == NULL ){
		printf( "Error getting date input: %s\n", sbError->string );
		goto GET_DATE;
	}

GET_PAYEE:	
	sPayee = getTextInput( "Enter Payee", NULL, &cancel, sbError );
	if( cancel || skip ){
		return;
	}
	if( sPayee == NULL ){
		printf( "Error getting payee input: %s\n", sbError->string );
		goto GET_PAYEE;
	}
	
GET_AMOUNT:	
	sAmount = getTextInput( "Enter Amount (no dollar sign)", &skip, &cancel, sbError );
	if( cancel || skip ){
		return;
	}
	if( sAmount == NULL ){
		printf( "Error getting amount input: %s\n", sbError->string );
		return;
	}

	STRBUF* sbAmountInWords = getWordsForAmount( sAmount, sbError );
	if( sbAmountInWords == NULL ){
		printf( "Error getting words for amount: %s\n", sbError->string );
		goto GET_AMOUNT;
	}

	// add asterisks to amount in words
	// the width of the amount-in-words line is 6 inches and font is 10 CPI = 60 characters
	STRBUF* sbAmountWithAsterisks = stringCopyBuffer( sbAmountInWords );
	if( sbAmountWithAsterisks == NULL ){
		printf( "failed to copy amount buffer" );
		return;
	}
	int ctAsterisks = 60 - (sbAmountWithAsterisks->length + 4);
	if( ctAsterisks >= 6 ){
		SVALUE* sAsterisks = stringCreateRepeating( '*', ctAsterisks / 2 );
		stringInsertSS( sbAmountWithAsterisks, sAsterisks->string, "  " );
		stringAppendSS( sbAmountWithAsterisks, "  ", sAsterisks->string );
	}

GET_MEMO:	
	sMemo = getTextInput( "Enter Memo", &skip, &cancel, sbError );
	if( cancel || skip ){
		return;
	}
	if( sMemo == NULL ){
		printf( "Error getting memo input: %s\n", sbError->string );
		goto GET_MEMO;
	}

   	printf( "\nCheck #%s will be printed as follows:\n", sCheckNumber->string );
	printf( "----------------------------------------------\n");	
	printf( "    Date: %s\n", sDate->string );
	printf( "   Payee: %s\n", sPayee->string );
	printf( "  Amount: %s\n", sAmount->string );
	printf( "In Words: %s\n", sbAmountInWords->string );
	printf( "    Memo: %s\n", sMemo->string );
	printf( "----------------------------------------------\n");	
	bool zPrint = getBooleanInput( "Ok to print?", &skip, &cancel, sbError );
	if( cancel ) return;
	if( skip ) return;
	if( ! zPrint ) return;

	// create PCL job form
   	unsigned int dpi          = 600;
	PAGE_SIZE ePageSize       = Com_10;
	PAPER_SOURCE ePaperSource = FEED_feeder;
	ORIENTATION eOrientation  = ORIENTATION_Landscape;
	char* sJobName            = stringCreateValueF( "Print Check #%s", sCheckNumber->string )->string;
	char* sDisplayText        = stringCreateValueF( "Printing Check #%s", sCheckNumber->string )->string;
	PCL_Form* pcl = createPCL( sJobName, sDisplayText, ePageSize, ePaperSource, eOrientation, dpi, NULL );
	if( pcl == NULL ){
		printf( "failed to create PCL object for envelope\n" );
		return;
	}

	// set the font for the date, which will be the default going forwards
	FONT_DEFINITION* font = (FONT_DEFINITION*) malloc( sizeof( FONT_DEFINITION ) );
	if( font == NULL ){
		printf( "Failed to allocate memory for font definition\n" );
	   	return;
	}
	font->eSymbolSet = Roman_8;
	font->eSpacing   = SPACING_Proportional;
	font->iHeight    = 1200;
	font->eStyle     = Upright;
	font->eStroke    = Bold;
	font->eFamily    = CG_Omega;
	
	// only the first text element needs to define the font, the others will use it
	if( ! addTextElement( pcl, 4050, 670, sDate->string, font ) ){
		printf( "failed to add date text element" );
		return;
	}

	if( ! addTextElement( pcl, 1260, 870, sPayee->string, NULL ) ){
		printf( "failed to add payee text element" );
		return;
	}
	
	if( ! addTextElement( pcl, 4730, 900, sAmount->string, NULL ) ){
		printf( "failed to add amount text element" );
		return;
	}

	if( ! addTextElement( pcl, 840, 1055, sbAmountWithAsterisks->string, NULL ) ){
		printf( "failed to add amount with asterisks text element" );
		return;
	}

	if( ! addTextElement( pcl, 1000, 1400, sMemo->string, NULL ) ){
		printf( "failed to add memo text element" );
		return;
	}

	char* pcl_buffer = createJob_PCL_Form( pcl );
	size_t lenBuffer = 0;
	while( pcl_buffer[ lenBuffer++ ] != 0 );

/* debugging
	printf( "len buffer: %zu\n", lenBuffer );
	printPCL( pcl_buffer );
	// populate buffer with print job content
*/
	
TRY_AGAIN:
	if( printBuffer2IP( sPrinterIP, pcl_buffer, lenBuffer, sbError ) ){
		printf( "check content sent to printer\n" );
		if( writeCheckRecord( sDataFilePath, sCheckNumber, sDate, sPayee, sAmount, sbAmountInWords, sMemo, sbError ) ){
			printf( "check written to log %s\n", sDataFilePath );
		} else {
			printf( "failed to print check to log: %s\n", sbError->string );
		}
	} else {
		printf( "failed to print check: %s\n", sbError->string );
		sbError->length = 0; // reset error
		if( getBooleanInput( "Try again?", &skip, &cancel, sbError ) ) goto TRY_AGAIN;
	}
	stringFreeBuffer( sbError );
	free( pcl_buffer );
	
	return;
}

/* OLD CHECK PRINT LOGIC 
 *	
	STRBUF* buffer = stringCreateBuffer_capacity( 10000 );
	stringAppend( buffer, "\033%-12345X@PJL JOB NAME = \"Check Printout\"\n" );
	stringAppend( buffer, "@PJL ENTER LANGUAGE = PCL\n" );
	stringAppend( buffer, "\033E\033&l81A\033&l0H\n" );
	stringAppendSS( buffer, "\033*p2050x325Y", sDate->string );
	stringAppendSS( buffer, "\033*p630x415Y", sPayee->string );
	stringAppendSS( buffer, "\033*p2365x450Y", sAmount->string );
	stringAppendSS( buffer, "\033*p420x525Y", sbAmountWithAsterisks->string );
	stringAppendSS( buffer, "\033*p500x690Y", sMemo->string );
	stringAppend( buffer, "\033&l1T\n" );
	stringAppend( buffer, "\033E\n" );
	stringAppend( buffer, "\033%-12345X@PJL EOJ NAME = \"Check Printout\"\n" );
	stringAppend( buffer, "\033%-12345X\n" );
	*/

static void printPCL( char* sPCL ){
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

bool writeCheckRecord( char *sDataFilePath, SVALUE* sCheckNumber, SVALUE* sDate, SVALUE* sPayee, SVALUE* sAmount, STRBUF* sbAmountWords, SVALUE* sMemo, STRBUF* sbError ){
	char* s;
	bool skip = false;
	bool cancel = false;
	

	// make sure file exists
	if( access( sDataFilePath, F_OK ) == -1 ){
		STRBUF* sbPrompt = stringCreateBuffer_capacity( 10000 );
		stringAppend( sbPrompt, "Log file does not exist (" );
		stringAppend( sbPrompt, sDataFilePath );
		stringAppend( sbPrompt, "), do you want to create it?" );
		bool zCreateLogFile = getBooleanInput( sbPrompt->string, &skip, &cancel, sbError );
		if( zCreateLogFile ){
			int fNewLogFile = open( sDataFilePath, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH );
			if( fNewLogFile == -1 ){
				stringAppend( sbError, "Failed to create file: " );
				stringAppend( sbError, sDataFilePath );
				return false;
			} else {
				printf( "Created file %s\n", sDataFilePath );
			}
		}
		stringAppend( sbError, "User cancelled (no log file)" );
		return false;
	}

	// locate and verify the data file
	if( access( sDataFilePath, R_OK | W_OK ) == -1 ){
		stringAppend( sbError, "current process does not have read/write permission for data file: " );
		stringAppend( sbError, sDataFilePath );
		return false;
	}

	// open the data file and read its contents into program memory
	int fdData = open( sDataFilePath, O_RDWR | O_APPEND );
	if( fdData == -1 ){
		stringAppend( sbError, "failed to open data file for read/write/append [" );
		stringAppend( sbError, sDataFilePath );
		stringAppend( sbError, "]: " );
		stringAppend( sbError, strerror( errno ) );
		return false;
	}

	char sCurrentTime[ 100 ];
	getCurrentTimeString( sCurrentTime );

	write( fdData, sCurrentTime, 10 );
	write( fdData, "\t", 1 );
	write( fdData, sCheckNumber->string, sCheckNumber->length );
	write( fdData, "\t", 1 );
	write( fdData, sDate->string, sDate->length );
	write( fdData, "\t", 1 );
	write( fdData, sPayee->string, sPayee->length );
	write( fdData, "\t", 1 );
	write( fdData, sAmount->string, sAmount->length );
	write( fdData, "\t", 1 );
	write( fdData, sbAmountWords->string, sbAmountWords->length );
	write( fdData, "\t", 1 );
	write( fdData, sMemo->string, sMemo->length );
	write( fdData, "\n", 1 );

	close( fdData );
	return true;
}

void getCurrentTimeString( char* time_string ){
	time_t raw_time;
	time( &raw_time );
	struct tm *timeinfo = localtime( &raw_time );
	size_t length = strftime( time_string, 100, "%F", timeinfo ); // note that length includes terminating null
	return;
}

void getCurrentDate_SpelledOut( char* time_string ){
	size_t length; 
	time_t raw_time;
	time( &raw_time );
	struct tm *timeinfo = localtime( &raw_time );
	if( timeinfo->tm_mday < 10 ){
		length = strftime( time_string, 100, "%B%e, %Y", timeinfo ); // note that length includes terminating null
	} else {
		length = strftime( time_string, 100, "%B %e, %Y", timeinfo ); // note that length includes terminating null
	}
	return;
}

STRBUF* getWordsForAmount( SVALUE* amount, STRBUF* sbError ){
	int decimal_values[ 7 ]; // one-based, up to six digits (hundreds of thousands)
	if( amount == NULL ){
		stringAppend( sbError, "internal error, amount is missing" );
		return NULL;
	}
	STRBUF* buffer = stringCreateBuffer();
	int pos = -1;
	int xDecimal = 0;
	int posCents = 0;
	while( true ){
		pos++;
		if( pos == amount->length ) break;
		char c = amount->string[ pos ];
		if( c == '0' && xDecimal == 0 ) continue; // ignore leading zeroes
		if( c == ',' ) continue; // ignore commas
		if( c == '.' ){
			posCents = pos + 1;
			break;
		}
		if( c < '0' && c > '9' ){
			stringAppend( sbError, "invalid character in amount" );
			return NULL;
		}
		xDecimal++;
		decimal_values[ xDecimal ] = c - '0';
	}
	if( xDecimal == 0 && posCents == 0 ){
		stringAppend( sbError, "no dollars and no cents in amount" );
		return NULL;
	}
	if( xDecimal > 6 ){
		stringAppend( sbError, "unable to print checks for more than $999,999.99" );
		return NULL;
	}
	if( xDecimal == 6 ){
		stringAppendSS( buffer, digit_names[ decimal_values[ 1 ] ], " HUNDRED" );
		if( decimal_values[ 2 ] > 0 || decimal_values[ 3 ] > 0 ) stringAppend( buffer, " " );
	}
	if( xDecimal >= 5 && decimal_values[ xDecimal - 4 ] == 1 ){
		stringAppend( buffer, teens_names[ decimal_values[ xDecimal - 3 ]] );
	} else {
		if( xDecimal >= 5 && decimal_values[ xDecimal - 4 ] > 0 ){
			stringAppend( buffer, tens_names[ decimal_values[ xDecimal - 4 ]] );
			if( decimal_values[ xDecimal - 3 ] > 0 ) stringAppend( buffer, "-" );
		}
		if( xDecimal >= 4 && decimal_values[ xDecimal - 3 ] > 0 ) stringAppend( buffer, digit_names[ decimal_values[ xDecimal - 3 ]] );
	}
	if( xDecimal > 3 ){
		stringAppend( buffer, " THOUSAND" );
		if( decimal_values[ xDecimal - 2 ] > 0 || decimal_values[ xDecimal - 1 ] > 0 || decimal_values[ xDecimal ] > 0 ){
			stringAppend( buffer, ", " );
		}
	}
	if( xDecimal > 2 && decimal_values[ xDecimal - 2 ] > 0 ){
		stringAppendSS( buffer, digit_names[ decimal_values[ xDecimal - 2 ] ], " HUNDRED" );
		if( decimal_values[ xDecimal - 1 ] > 0 || decimal_values[ xDecimal ] ) stringAppend( buffer, " " );
	}
	if( xDecimal > 1 && decimal_values[ xDecimal - 1 ] == 1 ){
		stringAppend( buffer, teens_names[ decimal_values[ xDecimal ]] );
	} else {
		if( xDecimal > 1 && decimal_values[ xDecimal - 1 ] > 0 ){
			stringAppend( buffer, tens_names[ decimal_values[ xDecimal - 1 ]] );
			if( decimal_values[ xDecimal ] > 0 ) stringAppend( buffer, "-" );
		}
		if( decimal_values[ xDecimal ] > 0 ) stringAppend( buffer, digit_names[ decimal_values[ xDecimal ]] );
	}
	if( xDecimal > 0 ) stringAppend( buffer, " and " );
	if( posCents > 0 ){
		stringAppendSS( buffer, amount->string + posCents, "/100" );
	} else {
		stringAppend( buffer, "00/100" );
	}
	return buffer;
}

void testWordsForAmount(){
	STRBUF* sbError = stringCreateBuffer();
	SVALUE* sAmount = stringCreateValue( "24,000" );
	STRBUF* sbAmountInWords = getWordsForAmount( sAmount, sbError );
	printf( "24,000 %s\n", sbAmountInWords->string );
	sAmount = stringCreateValue( "25,000" );
	sbAmountInWords = getWordsForAmount( sAmount, sbError );
	printf( "25,000 %s\n", sbAmountInWords->string );
	sAmount = stringCreateValue( "23,000" );
	sbAmountInWords = getWordsForAmount( sAmount, sbError );
	printf( "23,000 %s\n", sbAmountInWords->string );
	sAmount = stringCreateValue( "34,000" );
	sbAmountInWords = getWordsForAmount( sAmount, sbError );
	printf( "34,000 %s\n", sbAmountInWords->string );
	sAmount = stringCreateValue( "35" );
	sbAmountInWords = getWordsForAmount( sAmount, sbError );
	printf( "35 %s\n", sbAmountInWords->string );
	sAmount = stringCreateValue( "60001.01" );
	sbAmountInWords = getWordsForAmount( sAmount, sbError );
	printf( "60001.01 %s\n", sbAmountInWords->string );
	sAmount = stringCreateValue( "75,063" );
	sbAmountInWords = getWordsForAmount( sAmount, sbError );
	printf( "75,063 %s\n", sbAmountInWords->string );
	sAmount = stringCreateValue( "205,003" );
	sbAmountInWords = getWordsForAmount( sAmount, sbError );
	printf( "205,003 %s\n", sbAmountInWords->string );
}
