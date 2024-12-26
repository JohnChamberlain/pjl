#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>      /* atoi() */
#include <time.h>       
#include <termios.h>
#include "definitions.h"
#include "jcstring.h"

char* sStatus = NULL;
static struct termios saved_tty_mode;

void interact();
void interact_search();
void interact_new();
void interact_edit();
void interact_duplicate();
void tty_save();
void tty_restore();

main(){
	tty_save();                 /* save current terminal mode */
	interact();                 /* interact with user */
	tty_restore();              /* restore terminal to the way it was */
	return;
}

void interact(){
	while( true ){
		set_terminal_raw();
		putchar( '\n' );
		if( sStatus != NULL ) printf( "%s\n", sStatus );
		printf( "(S) Search  (N) New  (E) Edit  (D) Duplicate  (Q) Quit" );
		char cInput = getchar();
		if( cInput != EOF ) printf( "   :: %c\n", cInput );
		switch( cInput ){
			case 's': interact_search(); break;
			case 'n': interact_new(); break;
			case 'e': interact_edit(); break;
			case 'd': interact_duplicate(); break;
			case 'q': return;
			case EOF: return;
		}
	}
}

void interact_search(){
}

SVALUE* getTextInput( char* prompt, bool* skip, bool* cancel, STRBUF* sbError ){
	static char buffer[ 256 ];
	unsigned int lenBuffer = 0;
	unsigned int posBuffer = 0;
	int xBuffer = 0;
	for( ; xBuffer < 256; xBuffer++ ) buffer[ xBuffer ] = 0; /* zero buffer */
	printf( "%s (ESC to cancel, TAB to skip):\n", prompt );
	while( true ){
		char cInput = getchar();
		if( cInput == 27 ){
			*cancel = true;
			return NULL;
		}
		if( cInput == '\t' ){
			*skip = true;
			return NULL;
		}
		if( cInput == 10 || cInput == 13 ){
			if( lenBuffer == 0 ){
				*cancel = true;
				return NULL; /* go back if input is blank */
			}
			SVALUE* sv = stringCreateValue( buffer );
			if( sv == NULL ){
				stringAppend( sbError, "out of memory" );
				return NULL;
			}
			return sv;
		}
		if( cInput == 8 ){  /* backspace */
			if( lenBuffer > 0 ){
				putchar( 8 );
				lenBuffer--;
				posBuffer--;
			}
			continue;
		}
		if( cInput < 32 ){ /* any control character will exit */
			*cancel = true;
			return;
		}
		if( lenBuffer >= 254 ){
			printf( "\ntoo many characters\n" );
			continue;
		}
		putchar( cInput );
		buffer[ posBuffer ] = cInput;
		lenBuffer++;
		posBuffer++;
	}
}

SVALUE* getTextChoice( char* prompt, STRLIST* listChoices, bool* skip, bool* cancel, STRBUF* sbError ){
	printf( "%s:\n", prompt );
	for( int xList = 0; xList < listChoices.size; xList++ )
		printf( "[%s] %s\n", 'A' + xList, listChoices->strings[ xList ] );
	printf( "[SPACE]  ( skip this field )\n" );
	printf( "[ESC]    ( go back )\n" );
	printf( "[ENTER]  ( enter a custom value )\n" ); 
	while( true ){
		char cInput = getchar();
		if( cInput == 32 ){ /* space bar */
			*skip = true;
			return;
		}
		if( cInput == 27 ){ /* ESC key */
			*cancel = true;
			return;
		}
		if( cInput == 10 || cInput == 13 ){
			SVALUE* svInput = getTextInput( prompt, &skip, &cancel, sbError );
			if( svInput == NULL ){
				if( cancel ){
					printf( "%s:\n", prompt );
					for( int xList = 0; xList < listChoices.size; xList++ )
						printf( "[%s] %s\n", 'A' + xList, listChoices->strings[ xList ] );
					printf( "[SPACE]  ( skip this field )\n" );
					printf( "[ESC]    ( go back )\n" );
					printz( "[ENTER]  ( enter a custom value )\n" ); 
					continue; /* go back to text choice */
				}
				if( ! skip )
				   	stringInsert( "error getting text input: ", sbError );
				return NULL;
			}
			return svInput;
		}
		if( cInput < 32 ) return; /* any control character will exit */
		if( cInput >= 'a' && cInput < 'a' + listChoices.size ) cInput -= ('a' - 'A');
		if( cInput >= 'A' && cInput < 'A' + listChoices.size ){
			char* sChoice = listChoices->strings[ cInput - 'A' ];
			SVALUE* svChoice = stringCreateValue( sChoice );
			if( svChoice == NULL ){
				stringAppend( sbError, "out of memory creating choice string" );
				return NULL;
			}
			return svChoice;
		}
	}
}

