#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>

static int base = 0;                       /* GPIO number offset, 0 on older kernels */
module_param(base, int, 0);

static unsigned int ledGreen = 4;          /* GPIO4  (pin 7)  Green LED */
static unsigned int pushButton = 11;       /* GPIO11 (pin 23) Push button */
static unsigned int irqNumber;
static bool ledOn = 0;
static unsigned long last = 0;             /* time of last accepted press */

static irqreturn_t rpi_gpio_isr(int irq, void *dev_id)
{
    if (time_before(jiffies, last + msecs_to_jiffies(200)))
        return IRQ_HANDLED;                /* ignore switch bounce */
    last = jiffies;

    ledOn = !ledOn;                        /* toggle the LED state */
    gpio_set_value(ledGreen, ledOn);       /* set LED accordingly */
    printk(KERN_ALERT "GPIO Interrupt! Green LED %s\n", ledOn ? "ON" : "OFF");
    return IRQ_HANDLED;
}

static int __init rpi_gpio_init(void)
{
    int result;
    printk(KERN_ALERT "Initializing the GPIO LKM\n");
    ledGreen += base;
    pushButton += base;

    result = gpio_request(ledGreen, "sysfs");
    if (result) { printk(KERN_ALERT "Cannot get GPIO %d\n", ledGreen); return result; }
    result = gpio_request(pushButton, "sysfs");
    if (result) { printk(KERN_ALERT "Cannot get GPIO %d\n", pushButton); gpio_free(ledGreen); return result; }

    ledOn = true;
    gpio_direction_output(ledGreen, ledOn);    /* LED starts ON */
    gpio_direction_input(pushButton);

    irqNumber = gpio_to_irq(pushButton);
    printk(KERN_ALERT "Button mapped to IRQ: %d\n", irqNumber);

    result = request_irq(irqNumber, rpi_gpio_isr, IRQF_TRIGGER_RISING,
                         "rpi_gpio_handler", NULL);
    if (result) { gpio_free(ledGreen); gpio_free(pushButton); }
    return result;
}

static void __exit rpi_gpio_exit(void)
{
    free_irq(irqNumber, NULL);
    gpio_set_value(ledGreen, 0);           /* turn the LED off */
    gpio_free(ledGreen);
    gpio_free(pushButton);
    printk(KERN_ALERT "Goodbye from the GPIO LKM!\n");
}

module_init(rpi_gpio_init);
module_exit(rpi_gpio_exit);
MODULE_LICENSE("GPL");