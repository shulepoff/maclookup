#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#define VERSION "0.3 alfa"

struct globalArgs_t {
    int updOui;       /* option -u */
    int configure;    /* option -c */
    int information;  /* option -i */
    int usage;          /* option -h */
    char *macAddress; /* MAC */
} globalArgs;

static const char *optString = "ucihv";

enum { Folder, Url, nKeys};
char *keys[nKeys] = {
    [Folder] = "folder",
    [Url]     = "url",
};
char *config[nKeys];
char *trim(char *s) {
    char *p = s + strlen(s) -1;
    while (isascii(*s) && isspace(*s))
        s++;
    while (isascii(*p) && isspace(*p))
        *p-- = '\0';
    return s;
}

void read_conf(FILE *fd) 
{
    char buf[81], *s, *p;
    char *key, *value;
    int i;

    while (fgets(buf, sizeof(buf), fd) != NULL) {
        s = trim(buf);
        /* got comment or empty line? go ahead */
        if (*s == '#' || *s == '\0')
            continue;
        /* not a key=value pair? go ahead */
        p = strchr(s, '=');
        if (!p)
            continue;
        /* separate key and value */
        *p++ = '\0';
        /* trim again in case there was a whitespace around '=' */
        key = trim(s);
        value = trim(p);
        /* and store it finally */
        for (i = 0; i < nKeys; i++)
            if (strcmp(keys[i], key) == 0)
                config[i] = strdup(value);
    }    
}
void print_usage(FILE *handle) 
{
    fputs("Usage: maclookup [-u][-v][-c][-i][-h] [MAC address]\n",handle);
}
char *mac_sanitize(char *mac) 
{
    int i,j;
    /* UPPERCASE that AND remove ':' and '-' */
    for(i=0,j=0; mac[i]; i++) {
        if(mac[i] >= 'a' && mac[i] <= 'z') {
            mac[i] = mac[i]-32;
        }
        if (mac[i]==':' || mac[i]=='-') continue;
        if (j<6) mac[j] = mac[i]; else mac[j]=0 ;
        j++;
    }
    mac[j]=0;
    return mac;
}
void open_conf(void) 
{
     char *home, *filename;
     home = getenv("HOME");
     char *fname = "/.maclookup";
     filename = strcat(home,fname);
     FILE *f = fopen(filename,"r");
     if (!f) {
         f = fopen(filename,"w");
         fprintf(f, "folder = /usr/share/ieee-data/\n");
         fprintf(f, "url = https://linuxnet.ca/ieee/oui.txt.bz2\n");
         printf("File %s created\n", filename);
         fclose(f);
         f = fopen(filename,"r");
     }
     read_conf(f);
     fclose(f);
}    

void print_info(void) 
{
    printf("Folder ieee-data is %s \n",config[Folder]);
    printf("URL for update oui.txt is %s \n",config[Url]);
}

void mac_lookup(void) 
{
    char temp[512];
    FILE *file = fopen(strcat(config[Folder],"/oui.txt"), "r");

    if (!file) {
        fprintf(stderr, "Error opening file \n");
        exit( EXIT_FAILURE );
    }
    while(fgets(temp,sizeof(temp),file) != NULL) {
        char oui[7];
        if ((strstr(temp, globalArgs.macAddress)) != NULL) {
            strncpy(oui,temp,6);
            printf("%s %s",oui,temp+20);
            break;
        }
    }
    
    if(file) {
        fclose(file);
    }
}

void update_oui(void) 
{
    char str[255];
    /* 1 - get oui.txt from web */
    strcpy(str, "curl ");
    strcat(str, config[Url]);
    strcat(str, " | bzip2 -d > /tmp/oui.txt");
    printf("%s\n",str);
    
    FILE * f = popen(str,"r");
    if (f==0) {
        fprintf(stderr, "Could not update oui.txt \n");
        exit(EXIT_FAILURE);
    }
    pclose(f);
    
    /* 2 - copy oui.txt into directory from config */ 
    strcpy(str,"cp /tmp/oui.txt ");
    strcat(str, config[Folder]);
    strcat(str, "oui.txt");
    f = popen(str,"r");
    if (f==0) {
        fprintf(stderr,"Could not copy oui.txt into directory \n");
        exit( EXIT_FAILURE );
    }
    pclose(f);
}

int main(int argc, char *argv[] ) 
{
    int opt = 0;
    /* Init globalArgs before works */
    globalArgs.updOui = 0;  /* False */
    globalArgs.configure = 0;
    globalArgs.usage = 0;
    globalArgs.information = 0;
    globalArgs.macAddress = NULL;

    while((opt = getopt(argc, argv, optString)) != -1) {
            switch(opt) {
                case 'u':globalArgs.updOui = 1; break;
                case 'c':globalArgs.configure = 1; break;
                case 'i':globalArgs.information = 1; break;
                case 'h':globalArgs.usage = 1; break;
                case 'v':printf("Maclookup v %s \n", VERSION); exit(0);
                case '?':globalArgs.usage = 2; break;
                default: globalArgs.usage = 2; break;
            }
    }
    if (globalArgs.usage == 2) {
        print_usage(stderr);
        fputs("Try `maclookup -h' for more information.\n",stderr);
        exit(EXIT_FAILURE);
    }
    if ( globalArgs.usage == 1) {
        print_usage(stdout);
        fputs( "OPTIONS: \n",stdout);
        fputs( " -c\tcreate config file\n",stdout);
        fputs( " -i\tdisplay info from configuration file\n",stdout);
        fputs( " -u\tupdate oui.txt from web\n",stdout);
        fputs( " -v\tdisplay program version\n",stdout);
        fputs( " -h\tdisplay this help\n",stdout);
        exit(EXIT_SUCCESS);
    }
    open_conf();
    if (globalArgs.updOui == 1) {
        update_oui();
        exit(EXIT_SUCCESS);
    }
    if (globalArgs.information == 1) {
        print_info();
        exit( EXIT_SUCCESS );
    }

    globalArgs.macAddress = argv[optind];
    if (globalArgs.macAddress) {
        globalArgs.macAddress = mac_sanitize(globalArgs.macAddress);
        if (strlen(globalArgs.macAddress)<6) {
            fputs("MAC must be 6 char min\n",stderr);
            print_usage(stderr);
            exit(EXIT_FAILURE);
        }
        mac_lookup();
    }
    return (EXIT_SUCCESS);
}
