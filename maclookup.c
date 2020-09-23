#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

struct globalArgs_t {
	int updOui;		/* option -u */
	int configure;	/* option -c */
	int information; /* option -i */
	char *macAddress; 
} globalArgs;

static const char *optString = "ucih?";

void display_usage(void) {
	puts( "maclookup - Display Vendor Information by MAC address");
	puts( "USAGE: ");
	puts( "maclookup [-ucih] XX:XX:XX ");
	exit( EXIT_FAILURE);
}
void mac_lookup(void) {
	char temp[512];
	FILE *file = fopen("oui.txt", "r");
	if (!file) {
		fprintf(stderr, "Error opening file");
		exit( EXIT_FAILURE );
	}
	while(fgets(temp,512,file) != NULL) {
		if ((strstr(temp, globalArgs.macAddress)) != NULL) {
			printf("\n%s\n",temp);
		}
	}
	
	if(file) {
		fclose(file);
	}
}
int main(int argc, char *argv[] ) {
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
					break;
				case 'c':
					globalArgs.configure = 1;
					break;
				case 'i':
					globalArgs.information = 1;
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
	/*
	printf("Update = %d, Config = %d, Information = %d \n", globalArgs.updOui, globalArgs.configure, globalArgs.information);
	*/
	mac_lookup();
	return (EXIT_SUCCESS);
}