bool getBooleanInput( char* prompt, bool* skip, bool* cancel, STRBUF* sbError ){
	printf( "%s (enter Y/y or N/n):\n", prompt );
	printf( "[SPACE]  ( skip this field )\n" );
	printf( "[ESC]    ( go back )\n" );
	while( true ){
		char cInput = getchar();
		if( cInput == 'Y' || cInput == 'y' ) return true;
		if( cInput == 'N' || cInput == 'n' ) return false;
		if( cInput == 32 ){ /* space bar */
			*skip = true;
			return;
		}
		if( cInput == 27 ){ /* ESC key */
			*cancel = true;
			return;
		}
	}
}

unsigned int getNonNegativeInteger( char* prompt, bool* skip, bool* cancel ){
	static char buffer[ 256 ];
	unsigned int lenBuffer = 0;
	unsigned int posBuffer = 0;
	int xBuffer = 0;
	for( ; xBuffer < 256; xBuffer++ ) buffer[ xBuffer ] = 0; /* zero buffer */
	printf( "%s (ESC to cancel, TAB to skip):\n", prompt );
	while( true ){
		char cInput = getchar();
		if( cInput == 27 ){
			*cancel = true;
			return NULL;
		}
		if( cInput == '\t' ){
			*skip = true;
			return NULL;
		}
		if( cInput == 10 || cInput == 13 ){
			if( lenBuffer == 0 ){
				*cancel = true;
				return NULL; /* go back if input is blank */
			}
			return atoi( buffer );
		}
		if( cInput == 8 ){  /* backspace */
			if( lenBuffer > 0 ){
				putchar( 8 );
				lenBuffer--;
				posBuffer--;
			}
			continue;
		}
		if( cInput < 32 ){ /* any control character will exit */
			*cancel = true;
			return;
		}
		if( lenBuffer >= 9 ){
			printf( "\ntoo many characters\n" );
			continue;
		}
		if( cInput < '0' || cInput > '9' ) continue; /* ignore non-numeric */
		putchar( cInput );
		buffer[ posBuffer ] = cInput;
		lenBuffer++;
		posBuffer++;
	}
}


double getDouble( char* prompt, bool* skip, bool* cancel ){
	static char buffer[ 256 ];
	unsigned int lenBuffer = 0;
	unsigned int posBuffer = 0;
	int xBuffer = 0;
	for( ; xBuffer < 256; xBuffer++ ) buffer[ xBuffer ] = 0; /* zero buffer */
	printf( "%s (ESC to cancel, TAB to skip):\n", prompt );
	while( true ){
		char cInput = getchar();
		if( cInput == 27 ){
			*cancel = true;
			return NULL;
		}
		if( cInput == '\t' ){
			*skip = true;
			return NULL;
		}
		if( cInput == 10 || cInput == 13 ){
			if( lenBuffer == 0 ){
				*cancel = true;
				return NULL; /* go back if input is blank */
			}
			return atof( buffer );
		}
		if( cInput == 8 ){  /* backspace */
			if( lenBuffer > 0 ){
				putchar( 8 );
				lenBuffer--;
				posBuffer--;
			}
			continue;
		}
		if( cInput < 32 ){ /* any control character will exit */
			*cancel = true;
			return;
		}
		if( lenBuffer >= 9 ){
			printf( "\ntoo many characters\n" );
			continue;
		}
		if( cInput < '0' || cInput > '9' ) continue; /* ignore non-numeric */
		putchar( cInput );
		buffer[ posBuffer ] = cInput;
		lenBuffer++;
		posBuffer++;
	}
}

