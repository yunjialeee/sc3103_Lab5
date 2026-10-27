#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>

int main(void)
{
    int fd0 = open("/dev/gpiochip0", O_RDWR);
    struct gpiohandle_request req_GY, req_R, req_B;
    struct gpiohandle_data data_GY, data_R, data_B;
    int i;

    /* ---- A1: blink Green/Yellow for 5 s ---- */
    memset(&req_GY, 0, sizeof(req_GY));
    req_GY.lines = 2;
    req_GY.lineoffsets[0] = 4;     /* Green  */
    req_GY.lineoffsets[1] = 17;    /* Yellow */
    req_GY.flags = GPIOHANDLE_REQUEST_OUTPUT;
    ioctl(fd0, GPIO_GET_LINEHANDLE_IOCTL, &req_GY);

    data_GY.values[0] = 1;
    data_GY.values[1] = 0;
    for (i = 0; i < 5; i++) {
        ioctl(req_GY.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data_GY);
        usleep(1000000);
        data_GY.values[0] = !data_GY.values[0];
        data_GY.values[1] = !data_GY.values[1];
    }
    data_GY.values[0] = 0;         /* stop: both off */
    data_GY.values[1] = 0;
    ioctl(req_GY.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data_GY);

    /* ---- A2: wait for button, then blink Red for 10 s ---- */
    memset(&req_R, 0, sizeof(req_R));
    req_R.lines = 1;
    req_R.lineoffsets[0] = 27;     /* Red LED */
    req_R.flags = GPIOHANDLE_REQUEST_OUTPUT;
    ioctl(fd0, GPIO_GET_LINEHANDLE_IOCTL, &req_R);

    memset(&req_B, 0, sizeof(req_B));
    req_B.lines = 1;
    req_B.lineoffsets[0] = 11;     /* Button */
    req_B.flags = GPIOHANDLE_REQUEST_INPUT;
    ioctl(fd0, GPIO_GET_LINEHANDLE_IOCTL, &req_B);

    printf("Press the button...\n");
    do {                           /* wait until button is pressed */
        ioctl(req_B.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data_B);
        usleep(10000);
    } while (data_B.values[0] == 0);

    printf("Pressed! Blinking Red for 10 s\n");
    data_R.values[0] = 1;
    for (i = 0; i < 10; i++) {
        ioctl(req_R.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data_R);
        usleep(1000000);
        data_R.values[0] = !data_R.values[0];
    }
    data_R.values[0] = 0;          /* Red off at the end */
    ioctl(req_R.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data_R);

    close(req_GY.fd); close(req_R.fd); close(req_B.fd); close(fd0);
    return 0;
}
