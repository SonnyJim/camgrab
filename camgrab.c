#include "camgrab.h"

//char time_str[24];

pthread_t cam_threads[MAX_CAMS];
pthread_t rotate_thread;

char* get_time (void)
{
    time_t current_time;
    struct tm * timeinfo;
    
    char *time_str = malloc(24);
    if (time_str == NULL)
    {
        fprintf (stderr, "Error malloc of time_str\n");
        running = 0;
    }
    
    time ( &current_time );
    timeinfo = localtime ( &current_time );
    strcpy (time_str, asctime (timeinfo));
    return time_str;
}

char* get_year (void)
{
    time_t current_time;
    struct tm * time_info;
 
    char *time_str = malloc(24);
    if (time_str == NULL)
    {
        fprintf (stderr, "Error malloc of time_str\n");
        running = 0;
    }
    
    time (&current_time);
    time_info = localtime(&current_time);

    strftime(time_str, sizeof(time_str), "%Y", time_info);
    return time_str;
}

char* get_month (void)
{
    time_t current_time;
    struct tm * time_info;
 
    char *time_str = malloc(24);
    if (time_str == NULL)
    {
        fprintf (stderr, "Error malloc of time_str\n");
        running = 0;
    }
    
    time (&current_time);
    time_info = localtime(&current_time);

    strftime(time_str, sizeof(time_str), "%m", time_info);
    return time_str;
}

char* get_day (void)
{
    time_t current_time;
    struct tm * time_info;
 
    char *time_str = malloc(24);
    if (time_str == NULL)
    {
        fprintf (stderr, "Error malloc of time_str\n");
        running = 0;
    }
    
    time (&current_time);
    time_info = localtime(&current_time);

    strftime(time_str, sizeof(time_str), "%d", time_info);
    return time_str;
}

char* get_hour (void)
{
    time_t current_time;
    struct tm * time_info;
 
    char *time_str = malloc(24);
    if (time_str == NULL)
    {
        fprintf (stderr, "Error malloc of time_str\n");
        running = 0;
    }
    
    time (&current_time);
    time_info = localtime(&current_time);

    strftime(time_str, sizeof(time_str), "%H", time_info);
    return time_str;
}

char* get_filename_time (char* dir, char* camera_name)
{
    time_t current_time;
    struct tm * time_info;
    char buffer[1024];
 
    char *time_str = malloc(24);
    if (time_str == NULL)
    {
        fprintf (stderr, "Error malloc of time_str\n");
        running = 0;
    }
    
    time (&current_time);
    time_info = localtime(&current_time);

    strcpy (buffer, camera_name);
    strcat (buffer, "-");
    strftime(time_str, sizeof(time_str), "%y%m%d_%H%M%S", time_info);
    strcat (buffer, time_str);
    strcat (buffer, ".jpg");
    strcpy (time_str, buffer);

    return time_str;
}

int log_text (char* text)
{
    FILE *fp;
    char logline[1024];

    fp = fopen (LOGFILE, "a");
    if (fp == NULL)
    {
        fprintf (stderr, "Error opening %s for writing\n", LOGFILE);
        return 1;
    }
    
    sprintf (logline, "%s: %s\n", get_time(), text);
    fputs (logline, fp);
    fclose (fp);
}

//See if we have a folder for todays date
void build_filename (int cam_num)
{
    char dir_name[1024];
    char filename[1024];
    int ret;

    //Build the directory structure and create them if needed
    strcpy (dir_name, cams[cam_num].dir);
    strcat (dir_name, get_year());
    strcat (dir_name, "/");
    ret = mkdir (dir_name, S_IRWXU);
    strcat (dir_name, get_month());
    strcat (dir_name, "/");
    ret = mkdir (dir_name, S_IRWXU);
    strcat (dir_name, get_day());
    strcat (dir_name, "/");
    ret = mkdir (dir_name, S_IRWXU);
    strcat (dir_name, get_hour());
    strcat (dir_name, "/");
    ret = mkdir (dir_name, S_IRWXU);
   
    //Append the filename
    strcpy (cams[cam_num].filename, "");
    strcpy (cams[cam_num].filename, dir_name);
    sprintf (dir_name, "CAM%i", cam_num + 1);
    strcat (cams[cam_num].filename, get_filename_time (cams[cam_num].dir, dir_name));
    
    return;

}

//Copy the latest image to /var/ww/html/cameras
static int build_html (int cam_num, char* filename)
{
    char name[1024];
    char cmd[1024];
    sprintf (name, "CAM%i.jpg", cam_num + 1);
    sprintf (cmd, "cp %s /var/www/html/cameras/%s", filename, name);
    system (cmd);
    return 0;
}

