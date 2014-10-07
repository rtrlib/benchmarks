#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netdb.h>
#include "rtrlib/rtrlib.h"
#include "getusage.c"
#include <signal.h>
#include <sys/time.h>


unsigned int get_timediff(const struct timeval start,
        const struct timeval end) {
    return (((end.tv_sec * 1000000) + end.tv_usec) -
            ((start.tv_sec * 1000000) + start.tv_usec));
}

void sig_handler(){
    fprintf(stderr, "FLUSHING\n");
    fflush(NULL);

}

void read_prefixes(const char* filename, struct pfx_record** recs, unsigned int*
        recs_len){
    unsigned int records_added = 0;
    char buf[100];

    printf("\nLoading prefixes from %s into memory...\n", filename);
    FILE* f = fopen(filename, "r");
    if(f == NULL){
        perror("fopen error");
        exit(EXIT_FAILURE);
    }

    int fscanf_rtval = 1;
    do{
        if(*recs_len < records_added +1){
            void* tmp = realloc(*recs, (*recs_len +1) * sizeof(struct pfx_record));
            if(tmp == NULL){
                perror("realloc error");
                exit(EXIT_FAILURE);
                free(recs);
            }
            *recs = tmp;
        }

        printf("\rLoading prefix: %6u", records_added);
        fflush(stdout);

        fscanf_rtval = fscanf(f, "%s %hhu %hhu %u", buf,
                &((*recs)[records_added].min_len),
                &((*recs)[records_added].max_len),
                &((*recs)[records_added].asn));

        if(ip_str_to_addr(buf, &((*recs)[records_added].prefix)) == -1){
            fprintf(stderr, "rtr_str_to_ipaddr error: %s",buf);
            exit(EXIT_FAILURE);
        }
        if(fscanf_rtval != EOF){
            records_added++;
            (*recs_len)++;
        }
        else{
            void* tmp = realloc(*recs, (*recs_len) * sizeof(struct pfx_record)
                    );
            if(tmp == NULL){
                perror("realloc error");
                exit(EXIT_FAILURE);
                free(recs);
            }
            *recs = tmp;
        }


    } while(fscanf_rtval != EOF);
    printf("\rLoading prefix: %6u", records_added);

    fclose(f);
    printf("\n");

}

void fill_pfx_table(pfx_table* pfxt, const struct pfx_record* prefixes,
        const unsigned int prefix_len){

    for (unsigned int i = 0; i < prefix_len; i++) {
        const pfx_record* rec = prefixes + i;
        if(pfx_table_add(pfxt, rec) == PFX_ERROR){
            fprintf(stderr, "\nError adding record %u to pfx_table", i);
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char* argv[])
{
    char roa_file[200];
    if(argc != 1){
        printf("Usage:\n");
        printf("%s <roa-file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    strncpy(roa_file, argv[1], sizeof(roa_file));


    unsigned int prefix_len = 0;
    struct pfx_record* prefixes = NULL;
    read_prefixes(roa_file, &prefixes, &prefix_len);


    const pid_t pid = getpid();

    struct pstat start_cpu;
    struct pstat end_cpu;
    long unsigned int ucpu_usage;
    long unsigned int scpu_usage;
    pfx_table pfxt;
    pfx_table_init(&pfxt, NULL);

    printf("Adding records to pfx_table...\n");

    if(get_usage(pid, &start_cpu) == -1){
        fprintf(stderr, "GET USAGE ERROR\n");
        exit(EXIT_FAILURE);
    }
    struct timeval stime;
    gettimeofday(&stime, NULL);

    fill_pfx_table(&pfxt, prefixes, prefix_len);

    struct timeval etime;
    gettimeofday(&etime, NULL);
    if(get_usage(pid, &end_cpu) == -1){
        fprintf(stderr, "GET USAGE ERROR\n");
    }
    printf(" Done\n");

    calc_cpu_usage(&end_cpu, &start_cpu, &ucpu_usage, &scpu_usage);
    printf("cpu ticks: %lu\n", ucpu_usage + scpu_usage);
    printf("usecs: %u\n", get_timediff(stime, etime));
    pfx_table_free(&pfxt);


    return 0;
}
