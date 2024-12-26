#include "definitions.h"

unsigned int giAccessionNumber = 0;

RECORDS gMedia;
RECORDS gPlaces;
RECORDS gAuthors;
RECORDS gVendors;
RECORDS gPublishers;

bool recordNew( RECORDS recordset, void* pNewRecord, STRBUF* sbError );
bool recordDelete( RECORDS recordset, unsigned int xRecordToBeDeleted, STRBUF* sbError );
void freeMedia( MEDIA* pMedia );
void freePlace( PLACE* pPlace );
void freeAuthor( AUTHOR* pAuthor );
void freeVendor( VENDOR* pVendor );
RECORD* getAuthorByName( char* sBuffer, size_t lenBuffer, unsigned int* iOccurrence );
AUTHORSHIP* createAuthorship( AUTHOR* pAuthor, SVALUE* svCustomAuthor, SVALUE* svRole, STRBUF* sbError };
PUBLICATION* creatPublication( PUBLISHER* pPublisher, SVALUE* svCustomPublisher, SVALUE* svImprint, PLACE* place, STRBUF* sbError );

bool recordNew( RECORDS recordset, void* pNewRecord, STRBUF* sbError ){
	if( recordset.delete_count > 0 ){ /* fill the deleted entry with the new item */
		unsigned int xRecord = recordset->deletes[ --recordset.delete_count ];
		recordset->records[ xRecord ] = pNewRecord;
		recordset.record_count++;
	} else {
		if( gMedia.record_count == recordset.record_capacity ){ /* need to increase capacity */
			unsigned int iNewCapacity = recordset.record_capacity * 1.2;
			void** pExpandedRecordArray = realloc( recordset.records, iNewCapacity * sizeof( void* ) );
			if( pExpandedRecordArray == NULL ){
				stringInsert( "Failed to expand record array", sbError );
				return false;
			}
			recordset.record_capacity = iNewCapacity;
		}
		recordset->records[ recordset.record_count ] = pNewRecord;
		recordset.record_count++;
	}
	return true;
}

void freeMedia( MEDIA* pMedia ){}
void freePlace( PLACE* pPlace ){}
void freeAuthor( AUTHOR* pAuthor ){}
void freeVendor( VENDOR* pVendor ){}

bool recordDelete( RECORDS recordset, unsigned int xRecordToBeDeleted, STRBUF* sbError ){
	if( xRecordToBeDeleted > recordset.record_count ){
		string.append( "invalid record index", sbError );
		return false;
	}
	void* pRecordToBeDeleted = recordset->records[ xRecordToBeDeleted ];
	if( pRecordToBeDeleted == NULL ){
		string.append( "no such record exists", sbError );
		return false;
	}
	if( recordset.delete_count == recordset.delete_capacity ){ /* need to increase capacity */
		unsigned int iNewCapacity = recordset.delete_capacity * 1.2;
		void** pExpandedDeleteList = realloc( recordset.deletes, iNewCapacity * sizeof( void* ) );
		if( pExpandedDeleteList == NULL ){
			stringInsert( "Failed to expand delete list", sbError );
			return false;
		}
		recordset.delete_capacity = iNewCapacity;
	}
	if( recordset.records == gMedia.records ) freeMedia( pRecordToBeDeleted );
	else if( recordset.records == gPlaces.records ) freePlace( pRecordToBeDeleted );
	else if( recordset.records == gAuthors.records ) freeAuthor( pRecordToBeDeleted );
	else if( recordset.records == gVendors.records ) freeVendor( pRecordToBeDeleted );
	else {
		string.append( "unknown recordset", sbError );
		return false;
	}
	recordset->records[ xRecordToBeDeleted ] = NULL;
	recordset->deletes[ recordset.delete_count ] = xRecordToBeDeleted;
	recordset.delete_count++;
	return true;
}
	
RECORD* getAuthorByName( char* sBuffer, size_t lenBuffer, unsigned int* iOccurrence ){
	if( gAuthors == NULL ) return NULL;
	if( gAuthors->record_count == 0 ) return NULL;
	if( iOccurrence < 0 ) iStartingIndex == 0;
	if( iOccurrence >= gAuthors->record_count ) iStartingIndex == gAuthors->record_count % iOccurrence;
	unsigned int xAuthor = iStartingIndex;
	while( true ){
		AUTHOR* pAuthor = gAuthors->records[ xAuthor ];
		if( pAuthor != NULL ){
			if( stringMatchCharArray( pAuthor->surname, sBuffer, 0, lenBuffer ) ) return pAuthor;
		}
		xAuthor++;
		if( xAuthor >= gAuthors->length ) xAuthor = 0;
		if( xAuthor == iStartingIndex ) return NULL;
	}
}

AUTHORSHIP* createAuthorship( AUTHOR* pAuthor, SVALUE* svCustomAuthor, SVALUE* svRole, STRBUF* sbError }{
	AUTHORSHIP* pAuthorship = malloc( sizeof( AUTHORSHIP ) );
	if( pAuthorship == NULL ){
		string.append( "out of memory", sbError );
		return NULL;
	}
	if( pAuthor == NULL && svCustomAuthor == NULL ){
		string.append( "no author value supplied", sbError );
		return NULL;
	}
	pAuthorship->author = pAuthor;
	pAuthorship->name = svCustomAuthor;
	svRole = svRole;
	return pAuthorship;
}

PUBLICATION* creatPublication( PUBLISHER* pPublisher, SVALUE* svCustomPublisher, SVALUE* svImprint, PLACE* place, STRBUF* sbError );
	PUBLICATION* pPublication = malloc( sizeof( PUBLICATION* ) );
	if( pPublication == NULL ){
		string.append( "out of memory", sbError );
		return NULL;
	}
	if( pPublisher == NULL && svCustomPublisher == NULL ){
		string.append( "no publisher value supplied", sbError );
		return NULL;
	}
	pPublication->publisher = pPublisher;
	pPublication->name = svCustomPublisher;
	pPublication->imprint = svImprint;
	pPublication->place = place;
	return pPublication;
}

