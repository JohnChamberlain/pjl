#include <stdlib.h> /* realloc */
#include "jcshared.h"

unsigned int giAccessionNumber = 0;

RECORDS gMedia;
RECORDS gPlaces;
RECORDS gAuthors;
RECORDS gVendors;
RECORDS gPublishers;

bool recordNew( RECORDS* recordset, void* pNewRecord, STRBUF* sbError ){
	if( recordset->delete_count > 0 ){ /* fill the deleted entry with the new item */
		unsigned int xRecord = *recordset->deletes[ --recordset->delete_count ];
		recordset->records[ xRecord ] = pNewRecord;
		recordset->record_count++;
	} else {
		if( gMedia.record_count == recordset->record_capacity ){ /* need to increase capacity */
			unsigned int iNewCapacity = recordset->record_capacity * 1.2;
			void** pExpandedRecordArray = realloc( recordset->records, iNewCapacity * sizeof( void* ) );
			if( pExpandedRecordArray == NULL ){
				stringInsert( sbError, "Failed to expand record array" );
				return false;
			}
			recordset->record_capacity = iNewCapacity;
		}
		recordset->records[ recordset->record_count ] = pNewRecord;
		recordset->record_count++;
	}
	return true;
}

void freeMedia( MEDIA* pMedia ){}
void freePlace( PLACE* pPlace ){}
void freeAuthor( AUTHOR* pAuthor ){}
void freeVendor( VENDOR* pVendor ){}

bool recordDelete( RECORDS* recordset, unsigned int xRecordToBeDeleted, STRBUF* sbError ){
	if( xRecordToBeDeleted > recordset->record_count ){
		stringAppend( sbError, "invalid record index" );
		return false;
	}
	void* pRecordToBeDeleted = recordset->records[ xRecordToBeDeleted ];
	if( pRecordToBeDeleted == NULL ){
		stringAppend( sbError, "no such record exists" );
		return false;
	}
	if( recordset->delete_count == recordset->delete_capacity ){ /* need to increase capacity */
		unsigned int iNewCapacity = recordset->delete_capacity * 1.2;
		void** pExpandedDeleteList = realloc( recordset->deletes, iNewCapacity * sizeof( void* ) );
		if( pExpandedDeleteList == NULL ){
			stringInsert( sbError, "Failed to expand delete list" );
			return false;
		}
		recordset->delete_capacity = iNewCapacity;
	}
	if( recordset->records == gMedia.records ) freeMedia( pRecordToBeDeleted );
	else if( recordset->records == gPlaces.records ) freePlace( pRecordToBeDeleted );
	else if( recordset->records == gAuthors.records ) freeAuthor( pRecordToBeDeleted );
	else if( recordset->records == gVendors.records ) freeVendor( pRecordToBeDeleted );
	else {
		stringAppend( sbError, "unknown recordset" );
		return false;
	}
	recordset->records[ xRecordToBeDeleted ] = NULL;
	*recordset->deletes[ recordset->delete_count ] = xRecordToBeDeleted;
	recordset->delete_count++;
	return true;
}
	
AUTHOR* getAuthorByName( char* sBuffer, unsigned int lenBuffer, unsigned int* iOccurrence ){
	if( gAuthors.record_count == 0 ) return NULL;
	int iStartingIndex = 0;
	if( *iOccurrence >= gAuthors.record_count ) iStartingIndex == gAuthors.record_count % *iOccurrence;
	unsigned int xAuthor = iStartingIndex;
	while( true ){
		AUTHOR* pAuthor = gAuthors.records[ xAuthor ];
		if( pAuthor != NULL ){
			if( stringMatchCharArray( pAuthor->surname, sBuffer, 0, lenBuffer ) ) return pAuthor;
		}
		xAuthor++;
		if( xAuthor >= gAuthors.record_count ) xAuthor = 0;
		if( xAuthor == iStartingIndex ) return NULL;
	}
}

PUBLISHER* getPublisherByName( char* buffer, unsigned int lenBuffer, unsigned int* iOccurrence ){
	if( gPublishers.record_count == 0 ) return NULL;
	int iStartingIndex = 0;
	if( *iOccurrence >= gPublishers.record_count ) iStartingIndex == gPublishers.record_count % *iOccurrence;
	unsigned int xPublisher = iStartingIndex;
	while( true ){
		PUBLISHER* gPublisher = gPublishers.records[ xPublisher ];
		if( gPublisher != NULL ){
			if( stringMatchCharArray( gPublisher->name, buffer, 0, lenBuffer ) ) return gPublisher;
		}
		xPublisher++;
		if( xPublisher >= gPublisher->name->length ) xPublisher = 0;
		if( xPublisher == iStartingIndex ) return NULL;
	}
}

AUTHORSHIP* createAuthorship( AUTHOR* pAuthor, SVALUE* svCustomAuthor, SVALUE* svRole, STRBUF* sbError ){
	AUTHORSHIP* pAuthorship = malloc( sizeof( AUTHORSHIP ) );
	if( pAuthorship == NULL ){
		stringAppend( sbError, "out of memory" );
		return NULL;
	}
	if( pAuthor == NULL && svCustomAuthor == NULL ){
		stringAppend( sbError, "no author value supplied" );
		return NULL;
	}
	pAuthorship->author = pAuthor;
	pAuthorship->name = svCustomAuthor;
	svRole = svRole;
	return pAuthorship;
}

PUBLICATION* createPublication( PUBLISHER* pPublisher, SVALUE* svCustomPublisher, SVALUE* svImprint, PLACE* place, STRBUF* sbError ){
	PUBLICATION* pPublication = malloc( sizeof( PUBLICATION ) );
	if( pPublication == NULL ){
		stringAppend( sbError, "out of memory" );
		return NULL;
	}
	if( pPublisher == NULL && svCustomPublisher == NULL ){
		stringAppend( sbError, "no publisher value supplied" );
		return NULL;
	}
	pPublication->publisher = pPublisher;
	pPublication->name = svCustomPublisher;
	pPublication->imprint = svImprint;
	pPublication->place = place->city;
	return pPublication;
}

PLACE* getPlace( SVALUE* s, STRBUF* sbError ){
	PLACE* place = matchPlace( s )[0];
	if( place != NULL ) return place;
	return createPlace( s, NULL, NULL, NULL, sbError ); /* TODO */
}
	
PLACE* createPlace( SVALUE* city, SVALUE* state, SVALUE* country, SVALUE* code, STRBUF* sbError ){
	PLACE* pPlace = malloc( sizeof( PLACE ) );
	if( pPlace == NULL ){
		stringAppend( sbError, "out of memory" );
		return NULL;
	}
	pPlace->city = city;
	pPlace->state = state;
	pPlace->country = country;
	pPlace->code = code;
}

PLACE** matchPlace( SVALUE* pattern ){
	unsigned int xPlace = 0;
	while( true ){
		if( gPlaces.records[ xPlace ] == NULL ) return NULL; /* place not found */
		PLACE* place = gPlaces.records[ xPlace ];
		/* TODO */
		break;
	}
	return NULL;
}	

