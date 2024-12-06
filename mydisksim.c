#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>

//constants for disk layouts
#define SECTORSPERTRACK 200
#define TRACKSPERCYLINDER 8
#define CYLINDERS 500000
#define RPM 10000
#define PHYSICALSECTORSIZE 512
#define LOGICALBLOCKSIZE 4096
#define TRACKTOTRACKSEEK 2 //used to track time in milliseconds
#define FULLSEEK 16 //in milliseonfs
#define TRANSFERRATE 1 //in Gb/s

//struct for the result of the disk req sim
struct Result {
	double arrival_time;
	double finish_time;
	double waiting_time;
	int psn;
	int cylinder;
	int surface;
	double sector_offset;
	int seek_distance;

//struct for disk requst
struct Request {
	double arrival_time; 
	int lbn; //logical block num
	int request_size; 
	struct Request *next;
	//struct Request *prev;
};

//start of linked lsit implementatino 
struct List {
	struct Request *head;
	int size; //curr size of lsit
};

struct Request *create_request(double arrival_time, int lbn, int request_size) {
	struct Request *request = malloc(sizeof(struct Request));
	if (node == NULL) {
		fprintf(stderr, "WHOOPS: Couldn't allocate memory for node; %s\n", strerror(errno));
		exit(-1);
	}
	request->arrival_time = arrival_time;
	request->lbn = lbn;
	request->request_size = request_size;
	request->next = NULL:
	return request;
}


struct List *create_list() {
	struct List *list = malloc(sizeof(struct List));
	if (list == NULL) {
		fprintf(stderr, "NOOOO: Couldn't create memory for the list; %s\n", strerror(errno);
		exit(-1)
	}
	list->head = NULL;
	list->size = 0;
	return list;
}

//func to add request to list
void request_add(struct List *list, struct Request *request) {
	if (list->head == NULL) {
		list->head = request;
	} else {
		struct Request *curr = list->head;
		while (curr->next != NULL) {
			curr = curr->next;
		}
		curr->next = request;
	}
	list->size++;
}

//fucn to read the input file given
struct List *readinput(const char *inputfile) {
	FILE *file = fopen(inputfile, "r");

	struct List *list = create_list();
	double arrival_time;
	int lbn, request_size;

	while (fscanf(file, "%lf %d %d", &arrival_time, &lbn, &request_size) == 3) {
		struct Request *request = create_request(arrival_time, lbn, request_size);
		request_add(list, request);
	}

	fclose(file);
	return list;
}

//func to calc seek time
double calc_seektime(int cylinder_start int end_cylinder) {
	int seek_distance = abs(end_cylinder - cylinder_start);
	if (seek_distance == 0) {
		return 0;
	} else {
		return ((FULLSEEK - TRACKTOTRACKSEEK) / (double)CYLINDERS) * seek_distance + TRACKTOTRACKSEEK;
	}
}
