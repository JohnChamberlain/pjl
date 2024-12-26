#include "stddef.h"        /* size_t */
#include <jcstring.h>

typedef enum { false = 0, true = !false } bool;

typedef struct {
	size_t size;
	(const char)* strings[];
} STRLIST;

static STRLIST listMediaType = {
	7,
	{
		"Book",
		"Map",
		"Booklet",
		"Pamphlet",
		"Magazine",
		"Report",
		"Journal"
	}
};

static STRLIST listBinding = {
	13,
	{
		"Buckram",
		"Perfect",
		"PerfectSewn",
		"LaminatedHardcover",
		"LaminatedSoftcover",
		"Stapled",
		"SaddleStapled",
		"Leather",
		"LeatherQuarterbound",
		"Softcover",
		"Paperboard",
		"LinenHardcover",
		"Paperback"
	}
};

static STRLIST listPaper = {
	5,
	{
		"Clay",
		"Laid",
		"Rag",
		"Pulp",
		"Lightweight"
	}
};

static STRLIST listRole = {
	3,
	{
		"Editor",
		"Illustrator",
		"Photographer"
	}
};

typedef struct {
	SVALUE* city,
	SVALUE* state,
	SVALUE* country,
	SVALUE* code
} PLACE;

typedef struct {
	SVALUE* name;
	SVALUE* surname;
	SVALUE* date_of_birth;
	SVALUE* date_of_death;
	SVALUE* place_of_birth;
	SVALUE* place_of_death;
	SVALUE* floruit;
} AUTHOR;

typedef struct {
	AUTHOR* author;  /* known author */
	SVALUE* name;
	SVALUE* role;     /* see role list */
} AUTHORSHIP;

typedef struct {
	SVALUE* name;
	SVALUE* imprint;
	PLACE** places;
} PUBLISHER;

typedef struct {
	SVALUE* name;
	SVALUE* address;
	PLACE* place;
	SVALUE* telecom;
} VENDOR;

typedef struct {
	SVALUE* media_type;
	SVALUE* title;
	unsigned int author_count;
	AUTHORSHIP** authors;
	PUBLICATION* publication;
	SVALUE* edition;
	SVALUE* printing;
	SVALUE* loc;
	SVALUE* isbn;
	SVALUE* subject;
	SVALUE* illustrations;
	unsigned int pages;
	bool index;
	unsigned int accession_no;
	SVALUE* accession_date;
	VENDOR* vendor;
	SVALUE* price;
	SVALUE* purchase_date;
	SVALUE* binding;
	SVALUE* paper;
	SVALUE* height;
	SVALUE* width;
	SVALUE* thickness;
	SVALUE* description;
	SVALUE* notes;
	SVALUE* exlibris;
} MEDIA;
	
typedef struct {
	void** records;
	unsigned int record_count;
	unsigned int record_length;
	unsigned int record_capacity;
	unsigned int** deletes;
	unsigned int delete_count;
	unsigned int delete_length;
	unsigned int delete_capacity;
} RECORDS;