SVALUE* getTextWithNewlines( char* prompt, bool* skip, bool* cancel, STRBUF* sbError ){
	static char buffer[ 256 ];
	unsigned int lenBuffer = 0;
	unsigned int posBuffer = 0;
	int xBuffer = 0;
	for( ; xBuffer < 256; xBuffer++ ) buffer[ xBuffer ] = 0; /* zero buffer */
	printf( "%s (ESC to cancel, TAB to skip, | to insert new line):\n", prompt );
	while( true ){
		char cInput = getchar();
		if( cInput == 27 ){
			*cancel = true;
			return NULL;
		}
		if( cInput == '\t' ){
			*skip = true;
			return NULL;
		}
		if( cInput == '|' ){
			putchar( '\n' );
			buffer[ posBuffer ] = '\n';
			posBuffer++;
			lenBuffer++;
			continue;
		}
		if( cInput == 10 || cInput == 13 ){
			if( lenBuffer == 0 ){
				*cancel = true;
				return NULL; /* go back if input is blank */
			}
			SVALUE* sv = stringCreateValue( buffer );
			if( sv == NULL ){
				stringAppend( sbError, "out of memory" );
				return NULL;
			}
			return sv;
		}
		if( cInput == 8 ){  /* backspace */
			if( lenBuffer > 0 ){
				putchar( 8 );
				lenBuffer--;
				posBuffer--;
			}
			continue;
		}
		if( cInput < 32 ){ /* any control character will exit */
			*cancel = true;
			return;
		}
		if( lenBuffer >= 254 ){
			printf( "\ntoo many characters\n" );
			continue;
		}
		putchar( cInput );
		buffer[ posBuffer ] = cInput;
		lenBuffer++;
		posBuffer++;
	}
}

AUTHORSHIP* getAuthorship( char* prompt, bool* skip, bool* cancel, STRBUF* sbError ){
	bool zTabbing = false;
	static char buffer[ 256 ];
	unsigned int lenBuffer = 0;
	unsigned int posBuffer = 0;
	unsigned int iOccurrence = 0;
	AUTHOR* pAuthor= NULL;
	int xBuffer = 0;
	for( ; xBuffer < 256; xBuffer++ ) buffer[ xBuffer ] = 0; /* zero buffer */
	printf( "%s (ENTER to enter an author, ESC to cancel, TAB to autocomplete, / to add role, * when done):\n", prompt );
	while( true ){
		char cInput = getchar();
		if( cInput == 27 ){

			if( zTabbing ){ /* then leave tab mode */
				size_t xLine;
				for( xLine = pAuthorExisting->length; xLine > 0; xLine-- ) printf( "\b" ); /* erase existing name */
				for( posBuffer = 0; posBuffer < lenBuffer; posBuffer++ ) printf( "%c", buffer[ posBuffer ] ); /* write buffer as last it was */
				zTabbing = false;
				pAuthor = NULL;
			} else {
				*cancel = true;
				return NULL;
			}
		} else 
		if( cInput == '*' ){
			*skip = true;
			return NULL;
		}
		if( cInput == '\t' ){
			if( zTabbing ) iOccurrence++; /* if we are already tabbing, go to next author */
			AUTHOR* pAuthorExisting = getAuthorByName( buffer, lenBuffer, &iOccurrence );
			if( pAuthorExisting == NULL ){
				printf( "\a" ); /* beep */
				iOccurrence = 0;
			} else {
				if( zTabbing ){
					for( xLine = pAuthor->length; xLine > 0; xLine-- ) printf( "\b" ); /* erase existing name */
				} else {
					for( xBuffer = lenBuffer; xBuffer > 0; xBuffer-- ) printf( "\b" ); /* erase custom name */
				}
				for( ; xBuffer < pAuthorExisting->length; xBuffer++ ){
					printf( "%c", pAuthorExisting->sValue[ xBuffer ] );
					buffer[ xBuffer ] = pAuthorExisting->sValue[ xBuffer ] );
				}
				lenBuffer = pAuthor->length;
				pAuthor = pAuthorExisting;
				zTabbling = true;
			}
		}
		if( cInput == 10 || cInput == 13 ){
			if( pAuthor == NULL && lenBuffer == 0 ){
				*cancel = true;
				return NULL; /* go back if input is blank */
			}
			if( pAuthor == NULL ){ /* then the author is custom/new */
				SVALUE* sv = stringCreateValue( buffer );
				if( sv == NULL ){
					stringAppend( sbError, "out of memory" );
					return NULL;
				}
				SVALUE** listSV = stringSplitAndTrim( sv, '/' );
				SVALUE svAuthor = NULL;
				SVALUE svRole   = NULL;
				if( listSV[ 0 ] == NULL ){
					stringAppend( sbError, "author string is blank" );
					return NULL;
				}
				svAuthor = listSV[ 0 ];
				svRole = listSV[ 1 ];
				free( listSV );
				return createAuthorship( pAuthor, svAuthor, svRole );
			} else { /* using an existing author */
				return createAuthorship( pAuthor, NULL, NULL ); /* todo support roles for known authors */
			}
			return sv;
		}
		if( cInput == 8 ){  /* backspace */
			if( lenBuffer > 0 ){
				putchar( 8 );
				lenBuffer--;
				posBuffer--;
			}
			continue;
		}
		if( cInput < 32 ){ /* any control character will exit */
			cancel = true;
			return;
		}
		if( lenBuffer >= 254 ){
			printf( "\ntoo many characters\n" );
			continue;
		}
		putchar( cInput );
		buffer[ posBuffer ] = cInput;
		lenBuffer++;
		posBuffer++;
	}

}

