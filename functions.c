#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/stat.h>
#include <assert.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include <libimobiledevice/afc.h>
#include <libimobiledevice/notification_proxy.h>
#include "prototypes.h"


int qwrite(afc_client_t afc, const char* from, const char *to)
{
	printf("Sending %s -> %s... ", from, to);

    uint64_t handle = 0;
	FILE *fd = fopen(from, "r");
	uint32_t bytes = 0;
	size_t totbytes = 0;
	char buf [8192];
    afc_error_t rtn = afc_file_open(afc, to, AFC_FOPEN_WRONLY, &handle);
    if (rtn != AFC_E_SUCCESS)
    {
        printf("Error !\nImpossible to prepare to send the file to %s !\n", to);
        return EXIT_FAILURE;
    }
    uint32_t bytes_written = 0;
    while(rtn==AFC_E_SUCCESS && (bytes=fread(buf, 1, 8192, fd)) > 0)
    {
        bytes_written = 0;
        rtn = afc_file_write(afc, handle, buf, bytes, &bytes_written);
        totbytes += bytes_written;
    }
    if (rtn)
    {
        printf("Error ! Can't copy %s to %s", from, to);
        return EXIT_FAILURE;
    }

	if(afc_file_close(afc, handle) != AFC_E_SUCCESS)
	{
        printf("Error !\nImpossible to close the afc connection !\n");
        return EXIT_FAILURE;
	}
	fclose(fd);

	printf("Transfer done !\n");
	return EXIT_SUCCESS;
}
