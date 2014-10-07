//Last modified: 28/10/12 18:37:36(CET) by Fabian Holler
#include <stdlib.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <strings.h>
#include <unistd.h>

struct pstat{
    long unsigned int utime_ticks;
    long int cutime_ticks;
    long unsigned int stime_ticks;
    long int cstime_ticks;
    long unsigned int vsize; // virtual memory size in bytes
    long unsigned int rss; //Resident  Set  Size in bytes

    long unsigned int cpu_total_time;
};

/*
 * read /proc data into the passed struct pstat
 * returns 0 on success, -1 on error
*/
int get_usage(const pid_t pid, struct pstat* result){
    //convert  pid to string
    char pid_s[20];
    snprintf(pid_s, sizeof(pid_s), "%d", pid);
    char stat_filepath[30] = "/proc/"; strncat(stat_filepath, pid_s,
            sizeof(stat_filepath) - strlen(stat_filepath) -1);
    strncat(stat_filepath, "/stat", sizeof(stat_filepath) -
            strlen(stat_filepath) -1);

    //Open /proc/stat and /proc/$pid/stat fds successive(dont want that cpu
    //ticks increases too much during measurements)
    int fd_stat = open(stat_filepath, O_RDONLY, O_DIRECT | O_CLOEXEC);
    FILE* fpstat = fdopen(fd_stat, "r");
    if(fpstat == NULL){
        perror("FOPEN ERROR ");
        return -1;
    }

    int fd_fstat = open("/proc/stat", O_RDONLY, O_DIRECT | O_CLOEXEC);
    FILE* fstat = fdopen(fd_fstat, "r");
    if(fstat == NULL){
        perror("FOPEN ERROR ");
        fclose(fstat);
        close(fd_fstat);
        return -1;
    }

    //read values from /proc/pid/stat
    bzero(result, sizeof(struct pstat));
    long int rss;
    if(fscanf(fpstat, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu"
                "%lu %ld %ld %*d %*d %*d %*d %*u %lu %ld",
                &result->utime_ticks, &result->stime_ticks,
                &result->cutime_ticks, &result->cstime_ticks, &result->vsize,
                &rss) == EOF) {
        fclose(fpstat);
        close(fd_stat);
        return -1;
    }
    fclose(fpstat);
    close(fd_stat);
    result->rss = rss * getpagesize();

    //read+calc cpu total time from /proc/stat
    long unsigned int cpu_time[10];
    bzero(cpu_time, sizeof(cpu_time));
    if(fscanf(fstat, "%*s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
                &cpu_time[0], &cpu_time[1], &cpu_time[2], &cpu_time[3],
                &cpu_time[4], &cpu_time[5], &cpu_time[6], &cpu_time[7],
                &cpu_time[8], &cpu_time[9]) == EOF) {
        fclose(fstat);
        close(fd_stat);
        return -1;
    }

    fclose(fstat);
    close(fd_fstat);

    for(int i=0; i < 10;i++){
        result->cpu_total_time += cpu_time[i];
    }

    return 0;
}

/*
* calculates the actual CPU usage(cur_usage - last_usage) in percent
* cur_usage, last_usage: both last measured get_usage() results
* ucpu_usage, scpu_usage: result parameters: user and sys cpu usage in %
*/
void calc_cpu_usage_pct(const struct pstat* cur_usage, const struct pstat*
        last_usage, double* ucpu_usage, double* scpu_usage){
    const long unsigned int total_time_diff = cur_usage->cpu_total_time -
                                              last_usage->cpu_total_time;

    *ucpu_usage = 100 * (((cur_usage->utime_ticks + cur_usage->cutime_ticks)
                    - (last_usage->utime_ticks + last_usage->cutime_ticks))
                    / (double) total_time_diff);

    *scpu_usage = 100 * ((((cur_usage->stime_ticks + cur_usage->cstime_ticks)
                    - (last_usage->stime_ticks + last_usage->cstime_ticks))) /
                    (double) total_time_diff);
}

/*
* calculates CPU usage(cur_usage - last_usage) difference
* cur_usage, last_usage: both last measured get_usage() results
* ucpu_usage, scpu_usage: result parameters: user and sys cpu usage in %
*/
void calc_cpu_usage(const struct pstat* cur_usage, const struct pstat*
        last_usage, long unsigned int* ucpu_usage, long unsigned int*
        scpu_usage){

    *ucpu_usage = (cur_usage->utime_ticks + cur_usage->cutime_ticks)
                    - (last_usage->utime_ticks + last_usage->cutime_ticks);

    *scpu_usage = (cur_usage->stime_ticks + cur_usage->cstime_ticks)
                    - (last_usage->stime_ticks + last_usage->cstime_ticks);
}