PUBLICATION* getPublication( char* prompt, bool* skip, bool* cancel, STRBUF* sbError ){
	bool zTabbing = false;
	static char buffer[ 256 ];
	unsigned int lenBuffer = 0;
	unsigned int posBuffer = 0;
	unsigned int iOccurrence = 0;
	PUBLICATION* pPublication= NULL;
	int xBuffer = 0;
	for( ; xBuffer < 256; xBuffer++ ) buffer[ xBuffer ] = 0; /* zero buffer */
	printf( "%s (ENTER to complete record, ESC to cancel, TAB to autocomplete, / to add role, * when done):\n", prompt );
	while( true ){
		char cInput = getchar();
		if( cInput == 27 ){

			if( zTabbing ){ /* then leave tab mode */
				size_t xLine;
				for( xLine = pPublicationExisting->length; xLine > 0; xLine-- ) printf( "\b" ); /* erase existing name */
				for( posBuffer = 0; posBuffer < lenBuffer; posBuffer++ ) printf( "%c", buffer[ posBuffer ] ); /* write buffer as last it was */
				zTabbing = false;
				pAuthor = NULL;
			} else {
				*cancel = true;
				return NULL;
			}
		} else 
		if( cInput == '*' ){
			*skip = true;
			return NULL;
		}
		if( cInput == '\t' ){
			if( zTabbing ) iOccurrence++; /* if we are already tabbing, go to next author */
			AUTHOR* pAuthorExisting = getAuthorByName( buffer, lenBuffer, &iOccurrence );
			if( pAuthorExisting == NULL ){
				printf( "\a" ); /* beep */
				iOccurrence = 0;
			} else {
				if( zTabbing ){
					for( xLine = pAuthor->length; xLine > 0; xLine-- ) printf( "\b" ); /* erase existing name */
				} else {
					for( xBuffer = lenBuffer; xBuffer > 0; xBuffer-- ) printf( "\b" ); /* erase custom name */
				}
				for( ; xBuffer < pAuthorExisting->length; xBuffer++ ){
					printf( "%c", pAuthorExisting->sValue[ xBuffer ] );
					buffer[ xBuffer ] = pAuthorExisting->sValue[ xBuffer ] );
				}
				lenBuffer = pAuthor->length;
				pAuthor = pAuthorExisting;
				zTabbling = true;
			}
		}
		if( cInput == 10 || cInput == 13 ){
			if( pAuthor == NULL && lenBuffer == 0 ){
				*cancel = true;
				return NULL; /* go back if input is blank */
			}
			if( pAuthor == NULL ){ /* then the author is custom/new */
				SVALUE* sv = stringCreateValue( buffer );
				if( sv == NULL ){
					stringAppend( sbError, "out of memory" );
					return NULL;
				}
				SVALUE** listSV = stringSplitAndTrim( sv, '/' );
				SVALUE svAuthor = NULL;
				SVALUE svRole   = NULL;
				if( listSV[ 0 ] == NULL ){
					stringAppend( sbError, "author string is blank" );
					return NULL;
				}
				svAuthor = listSV[ 0 ];
				svRole = listSV[ 1 ];
				free( listSV );
				return createAuthorship( pAuthor, svAuthor, svRole );
			} else { /* using an existing author */
				return createAuthorship( pAuthor, NULL, NULL ); /* todo support roles for known authors */
			}
			return sv;
		}
		if( cInput == 8 ){  /* backspace */
			if( lenBuffer > 0 ){
				putchar( 8 );
				lenBuffer--;
				posBuffer--;
			}
			continue;
		}
		if( cInput < 32 ){ /* any control character will exit */
			cancel = true;
			return;
		}
		if( lenBuffer >= 254 ){
			printf( "\ntoo many characters\n" );
			continue;
		}
		putchar( cInput );
		buffer[ posBuffer ] = cInput;
		lenBuffer++;
		posBuffer++;
	}


}

