#include "camgrab.h"
#include <stdlib.h>

static void cams_init (void)
{
    int i;

    for (i = 0; i < MAX_CAMS; i++)
    {
        cams[i].url = NULL;
        cams[i].dir = NULL;
        cams[i].password = NULL;
        cams[i].maxsize = 0;
        cams[i].interval = 0;
        cams[i].enabled = 0;
        cams[i].last_grabbed = 0;
    }
}

static void cfg_dump (void)
{
    int i;
    
    if (!verbose)
        return;
    fprintf (stdout, "Dumping camera config\n");
    fprintf (stdout, "num_cams: %i\n", num_cams);
    for (i = 0; i < num_cams; i++)
    {
        fprintf (stdout, "\n**** CAM%i ****\n", i + 1); 
        fprintf (stdout, "url = %s\n", cams[i].url);
        fprintf (stdout, "dir = %s\n", cams[i].dir);
        fprintf (stdout, "password = %s\n", cams[i].password);
        fprintf (stdout, "maxsize = %i\n", cams[i].maxsize);
        fprintf (stdout, "interval = %i\n", cams[i].interval);
        fprintf (stdout, "enabled = %i\n", cams[i].enabled);
    }
}

static int check_cfg_line (char* cfg_line)
{
    char name[5];
    char opt[256];
    int cam_num;

    strncpy (name, cfg_line, 4);
    name[5] = '\0';
    cam_num = atoi (name + 3);

    /* cam struct starts from 0 */
    if (cam_num > 0)
        cam_num -= 1;
    if (strncmp (cfg_line, "cam", 3) != 0 && strncmp (cfg_line, CFG_NUM_CAMS, strlen(CFG_NUM_CAMS)) != 0)
    {
        fprintf (stderr, "Error reading config, line doesn't start with cam: %s\n");
        return 1;
    }
    if (strncmp (cfg_line + 4, CFG_CAM_DIR, strlen (CFG_CAM_DIR)) == 0)
    {
        strcpy (opt, cfg_line + 4 + strlen (CFG_CAM_DIR));
        //Strip the newline
        opt[strlen (opt) - 1] = '\0';

        if (opt[strlen (opt) - 1] != '/')
             
        {
            fprintf (stderr, cfg_line);
            fprintf (stderr, "Directory does not have / at the end: %s\n", opt);
            return 1;
        }
        cams[cam_num].dir = malloc (sizeof (opt));
        strcpy (cams[cam_num].dir, opt);

    }
    else if (strncmp (cfg_line + 4, CFG_CAM_URL, strlen (CFG_CAM_URL)) == 0)
    {
        strcpy (opt, cfg_line + 4 + strlen (CFG_CAM_URL));
        opt[strlen (opt) - 1] = '\0';
        cams[cam_num].url = malloc (sizeof (opt));
        strcpy (cams[cam_num].url, opt);
    }
    else if (strncmp (cfg_line + 4, CFG_CAM_MAXSIZE, strlen (CFG_CAM_MAXSIZE)) == 0)
    {
        strcpy (opt, cfg_line + 4 + strlen (CFG_CAM_MAXSIZE));
        cams[cam_num].maxsize = atoi (opt);
    }
    else if (strncmp (cfg_line + 4, CFG_CAM_PASSWORD, strlen (CFG_CAM_PASSWORD)) == 0)
    {
        strcpy (opt, cfg_line + 4 + strlen (CFG_CAM_PASSWORD));
        opt[strlen (opt) - 1] = '\0';
        cams[cam_num].password = malloc (sizeof (opt));
        strcpy (cams[cam_num].password, opt);
    }
    else if (strncmp (cfg_line + 4, CFG_CAM_INTERVAL, strlen (CFG_CAM_INTERVAL)) == 0)
    {
        strcpy (opt, cfg_line + 4 + strlen (CFG_CAM_INTERVAL));
        cams[cam_num].interval = atoi (opt);
    }
    else if (strncmp (cfg_line + 4, CFG_CAM_ENABLED, strlen (CFG_CAM_ENABLED)) == 0)
    {
        strcpy (opt, cfg_line + 4 + strlen (CFG_CAM_ENABLED));
        if (opt[0] == 'y' || opt[0] == 'Y')
            cams[cam_num].enabled = 1;
        else
            cams[cam_num].enabled = atoi (opt);
    }
   return 0;
}


//Load the general configuration
//Set some default values before attempting to load config
int cfg_load (void)
{
    FILE *cfg_file;
    char cfg_line[1024];
    int len;
    char opt[256];
    
    num_cams = 0;

    if (verbose)
        fprintf (stdout, "Attempting to load configuration file %s\n", DEFAULT_CFG_FILE);
    cfg_file = fopen (DEFAULT_CFG_FILE, "r");
    
    if (cfg_file == NULL)
        return errno;


    while (fgets (cfg_line, 1024, cfg_file) != NULL)
    {
        if (cfg_line[0] != '#' && strlen (cfg_line) > 1)
        {
            if (strncmp (cfg_line, CFG_NUM_CAMS, strlen (CFG_NUM_CAMS)) == 0)
            {
                strcpy (opt, cfg_line + strlen (CFG_NUM_CAMS));
                num_cams = atoi (opt);
            }
        }
    }

    if (num_cams == 0)
    {
        fprintf (stderr, "Couldn't find num_cams in config file\n");
        return 1;
    }

    rewind (cfg_file);

    cams_init ();

    while (fgets (cfg_line, 1024, cfg_file) != NULL)
    {
        //Ignore remarks
        if (cfg_line[0] != '#' && strlen (cfg_line) > 1)
        {

            if (check_cfg_line (cfg_line))
            {
                fprintf (stderr, "Error reading config file\n");
                return 1;
            }
        }
    }

    fclose (cfg_file);
    cfg_dump ();
    return 0;
}
