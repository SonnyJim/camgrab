#include "camgrab.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

static unsigned int total = 0;
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
        fprintf (stdout, "tagging %s as oldest file\n", oldest_file);
    }

    return 0;
}

int sum(const char *fpath, const struct stat *sb, int typeflag) 
{
        total += sb->st_size;
            return 0;
}

int get_dir_size (char* dir) 
{
    total = 0;
    if (ftw(dir, &sum, 1)) 
    {
        perror("ftw");
        return -1;
    }
    return total;
}

static void remove_empty_dirs (char* dir)
{
    char cmd[1024];
    fprintf (stdout, "Checking for empty dirs\n");
    
    sprintf (cmd, "find %s -type d -empty -exec rmdir {} \\;", dir);
    fprintf (stdout, cmd);
    system (cmd);
}

void rotate_dir (char* dir, int maxsize)
{
    char buffer[256];
    long dir_size; 
    oldest_time = time(0);
    
    if (verbose)
    {
        sprintf (buffer, "Rotating directory: %s because size %i is bigger than %i\n", dir, get_dir_size (dir), maxsize);
        fprintf (stdout, buffer);
    }

    log_text (buffer);
    dir_size = get_dir_size (dir);
    
    //Find the oldest entry
    while (dir_size > maxsize)
    {
        if (ftw (dir, &get_date, 1))
        {
            perror ("ftw");
            return;
        }
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
        dir_size = get_dir_size (dir);
        fprintf (stdout, "New dir size: %i, maxsize %i\n", dir_size, maxsize);
    }
    remove_empty_dirs (dir);
}
