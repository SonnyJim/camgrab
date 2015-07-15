#include "camgrab.h"

char time_str[24];
char filename[1024];

char* get_time (void)
{
    time_t rawtime;
    struct tm * timeinfo;
    
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    strcpy (time_str, asctime (timeinfo));
    time_str[strlen(time_str) - 1] = 0;
    return time_str;
}

char* get_year (void)
{
    time_t current_time;
    struct tm * time_info;

    time (&current_time);
    time_info = localtime(&current_time);

    strftime(time_str, sizeof(time_str), "%Y", time_info);
    return time_str;
}

char* get_month (void)
{
    time_t current_time;
    struct tm * time_info;

    time (&current_time);
    time_info = localtime(&current_time);

    strftime(time_str, sizeof(time_str), "%m", time_info);
    return time_str;
}

char* get_day (void)
{
    time_t current_time;
    struct tm * time_info;

    time (&current_time);
    time_info = localtime(&current_time);

    strftime(time_str, sizeof(time_str), "%d", time_info);
    return time_str;
}

char* get_hour (void)
{
    time_t current_time;
    struct tm * time_info;

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
char* build_filename (int cam_num)
{
    char dir_name[1024];
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
    strcpy (filename, dir_name);
    sprintf (dir_name, "CAM%i", cam_num + 1);
    strcat (filename, get_filename_time (cams[cam_num].dir, dir_name));

    return filename;

}

int grab_image (int cam_num)
{
    CURL *curl;
    CURLcode res;
    FILE* fp;
    char filename[1024];
    char buffer[1024];
   
    if (verbose)
        fprintf (stdout, "Last grabbed CAM%i on %i\n", cam_num, cams[cam_num].last_grabbed);
    
    if (cams[cam_num].last_grabbed + cams[cam_num].interval > time (NULL))
    {
        if (verbose)
            fprintf (stdout, "Not grabbing, last grabbed %i, time %i\n", time (NULL));
        return;
    }

    strcpy (filename, build_filename(cam_num)); 
    fp = fopen (filename, "w");
    if (fp == NULL)
    {
        fprintf (stderr, "Error opening %s for writing\n", filename);
        return 1;
    }
    
    curl = curl_easy_init();
    if(curl) 
    {
        curl_easy_setopt(curl, CURLOPT_URL, cams[cam_num].url);
        curl_easy_setopt (curl, CURLOPT_HTTPAUTH, (long)CURLAUTH_ANY);
        curl_easy_setopt (curl, CURLOPT_USERPWD, cams[cam_num].password);
        curl_easy_setopt (curl, CURLOPT_WRITEDATA, fp);

         curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
         /* Perform the request, res will get the return code */ 
         res = curl_easy_perform(curl);
         
         /* Check for errors */ 
         if(res != CURLE_OK)
         {
             fprintf(stderr, "curl_easy_perform() failed for CAM%i: %s\n", cam_num, curl_easy_strerror(res));
             sprintf (buffer, "Error: Couldn't fetch image from CAM%i\n", cam_num);
             log_text (buffer);
             curl_easy_cleanup(curl);
             return 1;
         }
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
/*
//Function to rotate files once a directory hits a certain size
void rotate (void)
{
    if (get_dir_size (CAM1_DIR) > MAX_DIR_SIZE)
        rotate_dir (CAM1_DIR);
    
    if (get_dir_size (CAM2_DIR) > MAX_DIR_SIZE)
        get_dir_size (CAM2_DIR);

    if (get_dir_size (CAM3_DIR) > MAX_DIR_SIZE)
        get_dir_size (CAM3_DIR);
    
    if (get_dir_size (CAM4_DIR) > MAX_DIR_SIZE)
    get_dir_size (CAM4_DIR);
}
*/

static void rotate (int cam_num)
{
    if (get_dir_size (cams[cam_num].dir) > cams[cam_num].maxsize)
        rotate_dir (cams[cam_num].dir, cams[cam_num].maxsize);
}

static void grab_images (void)
{
    int i;

    for (i = 0; i < MAX_CAMS; i++)
    {
        if (cams[i].enabled)
        {
            grab_image (i);
            rotate (i);
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

    while (running)
    {
        grab_images ();
    }
    fprintf (stdout, "Exiting\n");
    return 0;
}
