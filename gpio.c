#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>

int main(void)
{
    int fd0 = open("/dev/gpiochip0", O_RDWR);
    struct gpiochip_info cinfo;
    struct gpiohandle_request req_GY, req_R, req_B;
    struct gpiohandle_data data_GY, data_R, data_B;
    int i, prev = 0;

    ioctl(fd0, GPIO_GET_CHIPINFO_IOCTL, &cinfo);
    printf("GPIO chip 0: %s, \"%s\", %u lines\n", cinfo.name, cinfo.label, cinfo.lines);

    /* ---- A1: blink Green/Yellow for 5 s ---- */
    memset(&req_GY, 0, sizeof(req_GY));
    req_GY.lines = 2;
    req_GY.lineoffsets[0] = 4;              /* Green  */
    req_GY.lineoffsets[1] = 17;             /* Yellow */
    req_GY.flags = GPIOHANDLE_REQUEST_OUTPUT;
    ioctl(fd0, GPIO_GET_LINEHANDLE_IOCTL, &req_GY);

    data_GY.values[0] = 1;
    data_GY.values[1] = 0;
    printf("A1: blinking Green/Yellow for 5 s\n");
    for (i = 0; i < 5; i++) {
        ioctl(req_GY.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data_GY);
        usleep(1000000);
        data_GY.values[0] = !data_GY.values[0];
        data_GY.values[1] = !data_GY.values[1];
    }
    data_GY.values[0] = 0;                  /* both off */
    data_GY.values[1] = 0;
    ioctl(req_GY.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data_GY);

    /* ---- A2: button toggles Red for ~10 s ---- */
    memset(&req_R, 0, sizeof(req_R));
    req_R.lines = 1;
    req_R.lineoffsets[0] = 27;              /* Red LED */
    req_R.flags = GPIOHANDLE_REQUEST_OUTPUT;
    ioctl(fd0, GPIO_GET_LINEHANDLE_IOCTL, &req_R);

    memset(&req_B, 0, sizeof(req_B));
    req_B.lines = 1;
    req_B.lineoffsets[0] = 11;              /* Push button */
    req_B.flags = GPIOHANDLE_REQUEST_INPUT;
    ioctl(fd0, GPIO_GET_LINEHANDLE_IOCTL, &req_B);

    data_R.values[0] = 0;
    printf("A2: press the button to toggle Red (10 s)\n");
    for (i = 0; i < 200; i++) {             /* 200 x 50 ms = 10 s */
        ioctl(req_B.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data_B);
        if (data_B.values[0] && !prev) {    /* new press (0 -> 1) */
            data_R.values[0] = !data_R.values[0];
            ioctl(req_R.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data_R);
            printf("Button pressed -> Red %s\n", data_R.values[0] ? "ON" : "OFF");
        }
        prev = data_B.values[0];
        usleep(50000);                      /* 50 ms also filters switch bounce */
    }
    data_R.values[0] = 0;                   /* Red off at the end */
    ioctl(req_R.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data_R);
    printf("Done\n");

    close(req_GY.fd); close(req_R.fd); close(req_B.fd); close(fd0);
    return 0;
}