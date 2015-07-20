#include "camgrab.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

static long long total = 0;
static unsigned int oldest_time;
char oldest_file[1024];
int file_count = 0;

int get_date(const char *fpath, const struct stat *sb, int typeflag) 
{
    if (S_ISDIR(sb->st_mode))
    {
        //Ignore, it's a directory
        return 0;
    }
    
    if (sb->st_mtime < oldest_time)
    {
        oldest_time = sb->st_mtime;
        strcpy (oldest_file, fpath);
        if (verbose)
            fprintf (stdout, "tagging %s as oldest file\n", oldest_file);
    }

    return 0;
}

int sum(const char *fpath, const struct stat *sb, int typeflag) 
{
        total += sb->st_size;
            return 0;
}

void rotate (int cam_num)
{
    long long dir_size;

    dir_size = get_dir_size (cam_num);
    if (dir_size == -1)
    {
        fprintf (stdout, "Error getting dir_size\n");
        return;
    }
    else if (dir_size > cams[cam_num].maxsize)
        rotate_dir (cam_num);
}

long long get_dir_size (int cam_num) 
{
    total = 0;
    if (ftw(cams[cam_num].dir, &sum, 1)) 
    {
        perror("ftw");
        return -1;
    }
    if (verbose)
        fprintf (stdout, "get_dir_size: %s: %lu\n", cams[cam_num].dir, total);
    return total;
}

static void remove_empty_dirs (char* dir)
{
    char cmd[1024];
    strcpy (cmd, "");
    
    if (verbose)
    {
        fprintf (stdout, "Checking for empty dirs from %s\n", dir);
        sprintf (cmd, "find %s -type d -empty -exec rmdir {} \\;", dir);
        fprintf (stdout, cmd);
        fprintf (stdout, "\n");
    }
    system (cmd);
}

void rotate_dir (int cam_num)
{
    char buffer[256];
    long long dir_size; 
    oldest_time = time(0);
    
    if (verbose)
    {
        sprintf (buffer, "Rotating directory: %s because size %i is bigger than %i\n", cams[cam_num].dir, get_dir_size (cam_num), cams[cam_num].maxsize);
        fprintf (stdout, buffer);
    }

    log_text (buffer);
    dir_size = get_dir_size (cam_num);
    if (verbose)
        fprintf (stdout, "Pre-rotate %s size: %i\n", cams[cam_num].dir, dir_size);
     
    //Find the oldest entry
    while (dir_size > cams[cam_num].maxsize)
    {

        if (ftw (cams[cam_num].dir, &get_date, 1))
        {
            perror ("ftw");
            return;
        }
        oldest_time = time(0);
        if (strlen (oldest_file) > 1)
        {
            if (verbose)
                fprintf (stdout, "Removing file %s\n", oldest_file);
            if (remove (oldest_file) == -1)
            {
                fprintf (stderr, "Error removing file: %s\n", oldest_file);
                perror ("remove");
                return;
            }
        }
        dir_size = get_dir_size (cam_num);
        if (verbose)
            fprintf (stdout, "%s size: %i, maxsize %i\n", cams[cam_num].dir, dir_size, cams[cam_num].maxsize);
    }
    remove_empty_dirs (cams[cam_num].dir);
}