int grab_image (int cam_num)
{
    CURL *curl;
    CURLcode res;
    FILE* fp;
    char filename[1024];
    char buffer[1024];
   
    //if (verbose)
    //    fprintf (stdout, "Last grabbed CAM%i on %i\n", cam_num + 1, cams[cam_num].last_grabbed);
    
    if (cams[cam_num].last_grabbed + cams[cam_num].interval > time (NULL))
    {
        //if (verbose)
        //    fprintf (stdout, "Not grabbing, last grabbed %i, time %i\n", time (NULL));
        return;
    }

    build_filename(cam_num); 
    fp = fopen (cams[cam_num].filename, "w");
    if (fp == NULL)
    {
        fprintf (stderr, "Error opening %s for writing\n", cams[cam_num].filename);
        return 1;
    }
     
    curl = curl_easy_init();
    if (curl) 
    {
        curl_easy_setopt (curl, CURLOPT_TIMEOUT, 5); 
        curl_easy_setopt (curl, CURLOPT_URL, cams[cam_num].url);
        curl_easy_setopt (curl, CURLOPT_HTTPAUTH, (long)CURLAUTH_ANY);
        curl_easy_setopt (curl, CURLOPT_USERPWD, cams[cam_num].password);
        curl_easy_setopt (curl, CURLOPT_WRITEDATA, fp);
        
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        /* Perform the request, res will get the return code */ 
        res = curl_easy_perform(curl);
        
        /* Check for errors */ 
        if(res != CURLE_OK)
        {

            fprintf(stderr, "%s curl_easy_perform() failed for CAM%i: %s\n", get_time(), cam_num + 1, curl_easy_strerror(res));
            sprintf (buffer, "Error: !CURLE_OK Couldn't fetch image from CAM%i: %s\nurl: %s", cam_num + 1, curl_easy_strerror(res), cams[cam_num].url);
            log_text (buffer);
            fclose (fp);
            curl_easy_cleanup(curl);
            return 1;
        }

        build_html (cam_num, cams[cam_num].filename);

        /* always cleanup */
        curl_easy_cleanup(curl);
        fclose (fp);
        cams[cam_num].last_grabbed = (long) time (NULL);
        if (cams[cam_num].last_grabbed == -1 )
            fprintf (stderr, "Error getting time?\n");
        return 0;
     }
     else
     {
         curl_easy_cleanup(curl);
         sprintf (buffer, "Error: Couldn't fetch image from CAM%i\n", cam_num);
         log_text (buffer);
         return 1;
     }
}

static void sig_handler (int signo)
{
    if (running)
    {
        log_text ("Shutting down due to signal");
        remove_pid ();
        running = 0;
    }
}

void* camgrab (void *arg)
{
    int cam_num =  *((int *) arg);
    free (arg);

    while (running)
    {
        grab_image (cam_num);
    }
}

void* rotate_dirs (void *arg)
{
    int i;
    
    while (running)
    {
        for (i = 0; i < num_cams; i++)
        {
            if (cams[i].enabled)
                rotate (i);
        }
        sleep (60);
    }
}

static void create_threads (void)
{
    int i;
    int ret;
    /*
    ret = pthread_create (&rotate_thread, NULL, &rotate_dirs, NULL);
    if (ret != 0)
    {
        fprintf (stdout, "Error creating rotate thread\n");
        exit (1);
    }
*/
    for (i = 0; i < num_cams; i++)
    {
        if (cams[i].enabled)
        {
            fprintf (stdout, "CAM%i enabled\n", i + 1); 
            int *arg = malloc (sizeof (*arg));
            if (arg == NULL)
            {
                fprintf (stderr, "Error malloc for arg\n");
                exit (1);
            }
            
            *arg = i;
            
            ret = pthread_create (&cam_threads[i], NULL, &camgrab, arg);
            fprintf (stdout, "pthread returned %i\n", ret);
            
            if (ret != 0)
            {
                fprintf (stderr, "Error creating thread %i\n", i);
            }
            else
            {
                fprintf (stdout, "started thread %i\n", i);
            }
        }
    }
}

int main(int argc, char **argv)
{
    int c;

    verbose = 0;
    log_text ("Starting camgrab");
    running = 1;
    //Setup the sig handler
    if (signal (SIGINT, sig_handler) == SIG_ERR)
    {
        fprintf (stderr, "Couldn't setup sig_handler\n");
        return 1;
    }
     
    if (check_pid ())
        return 1;
    
    while ((c = getopt (argc, argv, "v")) != -1)
    {
        switch (c)
        {
            case 'v':
            verbose = 1;
            fprintf (stdout, "Verbose mode on\n");
            break;
            default:
            break;
        }
    }
    
    if (cfg_load ())
    {
        fprintf (stderr, "Error loading config file %s\n", DEFAULT_CFG_FILE);
        return 1;
    }
   
    
    create_threads ();

    while (running)
    {
    }
    fprintf (stdout, "Exiting\n");
    return 0;
}
