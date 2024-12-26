#include "jcshared.h"

struct period_data {
	void

void doPayroll( SVALUE* sPrinterIP );
int getDaysInMonth( int Year, int Month0 );

const int START_Day = 158;  // June 7, 2021
const int START_Year = 2021;

int main () {
   time_t rawtime;
   struct tm *info;
   time( &rawtime );
   info = localtime( &rawtime );
   printf("Current local time and date: %s", asctime(info));
   return(0);
}

bool loadPayrollData( SVALUE* sPrinterIP, char* content_buffer, size_t ctBytesToPrint, STRBUF* sbError ){
int getDaysInMonth( int Year, int Month0 ){
	switch( Month0 ){
		case 0: return 31;
		case 1:
			if( Year % 400 == 0 ) return 29;
			if( Year % 100 == 0 ) return 28;
			if( Year % 4 == 0 ) return 29;
			return 28;
		case 2: return 31;
		case 3: return 30;
		case 4: return 31;
		case 5: return 30;
		case 6: return 31;
		case 7: return 31;
		case 8: return 30;
		case 9: return 31;
		case 10: return 30;
		case 11: return 31;
		default: return 0;
	}
}	
			
SVALUE* getDateString( int Year, int Month0, int Day, int dow0 ){
	STRBUF* buffer = stringCreateBuffer( 80 );
	switch( dow0 ){
		case 0: stringAppend( buffer, "Sunday" ); break;
		case 1: stringAppend( buffer, "Monday" ); break;
		case 2: stringAppend( buffer, "Tuesday" ); break;
		case 3: stringAppend( buffer, "Wednesday" ); break;
		case 4: stringAppend( buffer, "Thursday" ); break;
		case 5: stringAppend( buffer, "Friday" ); break;
		case 6: stringAppend( buffer, "Saturday" ); break;
		default: stringAppend( buffer, "???" ); break; 
	}
	stringAppend( buffer, ", " );
	if( Day < 10 ){
		stringAppendChar( buffer, (char)( Day + '0' );
	} else {
		stringAppendChar( buffer, (char)( Day / 10 + '0' );
		stringAppendChar( buffer, (char)( Day - Day / 10 + '0' );
	}	
	switch( Month0 ){
		case 0: stringAppend( buffer, "January" ); break;
		case 1: stringAppend( buffer, "February" ); break;
		case 2: stringAppend( buffer, "March" ); break;
		case 3: stringAppend( buffer, "April" ); break;
		case 4: stringAppend( buffer, "May" ); break;
		case 5: stringAppend( buffer, "June" ); break;
		case 6: stringAppend( buffer, "July" ); break;
		case 7: stringAppend( buffer, "August" ); break;
		case 8: stringAppend( buffer, "September" ); break;
		case 9: stringAppend( buffer, "October" ); break;
		case 10: stringAppend( buffer, "November" ); break;
		case 11: stringAppend( buffer, "December" ); break;
		default: stringAppend( buffer, "???" ); break;
	}
	stringAppendChar( buffer, ' ' );
	stringAppendInt( buffer, Year );
	SVALUE* date_string = stringCreateFromBuffer( buffer );
	stringFreeBuffer( buffer );
	return date_string;
}

void doPayroll( SVALUE* sPrinterIP ){

	if( is_valid_pointer( sPrinterIP ) ){
		printf( "payroll begin: %p\n", sPrinterIP );
	} else {	
		printf( "unable to begin payroll, printer IP is missing or invalid: %p\n", sPrinterIP );
		return;
	}	

	// get information from user
	STRBUF* sbError = stringCreateBuffer();
	bool skip = false;
	bool cancel = false;

	// get the current pay period, ending day
	struct tm *timeLocal;
	time_t timeNow;
	time( &timeNow );
	timeLocal = localtime( &timeNow );
	printf("Current local time and date: %s", asctime(info));
	unsigned int iPayPeriod_Year_end  = timeLocal->tm_year + 1900;
	unsigned int iPayPeriod_Month_end = timeLocal->tm_month;
	unsigned int iPayPeriod_Day_end   = timeLocal->tm_mday;
	iPayPeriod_Day_end -= timeLocal-tm_wday; // change the date to the previous Sunday, if it is not Sunday	
	if( iPayPeriod_Day_end < 1 ){ // then go to the previous month
		if( iPayPeriod_Month_end == 0 ){ // then go to the previous year
			iPayPeriod_Year_end--;
			iPayperiod_Month_end = 11;
			iPayPeriod_Day_end = iPayPeriodDay_end + 31;
		} else {
			iPayPeriod_Month_end--;
			iPayPeriod_Day_end = iPayPeriodDay_end + getDaysInMonth( iPayPeriod_Year_end, iPayPeriod_Month_end );
		}
	}
		
	// get the current pay period, beginning day
	unsigned int iPayPeriod_Year_begin  = iPayPeriod_Year_end;
	unsigned int iPayPeriod_Month_begin = iPayPeriod_Month_end;
	unsigned int iPayPeriod_Day_begin   = iPayPeriod_Day_end - 13;  // two week pay period
	if( iPayPeriod_Day_begin < 1 ){ // then go to the previous month
		if( iPayPeriod_Month_begin == 0 ){ // then go to the previous year
			iPayPeriod_Year_begin--;
			iPayperiod_Month_begin = 11;
			iPayPeriod_Day_begin = iPayPeriodDay_begin + 31;
		} else {
			iPayPeriod_Month_begin--;
			iPayPeriod_Day_begin = iPayPeriodDay_begin + getDaysInMonth( iPayPeriod_Year_begin, iPayPeriod_Month_begin );
		}
	}
	SVALUE* sPayPeriod_begin = getDateString( iPayPeriod_Year_begin, iPayPeriod_Month_begin, iPayPeriod_Day_begin, 0 );   // pay period always ends on a Sunday
	SVALUE* sPayPeriod_end = getDateString( iPayPeriod_Year_end, iPayPeriod_Month_end, iPayPeriod_Day_end, 1 );   // pay period always begins on a Monday
	printf( "Current pay period: %s to %s\n", sPayPeriod_begin->string, sPayPeriod_end->string );
	bool zPrint = getYesNo( "Correct?" );
	if( ! zPrint ){
		printf( "pay period adjustment not supported" );
		return;
	}
	

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

	SVALUE* sCheckNumber = getTextInput( "Enter Check Number (for log)", &skip, &cancel, sbError );
	if( cancel || skip ){
		return;
	}
	if( sCheckNumber == NULL ){
		printf( "Error getting check number input: %s\n", sbError->string );
		return;
	}

	char sCurrentDate[ 100 ];
	getCurrentDate_SpelledOut( sCurrentDate );

	SVALUE* sDate = getTextInput_Default( "Enter Date", sCurrentDate, sbError );
	if( sDate == NULL ){
		printf( "Error getting date input: %s\n", sbError->string );
		return;
	}

	SVALUE* sPayee = getTextInput( "Enter Payee", &skip, &cancel, sbError );
	if( cancel || skip ){
		return;
	}
	if( sPayee == NULL ){
		printf( "Error getting payee input: %s\n", sbError->string );
		return;
	}

	SVALUE* sAmount = getTextInput( "Enter Amount (no dollar sign)", &skip, &cancel, sbError );
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
		return;
	}

	// add asterisks to amount in words
	STRBUF* sbAmountWithAsterisks = stringCopyBuffer( sbAmountInWords );
	if( sbAmountWithAsterisks == NULL ){
		printf( "failed to copy amount buffer" );
		return;
	}
	int ctAsterisks = 75 - ( sbAmountWithAsterisks->length * 13 / 9 );
	if( ctAsterisks >= 6 ){
		ctAsterisks = ctAsterisks / 2;
		SVALUE* sAsterisks = stringCreateRepeating( '*', ctAsterisks );
		stringInsertSS( sbAmountWithAsterisks, sAsterisks->string, "  " );
		stringAppendSS( sbAmountWithAsterisks, "  ", sAsterisks->string );
	}

	SVALUE* sMemo = getTextInput( "Enter Memo", &skip, &cancel, sbError );
	if( cancel || skip ){
		return;
	}
	if( sMemo == NULL ){
		printf( "Error getting memo input: %s\n", sbError->string );
		return;
	}

   	printf( "\nCheck #%s will be printed as follows:\n", sCheckNumber->string );
	printf( "----------------------------------------------\n");	
	printf( "    Date: %s\n", sDate->string );
	printf( "   Payee: %s\n", sPayee->string );
	printf( "  Amount: %s\n", sAmount->string );
	printf( "In Words: %s\n", sbAmountInWords->string );
	printf( "    Memo: %s\n", sMemo->string );
	printf( "----------------------------------------------\n");	
	printf( " Printer: %s\n", sPrinterIP->string );
	bool zPrint = getBooleanInput( "Ok to print?", &skip, &cancel, sbError );
	if( cancel ) return;
	if( skip ) return;
	if( ! zPrint ) return;

	// populate buffer with print job content
	STRBUF* buffer = stringCreateBuffer( 10000 );
	stringAppend( buffer, "\033%-12345X@PJL JOB NAME = \"Check Printout\"\n" );
	stringAppend( buffer, "@PJL ENTER LANGUAGE = PCL\n" );
	stringAppend( buffer, "\033E\033&l81A\033&l0H\n" );
	stringAppendSS( buffer, "\033*p2050x315Y", sDate->string );
	stringAppendSS( buffer, "\033*p630x405Y", sPayee->string );
	stringAppendSS( buffer, "\033*p2365x440Y", sAmount->string );
	stringAppendSS( buffer, "\033*p420x515Y", sbAmountWithAsterisks->string );
	stringAppendSS( buffer, "\033*p500x680Y", sMemo->string );
	stringAppend( buffer, "\033&l1T\n" );
	stringAppend( buffer, "\033E\n" );
	stringAppend( buffer, "\033%-12345X@PJL EOJ NAME = \"Check Printout\"\n" );
	stringAppend( buffer, "\033%-12345X\n" );
TRY_AGAIN:
	if( printBuffer2IP( sPrinterIP, buffer->string, buffer->length, sbError ) ){
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
	stringFreeBuffer( buffer );
	
	return;
}

