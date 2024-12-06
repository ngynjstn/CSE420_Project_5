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

struct Request *create_request(struct Request *request) {
	struct Request *request = malloc(sizeof(struct Node));
	if (node == NULL) {
		fprintf(stderr, "WHOOPS: Couldn't allocate memory for node; %s\n", strerror(errno));
		exit(-1);
	}
	node->request = request;
	node->next = NULL;
	return node;
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