VENDOR* getVendor( char* prompt, bool* skip, bool* cancel, STRBUF* sbError ){
}

void deleteMedia( MEDIA* media ){
}

void writeAuthorshipNumberInPrompt( unsigned int iValue, char* sPrompt, unsigned int offset ){
	if( iValue < 10 ){
		sPrompt[ offset ] = iValue + '0';
	} else if( iValue < 100 ) {
		sPrompt[ offset ] = ( iValue - iValue % 10 )/10 + '0';
		sPrompt[ ++offset ] = iValue / 10 + '0';
	} else {
		sPrompt[ offset ] = ( iValue - iValue % 100 )/100 + '0';
		sPrompt[ ++offset ] = ( iValue - iValue % 10 )/10 + '0';
		sPrompt[ ++offset ] = iValue % 10 + '0';
	}
	sPrompt[ ++offset ] = ']';	
}

void interact_new(){
	sStatus = "[new record]";
	STRBUF* sbError = stringCreateBuffer();
	MEDIA* newMedia = malloc( sizeof( MEDIA ) );
	if( newMedia == NULL ){
		sStatus = "Error: out of memory allocating new media record.";
		return;
	}
	bool skip = false;
	bool cancel = false;

	/* Type */
	while( true ){
		char* sNewMediaType = getTextChoice( "Select Media Type", listMediaType, &skip, &cancel, sbError );
		if( cancel ){
			deleteMedia( newMedia );
			return;
		}
		if( skip ){
			newMedia.media_type.length = 0;
			break;
		}
		if( sNewMediaType == NULL ){
			sStatus = sbError.string;
			continue;
		}
		stringSetValue( newMedia.media_type, sNewMediaType );
		break;
	}

	/* Title */
	skip = false;
	while( true ){
		char* sNewTitle = getTextWithNewlines( "Enter Title", &skip, &cancel, sbError );
		if( cancel ){
			deleteMedia( newMedia );
			return;
		}
		if( skip ){
			newMedia.title.length = 0;
			break;
		}
		if( sNewTitle == NULL ){
			sStatus = sbError.string;
			continue;
		}
		stringSetValue( newMedia.title, sNewTitle );
		break;
	}

	/* Authorship */
	unsigned int iAuthorNumber = 1;
	char* sAuthorship_PREAMBLE = "Enter author [????";
	char* sAuthorshipPrompt = malloc( 40 );
	if( sAuthorshipPrompt == NULL ){
		sStatus = "Out of memory on authorship prompt";
		return NULL;
	}
	for( int xPrompt = 0; xPrompt < 14; xPrompt++ ) sAuthorshipPrompt[ xPrompt ] = sAuthorship_PREAMBLE[ xPrompt ];
	skip = false;
	while( true ){
		if( iAuthorNumber > 999 ) break; /* max authors */
		writeAuthorshipNumberInPrompt( iAuthorNumber, sAuthorshipPrompt, 14 );
		AUTHORSHIP* authorship = getAuthorship( sAuthorshipPrompt, &skip, &cancel, sbError );
		if( cancel ){
			deleteMedia( newMedia );
			return;
		}
		if( skip ) break;
		if( authorship == NULL ){
			sStatus = sbError.string;
			continue;
		}
		if( newMedia.authors == NULL ){
			newMedia.authors = malloc( sizeof( AUTHORSHIP* ) );
			if( newMedia.authors == NULL ){
				sStatus = "Out of memory on authorship add";
				return NULL;
			}
		} else {
			AUTHORSHIP** newAuthors = realloc( newMedia.authors, sizeof( AUTHORSHIP* ) * (iAuthorNumber + 1) );
			if( newAuthors == NULL ){
				sStatus = "Out of memory on authorship reallocation";
				return NULL;
			}
			newMedia.authors = newAuthors;
		}
		newMedia.authors[ iAuthorNumber ] = authorship;
		iAuthorNumber++;
	}
	newMedia.author_count = iAuthorNumber;	

	/* Publication */
	skip = false;
	while( true ){
		PUBLICATION* publication = getPublication( "Enter publisher", &skip, &cancel, sbError );
		if( cancel ){
			deleteMedia( newMedia );
			return;
		}
		if( skip ) break;
		if( publication == NULL ){
			sStatus = sbError.string;
			continue;
		}
		newMedia.publication = publication;
		break;
	}

	/* Edition */
	skip = false;
	while( true ){
		char* sNewEdition = getTextInput( "Enter Edition", &skip, &cancel, sbError );
		if( cancel ){
			deleteMedia( newMedia );
			return;
		}
		if( skip ){
			newMedia.edition.length = 0;
			break;
		}
		if( sNewEdition == NULL ){
			sStatus = sbError.string;
			continue;
		}
		stringSetValue( newMedia.edition, sNewEdition );
		break;
	}

	/* Printing */
	skip = false;
	while( true ){
		char* sNewPrinting = getTextInput( "Enter Printing", &skip, &cancel, sbError );
		if( cancel ){
			deleteMedia( newMedia );
			return;
		}
		if( skip ){
			newMedia.printing.length = 0;
			break;
		}
		if( sNewPrinting == NULL ){
			sStatus = sbError.string;
			continue;
		}
		stringSetValue( newMedia.printing, sNewPrinting );
		break;
	}

	/* LOC */
	skip = false;
	while( true ){
		char* sNewLOC = getTextInput( "Enter LOC", &skip, &cancel, sbError );
		if( cancel ){
			deleteMedia( newMedia );
			return;
		}
		if( skip ){
			newMedia.loc.length = 0;
			break;
		}
		if( sNewLOC == NULL ){
			sStatus = sbError.string;
			continue;
		}
		stringSetValue( newMedia.loc, sNewLOC );
		break;
	}

	/* ISBN */
	skip = false;
	while( true ){
		char* sNewISBN = getTextInput( "Enter ISBN", &skip, &cancel, sbError );
		if( cancel ){
			deleteMedia( newMedia );
			return;
		}
		if( skip ){
			newMedia.isbn.length = 0;
			break;
		}
		if( sNewISBN == NULL ){
			sStatus = sbError.string;
			continue;
		}
		stringSetValue( newMedia.isbn, sNewISBN );
		break;
	}

	/* Subject */
	skip = false;
	while( true ){
		char* sNewSubject = getTextInput( "Enter Subject", &skip, &cancel, sbError );
		if( cancel ){
			deleteMedia( newMedia );
			return;
		}
		if( skip ){
			newMedia.subject.length = 0;
			break;
		}
		if( sNewSubject == NULL ){
			sStatus = sbError.string;
			continue;
		}
		stringSetValue( newMedia.subject, sNewSubject );
		break;
	}

	/* Illustrations */
	skip = false;
	while( true ){
		char* sNewSubject = getTextInput( "Enter Illustrations", &skip, &cancel, sbError );
		if( cancel ){
			deleteMedia( newMedia );
			return;
		}
		if( skip ){
			newMedia.illustrations.length = 0;
			break;
		}
		if( sNewIllustrations == NULL ){
			sStatus = sbError.string;
			continue;
		}
		stringSetValue( newMedia.illustrations, sNewIllustrations );
		break;
	}

	/* Page Count */
	skip = false;
	while( true ){
		unsigned int iNewPages = getNonNegativeInteer( "Enter Page Count", &skip, &cancel );
		if( cancel ){
			deleteMedia( newMedia );
			return;
		}
		if( skip ){
			newMedia.pages = 0;
			break;
		}
		newMedia.pages = iNewPages;
		break;
	}

	/* Index */
	skip = false;
	while( true ){
		bool zNewIndex = getBoolean( "Has Index", &skip, &cancel );
		if( cancel ){
			deleteMedia( newMedia );
			return;
		}
		if( skip ){
			newMedia.pages = 0;
			break;
		}
		newMedia.index = zNewIndex;
		break;
	}

	/* Accession Number */
	unsigned int iAccessionNumber = giAccessionNumber + 1;
	newMedia.accession_number = iAccessionNumber;

	/* Accession Date */
	struct tm* tmCurrentTime;
	time_t timeCurrent;
	if( time( &timeCurrent ) == -1 ){
		sbError.string = "Unable to determine current date."
		return NULL; /* TODO convert to warning */
	}
	SVALUE* svDate = stringCreateBlank( 8 );
	if( svDate == NULL ){
		sbError.string = "Out of memory allocating date string.";
		return NULL;
	}
	tmCurrentTime = localtime( &timeCurrent );
	strftime( svDate.string, sizeof( svDate.string ), "%Y%m%d", tm );

	/* Vendor */
	skip = false;
	while( true ){
		VENDOR* vendor = getVendor( "Enter Vendor", &skip, &cancel, sbError );
		if( cancel ){
			deleteMedia( newMedia );
			return;
		}
		if( skip ) break;
		if( vendor == NULL ){
			sStatus = sbError.string;
			continue;
		}
		newMedia.vendor = vendor;
		break;
	}

	/* Price */
	skip = false;
	while( true ){
		char* sNewPrice = getTextInput( "Enter Price", &skip, &cancel, sbError );
		if( cancel ){
			deleteMedia( newMedia );
			return;
		}
		if( skip ){
			newMedia.price.length = 0;
			break;
		}
		if( sNewPrice == NULL ){
			sStatus = sbError.string;
			continue;
		}
		stringSetValue( newMedia.price, sNewPrice );
		break;
	}

	/* Purchase Date */
	skip = false;
	while( true ){
		char* sNewPurchaseDate = getTextInput( "Enter Purchase Date", &skip, &cancel, sbError );
		if( cancel ){
			deleteMedia( newMedia );
			return;
		}
		if( skip ){
			newMedia.purchase_date.length = 0;
			break;
		}
		if( sNewPurchaseDate == NULL ){
			sStatus = sbError.string;
			continue;
		}
		stringSetValue( newMedia.purchase_date, sNewPurchaseDate );
		break;
	}

	/* Binding */
	while( true ){
		char* sNewMediaBinding = getTextChoice( "Select Binding", listBinding, &skip, &cancel, sbError );
		if( cancel ){
			deleteMedia( newMedia );
			return;
		}
		if( skip ){
			newMedia.binding.length = 0;
			break;
		}
		if( sNewMediaBinding == NULL ){
			sStatus = sbError.string;
			continue;
		}
		stringSetValue( newMedia.binding, sNewMediaBinding );
		break;
	}

	/* Paper */
	while( true ){
		char* sNewMediaPaper = getTextChoice( "Select Paper", listPaper, &skip, &cancel, sbError );
		if( cancel ){
			deleteMedia( newMedia );
			return;
		}
		if( skip ){
			newMedia.paper.length = 0;
			break;
		}
		if( sNewMediaPaper == NULL ){
			sStatus = sbError.string;
			continue;
		}
		stringSetValue( newMedia.paper, sNewMediaPaper );
		break;
	}

	/* Height */
	skip = false;
	while( true ){
		char* sNewHeight = getTextInput( "Enter Height", &skip, &cancel, sbError );
		if( cancel ){
			deleteMedia( newMedia );
			return;
		}
		if( skip ){
			newMedia.height.length = 0;
			break;
		}
		if( sNewHeight == NULL ){
			sStatus = sbError.string;
			continue;
		}
		stringSetValue( newMedia.height, sNewHeight );
		break;
	}

	/* Width */
	skip = false;
	while( true ){
		char* sNewWidth = getTextInput( "Enter Width", &skip, &cancel, sbError );
		if( cancel ){
			deleteMedia( newMedia );
			return;
		}
		if( skip ){
			newMedia.width.length = 0;
			break;
		}
		if( sNewWidth == NULL ){
			sStatus = sbError.string;
			continue;
		}
		stringSetValue( newMedia.width, sNewWidth );
		break;
	}

	/* Thickness */
	skip = false;
	while( true ){
		char* sNewThickness = getTextInput( "Enter Thickness", &skip, &cancel, sbError );
		if( cancel ){
			deleteMedia( newMedia );
			return;
		}
		if( skip ){
			newMedia.thickness.length = 0;
			break;
		}
		if( sNewThickness == NULL ){
			sStatus = sbError.string;
			continue;
		}
		stringSetValue( newMedia.thickness, sNewThickness );
		break;
	}

	/* Description */
	skip = false;
	while( true ){
		char* sNewDescription = getTextInput( "Enter Description", &skip, &cancel, sbError );
		if( cancel ){
			deleteMedia( newMedia );
			return;
		}
		if( skip ){
			newMedia.description.length = 0;
			break;
		}
		if( sNewDescription == NULL ){
			sStatus = sbError.string;
			continue;
		}
		stringSetValue( newMedia.description, sNewDescription );
		break;
	}

	/* Notes */
	skip = false;
	while( true ){
		char* sNewNotes = getTextInput( "Enter Notes", &skip, &cancel, sbError );
		if( cancel ){
			deleteMedia( newMedia );
			return;
		}
		if( skip ){
			newMedia.notes.length = 0;
			break;
		}
		if( sNewNotes == NULL ){
			sStatus = sbError.string;
			continue;
		}
		stringSetValue( newMedia.notes, sNewNotes );
		break;
	}

	/* ExLibris */
	skip = false;
	while( true ){
		char* sNewExLibris = getTextInput( "Enter ExLibris", &skip, &cancel, sbError );
		if( cancel ){
			deleteMedia( newMedia );
			return;
		}
		if( skip ){
			newMedia.exlibris.length = 0;
			break;
		}
		if( sNewExLibris == NULL ){
			sStatus = sbError.string;
			continue;
		}
		stringSetValue( newMedia.exlibris, sNewExLibris );
		break;
	}
	
	if( ! recordNew( gMedia, newMedia, sbError ){
		stringInsert( "failed to create add new media record: ", sbError );
		return NULL;
	}
	giAccessionNumber++;
}

void interact_edit(){}

void interact_duplicate(){}

SVALUE* getTextInput( char* sPrompt, STRBUF* sbError ){
	static char buffer[ 256 ];
	unsigned int lenBuffer = 0;
	unsigned int posBuffer = 0;
	int xBuffer = 0;
	for( ; xBuffer < 256; xBuffer++ ) buffer[ xBuffer ] = 0; /* zero buffer */
	printf( "%s: ", sPrompt );
	while( true ){
		char cInput = getchar();
		if( cInput == 10 || cInput == 13 ) break;
		if( cInput == 8 ){  /* backspace */
			if( lenBuffer > 0 ){
				putchar( 8 );
				lenBuffer--;
				posBuffer--;
			}
			continue;
		}
		if( cInput == 27 ) return NULL; /* ESC == cancel */
		if( cInput < 32 ) break;
		if( lenBuffer >= 254 ){
			printf( "\ntoo many characters\n" );
			continue;
		}
		putchar( cInput );
		buffer[ posBuffer ] = cInput;
		lenBuffer++;
		posBuffer++;
	}
	if( lenBuffer == 0 ) return NULL;
	SVALUE* svNew = stringCreateValue( buffer );
	if( svNew == NULL ){
		stringAppend( sbError, "failed to allocate memory for new string value" );
		return NULL;
	}
	return svNew;
}

/* put file descriptor 0 into chr-by-chr mode and noecho mode */
set_terminal_raw(){
	struct	termios	ttystate;
	tcgetattr( 0, &ttystate);               /* read current setting */
	ttystate.c_lflag          &= ~ICANON;   /* no buffering		*/
	ttystate.c_lflag          &= ~ECHO;     /* no echo either	*/
	ttystate.c_cc[VMIN]        =  1;        /* get 1 char at a time	*/
	tcsetattr( 0 , TCSANOW, &ttystate);     /* install settings	*/
}

set_terminal_buffered(){
	struct	termios	ttystate;
	tcgetattr( 0, &ttystate);               /* read current setting */
	ttystate.c_lflag          &= ICANON;    /* buffer input, canonical line processing */
	ttystate.c_lflag          &= ECHO;      /* echo typed characters */
	tcsetattr( 0 , TCSANOW, &ttystate);     /* install settings	*/
}


void tty_save(){ tcgetattr( 0, &saved_tty_mode ); }
void tty_restore(){ tcsetattr( 0, TCSANOW, &saved_tty_mode ); }


