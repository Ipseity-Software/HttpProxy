#include <plugin.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <dirent.h>
char *resolve_plugin(const char *host)
{
	void *dlyd; // a pointer to our dylib
	char *(*plugin)(const char *); //a pointer to our plugin's entry point
	DIR *dp; // a pointer to our directory
	struct dirent *entity; // will hold information about our directory entity
	char *fp; // buffer to our file path
	struct stat fpstat; // allows us to stat a file to determine if it's what we're looking for
	char *retval; // return value from plugin

	retval = NULL; // initialize retval
	dp = opendir(PLUGIN_DIR); // open current running directory
	if (dp != NULL) // this should never fail, unless we don't have permissions
	{
		while ((entity = readdir(dp)) && retval == NULL) // read loop for all entries in directory, end if found
		{
			if (strstr(entity->d_name, ".vx") != NULL) // if the file has ".vx" in its name
			{
				fp = malloc(strlen(PLUGIN_DIR) + strlen(entity->d_name) + 8); // allocate our buffer
				*fp = 0; // set a null for security
				strcpy(fp, PLUGIN_DIR); // copy base path
				strcpy(fp + strlen(fp), entity->d_name); // copy file name
				stat(fp, &fpstat); // get file statisticts
				if (fpstat.st_mode & (S_IFREG | S_IRUSR | S_IXUSR)) // regular file with read and execute permissions
				{
					dlyd = dlopen(fp, RTLD_LOCAL | RTLD_LAZY); // open the library
					*(void**)(&plugin) = dlsym(dlyd, "resolve"); // find the resolve function
					if (plugin != NULL) // if we found it
						retval = strdup((*plugin)(host)); // send data to plugin and record result
					dlclose(dlyd); // close our handle to the plugin
				}
				free(fp); // clear our buffer for next run
			}
		}
		closedir(dp);
	}
	return retval;
}
