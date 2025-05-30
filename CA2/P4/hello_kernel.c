#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
static int __init hello_world_init(void) {
		printk(KERN_INFO "Hello, Kernel!\n");
		return 0;
}
static void __exit hello_world_exit(void) {
		printk(KERN_INFO "Goodbye, Kernel!\n");
}
module_init(hello_world_init);
module_exit(hello_world_exit);
MODULE_LICENSE("BSD");
MODULE_AUTHOR("Mahdi Vajhi");
MODULE_DESCRIPTION("A simple hello world driver");
MODULE_VERSION("1.0.0v");
