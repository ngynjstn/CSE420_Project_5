#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
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
};

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
	if (request == NULL) {
		fprintf(stderr, "WHOOPS: Couldn't allocate memory for node; %s\n", strerror(errno));
		exit(-1);
	}
	request->arrival_time = arrival_time;
	request->lbn = lbn;
	request->request_size = request_size;
	request->next = NULL;
	return request;
}


struct List *create_list() {
	struct List *list = malloc(sizeof(struct List));
	if (list == NULL) {
		fprintf(stderr, "NOOOO: Couldn't create memory for the list; %s\n", strerror(errno));
		exit(-1);
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
};

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
double calc_seektime(int cylinder_start, int end_cylinder) {
	int seek_distance = abs(end_cylinder - cylinder_start);
	if (seek_distance == 0) return 0; 
	return (((FULLSEEK / 1000.0) - (TRACKTOTRACKSEEK / 1000.0)) / CYLINDERS) * seek_distance + (TRACKTOTRACKSEEK / 1000.0); // convet millsecons to seconds
}

double calc_rotational_latency(double current_sector_offset, double target_sector_offset, double seek_time) {
    //time it takes to travel one sector
    double time_per_sector = 60.0 / (RPM * SECTORSPERTRACK);

    //uodpdate the current sector position after the seek (i.e., move the head and update the current position)
    double current_position_after_seek = fmod(current_sector_offset + seek_time / time_per_sector, SECTORSPERTRACK);
 
    //make sure target sector offset is within the correct range
    target_sector_offset = fmod(target_sector_offset, SECTORSPERTRACK);

    //calculate the number of sectors we need to travel to get to the target offset
    double sectors_to_travel = fmod(target_sector_offset - current_position_after_seek + SECTORSPERTRACK, SECTORSPERTRACK);

    //ceturn the rotational latency (time to travel the required number of sectors)
    return sectors_to_travel * time_per_sector;
}

double calc_transfertime(int request_size) {
	return ((double)request_size * 8 * PHYSICALSECTORSIZE) / (TRANSFERRATE * 1e9); //thsi shows the exponent portion
}

void calc_disk_param(int lbn, int request_size, int *psn, int *psnfinal, int *cylinder, int *surface, double *sector_offset) {
	*psn = lbn * (LOGICALBLOCKSIZE / PHYSICALSECTORSIZE); //this one will be used to calculate the other becuase its the starting point
	*psnfinal = *psn + request_size; // put thsi portion to get psn final 
	*cylinder = *psn / (SECTORSPERTRACK * TRACKSPERCYLINDER);
    *surface = (*psn / SECTORSPERTRACK) % TRACKSPERCYLINDER;
    *sector_offset = *psnfinal % SECTORSPERTRACK;
}

//func to sim the FCFC alogrithms
void fcfs_algorithm(struct List *requests, const char *outputfile, int limit) {
	FILE *file = fopen(outputfile, "w");

	struct Request *curr = requests->head;
	double curr_time = 0;
	int curr_cylinder = 0;
	double curr_sector_offset = 0;
	int count = 0;

	while (curr != NULL && count < limit) { //counts the limit so it is able to read the input 
		int psnfinal, psn, surface, cylinder;
		double sector_offset;
		calc_disk_param(curr->lbn, curr->request_size, &psn, &psnfinal, &cylinder, &surface, &sector_offset);

		double seek_time = calc_seektime(curr_cylinder, cylinder);
		double rotational_latency = calc_rotational_latency(curr_sector_offset, sector_offset, seek_time);
		double transfertime = calc_transfertime(curr->request_size);
	
		double waiting_time = fmax(0, curr_time - curr->arrival_time);
		double finish_time = curr->arrival_time + waiting_time + seek_time + rotational_latency + transfertime;

		int seek_distance = abs(curr_cylinder - cylinder);

		fprintf(file, "%.6lf %.6lf %.6lf %d %d %d %.6lf %d\n",
                	curr->arrival_time, finish_time, waiting_time, psnfinal, cylinder, surface, sector_offset, seek_distance);
		
		curr_time = finish_time;
		curr_cylinder = cylinder;
		
		// **Update current_sector_offset after seeking**
        double time_per_sector = 60.0 / (RPM * SECTORSPERTRACK);
        int sectors_travelled_during_seek = seek_time / time_per_sector;  // Calculate how many sectors the head moved
        curr_sector_offset = fmod(curr_sector_offset + sectors_travelled_during_seek, SECTORSPERTRACK); // Update the offset
		
		curr = curr->next;
		count++;
	}
	
	fclose(file);
}

//implementation of shorets seek time first
void sstf_algorithm(struct List *request, const char *outputfile, int limit) {
	FILE *file = fopen(outputfile, "w");

	double curr_time = 0;
	int curr_cylinder = 0;
	double curr_sector_offset;
	int process_count = 0; //hold the process so it can continue

	struct Request *pendingreq[request->size];
	int pending_count = 0;

	//this populate the pending results array
	struct Request *curr = request->head;
	while (curr != NULL) {
		pendingreq[pending_count++] = curr;
		curr = curr->next;
	}

	while (pending_count > 0 && process_count < limit) {
		int closest_ind = -1; //negtaive to find the closest index
		int shortest_sd = INT_MAX; //shoretst seek distance max

		//find the closets request
		for (int i = 0; i < pending_count; i++) {
			int psnfinal, psn, cylinder, surface;
			double sector_offset;
			calc_disk_param(pendingreq[i]->lbn, pendingreq[i]->request_size,
			&psnfinal, &psn, &cylinder, &surface, &sector_offset);
			
			if (pendingreq[i]->arrival_time <= curr_time || closest_ind == -1) {
				int seek_distance = calc_seektime(curr_cylinder, cylinder);
				if (seek_distance < shortest_sd) {
					shortest_sd = seek_distance;
					closest_ind = i;
				}
			}
		}

		//if no request,  fast forward to the next reuqets arrival 
		if (closest_ind == -1) {
			curr_time = pendingreq[0]->arrival_time;
			continue;
		}

		//process the closest requests
		struct Request *select = pendingreq[closest_ind];
		int psnfinal, psn, cylinder, surface;
		double sector_offset;
		calc_disk_param(select->lbn, select->request_size,
				&psnfinal, &psn, &cylinder, &surface, &sector_offset);
		
		double seek_time = calc_seektime(curr_cylinder, cylinder);
        double rotational_latency = calc_rotational_latency(curr_sector_offset, sector_offset, seek_time);
        double transfertime = calc_transfertime(select->request_size);

		double waiting_time = fmax(0, curr_time - select->arrival_time);
        double finish_time = fmax(curr_time, select->arrival_time) + seek_time + rotational_latency + transfertime;

		//output it 
		fprintf(file, "%.6lf %.6lf %.6lf %d %d %d %.6lf %d\n",
                	select->arrival_time, finish_time, waiting_time,
                	psnfinal, cylinder, surface, sector_offset, abs(curr_cylinder - cylinder));

		//update the curr state
		curr_time = finish_time;
		curr_cylinder = cylinder;
		
		// **Update current_sector_offset after seeking**
        double time_per_sector = 60.0 / (RPM * SECTORSPERTRACK);
        int sectors_travelled_during_seek = seek_time / time_per_sector;  // Calculate how many sectors the head moved
        curr_sector_offset = fmod(curr_sector_offset + sectors_travelled_during_seek, SECTORSPERTRACK); // Update the offset
		

		//remove processed req
		for (int i = closest_ind; i < pending_count - 1; i++) {
			pendingreq[i] = pendingreq[i + 1];
		}
		pending_count--;
		process_count++;
	}
	fclose(file);
}

int main(int argc, char *argv[]) {
	const char *inputfile = argv[1];
    	const char *outputfile = argv[2];
    	const char *algorithm = argv[3];
    	int limit = (argc == 5) ? atoi(argv[4]) : INT_MAX;

	struct List *requests = readinput(inputfile);
	
	if (strcmp(algorithm, "fcfs") == 0) {
        	fcfs_algorithm(requests, outputfile, limit);
    	} else if (strcmp(algorithm, "sstf") == 0) {
		sstf_algorithm(requests, outputfile, limit);
	} else {
        fprintf(stderr, "Error: Unknown algorithm %s\n", algorithm);
        return -1;
    	}	

    	struct Request *current = requests->head;
    	while (current != NULL) {
        	struct Request *next = current->next;
        	free(current);
        	current = next;
    	}
    	free(requests);

    	return 0;
}