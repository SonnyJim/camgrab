#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>

#define LOGFILE "/mnt/data/Backup/Cameras/logs/camgrab.log"

#define DEFAULT_CFG_FILE "/etc/camgrab.conf"

//Set to 5Meg
int verbose;
int running;

// pidfile.c
int remove_pid (void);
int check_pid (void);

// rotate.c
void rotate (int cam_num);
long long get_dir_size (int cam_num);
void rotate_dir (int cam_num);

// cfg.c
int cfg_load (void);
int num_cams;

#define CFG_NUM_CAMS "num_cams="
#define CFG_CAM_DIR "_dir="
#define CFG_CAM_URL "_url="
#define CFG_CAM_MAXSIZE "_maxsize="
#define CFG_CAM_PASSWORD "_password="
#define CFG_CAM_INTERVAL "_interval="
#define CFG_CAM_ENABLED "_enabled="

#define MAX_CAMS 16

struct cam
{
    char *url;
    char *dir;
    char *password;
    long long maxsize;
    int interval;
    int enabled;
    long last_grabbed;
};

struct cam cams[MAX_CAMS];
