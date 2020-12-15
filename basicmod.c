#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
static struct timer_list blink_timer;
/*
 * The module commandline arguments ...
 */
static int speed = 400;
static int ios[2] = {-1, -1};
static int ios_len = 0;
module_param(speed, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(speed, "IO toggle speed in ms (int)");
module_param_array(ios, int, &ios_len, 0000);
MODULE_PARM_DESC(ios, "IO's to toggle [int]");
/* Define pins, directin and inital state of GPIOs for LEDs */
static struct gpio leds[2];
/* Tasklet to blink the LEDs */
static void blink(struct timer_list *t)
{
    int i;
    printk(KERN_INFO "%s\n", __func__);
    printk("Starting blink\n");
    for (i = 0; i < ios_len; i++)
    {
        if (ios[i])
        {
            gpio_set_value(leds[i].gpio, 1);
            mdelay(500);
            gpio_set_value(leds[i].gpio, 0);
        }
    }
    blink_timer.expires = jiffies + ((speed / 1000) * HZ); // 1 sec.
    add_timer(&blink_timer);
    printk("blink ended\n");
}
static int __init basicMod_init(void)
{
    int ret = 0;
    int i;
    struct gpio o1, o2;
    printk(KERN_INFO "%s\n", __func__);
    printk(KERN_INFO "Speed: %d\n", speed);
    printk(KERN_INFO "Toggling the following IO's:");
    for (i = 0; i < (sizeof ios / sizeof(int)); i++)
    {
        printk(KERN_INFO "ios[%d] = %d\n", i, ios[i]);
    }
    //check if 1 io was passed
    if (ios[0])
    {
        o1.gpio = ios[0];
        o1.flags = GPIOF_OUT_INIT_LOW;
        o1.label = "LED1";
        leds[0] = o1;
    }
    //check if io 2 was passed
    if (ios[1])
    {
        o2.gpio = ios[1];
        o2.flags = GPIOF_OUT_INIT_LOW;
        o2.label = "LED2";
        leds[1] = o2;
    }
    //register, turn on
    ret = gpio_request_array(leds, ARRAY_SIZE(leds));
    if (ret)
    {
        printk(KERN_ERR "Unable to request GPIOs: %d\n", ret);
        return ret;
    }
    /* init timer, add timer function */
    timer_setup(&blink_timer, blink, 0);
    blink_timer.function = blink;
    //blink_timer.data = 1L; // initially turn LED on
    blink_timer.expires = jiffies + (1 * HZ); // 1 sec.
    add_timer(&blink_timer);
    /* 
 * A non 0 return means init_module failed; module can't be loaded. 
 */
    return 0;
}
static void __exit basicMod_exit(void)
{
    int i;
    printk(KERN_INFO "%s\n", __func__);
    // deactivate timer if running
    del_timer_sync(&blink_timer);
    // turn all leds off
    for (i = 0; i < ios_len; i++)
    {
        gpio_set_value(leds[i].gpio, 0);
    }
    //free ios
    gpio_free_array(leds, ARRAY_SIZE(leds));
}
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Robin VH");
MODULE_DESCRIPTION("Basic Linux Kernel module");
module_init(basicMod_init);
module_exit(basicMod_exit);