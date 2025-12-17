#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/videodev2.h>


/**
 * main - Opens the camera device, gets and sets the absolute exposure control.
 */
int main(int argc, char *argv[]) {
    int fd;
    struct v4l2_control ctrl;

    char *device = NULL;
    int exposure = -1;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            device = argv[++i];  // get next argument
        } else if (strcmp(argv[i], "-e") == 0 && i + 1 < argc) {
            exposure = atoi(argv[++i]);  // get next argument and convert
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            fprintf(stderr, "Usage: %s -d <device> -e <exposure>\n", argv[0]);
            return 1;
        }
    }
    
    // Validate required arguments
    if (device == NULL) {
        fprintf(stderr, "Error: -d <device> is required\n");
        return 1;
    }
    
    if (exposure == -1) {
        fprintf(stderr, "Error: -e <exposure> is required\n");
        return 1;
    }
    
    // 1. Open the video device
    fd = open(device, O_RDWR);
    if (fd == -1) {
        fprintf(stderr, "Error opening device %s\n", device);
        return 1;
    }

    // --- 2. Get Current Exposure Value ---
    
    // Initialize the control structure
    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_EXPOSURE_ABSOLUTE;

    // Check if the control is supported and get its current value
    if (ioctl(fd, VIDIOC_G_CTRL, &ctrl) == -1) {
        if (errno == EINVAL) {
            fprintf(stderr, "Error: V4L2_CID_EXPOSURE_ABSOLUTE is not supported by this device.\n");
        } else {
            perror("Error getting V4L2_CID_EXPOSURE_ABSOLUTE");
        }
        close(fd);
        return 1;
    }
    
    printf("--- Current Exposure ---\n");
    printf("Current Exposure Value: %d\n", ctrl.value);
    
    // Check if Auto Exposure is enabled (if supported)
    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_EXPOSURE_AUTO;
    if (ioctl(fd, VIDIOC_G_CTRL, &ctrl) == 0) {
        printf("Current Exposure Mode: %s\n", 
               (ctrl.value == V4L2_EXPOSURE_MANUAL) ? "Manual" : 
               (ctrl.value == V4L2_EXPOSURE_APERTURE_PRIORITY) ? "Aperture Priority" : 
               "Automatic");
    }

    // --- 3. Set Manual Exposure Mode (Necessary for absolute control) ---
    
    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_EXPOSURE_AUTO;
    ctrl.value = V4L2_EXPOSURE_MANUAL;
    
    if (ioctl(fd, VIDIOC_S_CTRL, &ctrl) == -1) {
        // This might fail if the camera only supports AUTO or if it's already manual.
        // It's often required to set manual mode before setting absolute exposure.
        if (errno != EINVAL) {
            perror("Warning: Could not set exposure mode to Manual");
        }
    } else {
        printf("\nSuccessfully set exposure mode to Manual.\n");
    }


    // --- 4. Set New Absolute Exposure Value ---
    
    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_EXPOSURE_ABSOLUTE;
    ctrl.value = exposure;

    if (ioctl(fd, VIDIOC_S_CTRL, &ctrl) == -1) {
        perror("Error setting V4L2_CID_EXPOSURE_ABSOLUTE");
        close(fd);
        return 1;
    }

    // --- 5. Verify the New Exposure Value ---
    
    // Get the value again to verify it was set correctly (V4L2 may clip the value)
    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_EXPOSURE_ABSOLUTE;
    
    if (ioctl(fd, VIDIOC_G_CTRL, &ctrl) == -1) {
        perror("Error verifying V4L2_CID_EXPOSURE_ABSOLUTE");
        close(fd);
        return 1;
    }
    
    printf("\n--- New Exposure Set ---\n");
    printf("Requested Exposure: %d\n", exposure);
    printf("Verified Exposure Value (may be clipped): %d\n", ctrl.value);

    // 6. Close the device
    close(fd);
    
    return 0;
}