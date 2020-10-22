#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

struct globalArgs_t {
	int updOui;		/* option -u */
	int configure;	/* option -c */
	int information; /* option -i */
	char *macAddress; 
} globalArgs;

static const char *optString = "ucih?";

enum { Folder, Url, nKeys};
char *keys[nKeys] = {
	[Folder] = "folder",
	[Url]	 = "url",
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

void rdconf(FILE *fd) {
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
void display_usage(void) {
	puts( "Maclookup 0.01 ( https://github.com/shulepoff/maclookup )");
	puts( "	Display Vendor Information by MAC address");
	puts( "USAGE: ");
	puts( "	maclookup [-ucih] XX:XX:XX ");
	puts( "OPTIONS: ");
	puts( "	-c	- create config file");
	puts( "	-i	- display info from configuration file");
	puts( "	-u	- update oui.txt from web");
	puts( "	-h	- display this help");
	exit( EXIT_FAILURE);
}
char *mac_sanitize(char *mac){
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
void opncfg(void){
     char *home, *filename;
     home = getenv("HOME");
     char *fname = "/.maclookup";
     filename = strcat(home,fname);
     FILE *f = fopen(filename,"r");
     if (f) {
		 rdconf(f);
     }
     else {
         f = fopen(filename,"w");
         printf("File %s created\n", filename);
     }
     fclose(f);
}	
void info_display(void){
	printf("Folder ieee-data is %s \n",config[Folder]);
	printf("URL for update oui.txt is %s \n",config[Url]);
	exit( EXIT_FAILURE );
}
void mac_lookup(void) {
	char temp[512];
	FILE *file = fopen(strcat(config[Folder],"/oui.txt"), "r");

	if (!file) {
		fprintf(stderr, "Error opening file \n");
		exit( EXIT_FAILURE );
	}
	while(fgets(temp,sizeof(temp),file) != NULL) {
		if ((strstr(temp, globalArgs.macAddress)) != NULL) {
			printf("\n%s\n",temp);
		}
	}
	
	if(file) {
		fclose(file);
	}
}
void update_oui(void) {
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
	strcat(str, "/oui.txt");
	f = popen(str,"r");
	if (f==0) {
		fprintf(stderr,"Could not copy oui.txt into directory \n");
		exit( EXIT_FAILURE );
	}
	pclose(f);
}
int main(int argc, char *argv[] ) {
	opncfg();
	//printf("folder:   %s\n", config[Folder]);
	//printf("url:    %s\n", config[Url]);
	int opt = 0;
	/* Init globalArgs before works */
	globalArgs.updOui = 0;  /* False */
	globalArgs.configure = 0;
	globalArgs.information = 0;
	globalArgs.macAddress = NULL;

	while((opt = getopt(argc, argv, optString)) != -1) {
			switch(opt){
				case 'u':
					globalArgs.updOui = 1; /* True */
					update_oui();
					break;
				case 'c':
					globalArgs.configure = 1;
					break;
				case 'i':
					globalArgs.information = 1;
					info_display();
					break;
				case 'h':
				case '?':
					display_usage();
					break;
				default:
					fprintf(stderr,"getopt");
			}
	}
	globalArgs.macAddress = argv[optind];
	if (globalArgs.macAddress){
		globalArgs.macAddress = mac_sanitize(globalArgs.macAddress);
	/*
	printf("Update = %d, Config = %d, Information = %d \n", globalArgs.updOui, globalArgs.configure, globalArgs.information);
	*/
	// printf("MAC %s\n", globalArgs.macAddress);
		mac_lookup();
	}
	return (EXIT_SUCCESS);
}
