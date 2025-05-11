#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/list.h>

// Essential module information
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mahdi Vajhi");
MODULE_DESCRIPTION("Character-by-Character Virtual LIFO Driver");
MODULE_VERSION("1.0v");

// Device configurations with unique names
#define DEVICE_NAME_READ "lifo_read"
#define DEVICE_NAME_WRITE "lifo_write"
#define CLASS_NAME "lifo"
#define LIFO_MAX_SIZE (1024 * 1024)  // 1 MB buffer size

// Device numbers and structures
static dev_t dev_number;
static struct class *lifo_class = NULL;
static struct device *lifo_read_device = NULL;
static struct device *lifo_write_device = NULL;
static struct cdev lifo_read_cdev, lifo_write_cdev;

// LIFO character structure
struct lifo_char {
    char data;
    struct list_head list;
};

// LIFO stack and synchronization
static LIST_HEAD(lifo_stack);
static size_t total_size = 0;
static DEFINE_MUTEX(lifo_mutex);
static DECLARE_WAIT_QUEUE_HEAD(lifo_read_queue);

// Device operations prototypes
static int lifo_open(struct inode *inode, struct file *file);
static int lifo_release(struct inode *inode, struct file *file);
static ssize_t lifo_read(struct file *file, char __user *buf, size_t count, loff_t *offset);
static ssize_t lifo_write(struct file *file, const char __user *buf, size_t count, loff_t *offset);

// File operations structures
static const struct file_operations lifo_read_fops = {
    .owner = THIS_MODULE,
    .open = lifo_open,
    .release = lifo_release,
    .read = lifo_read,
};

static const struct file_operations lifo_write_fops = {
    .owner = THIS_MODULE,
    .open = lifo_open,
    .release = lifo_release,
    .write = lifo_write,
};

// Device operations implementations
static int lifo_open(struct inode *inode, struct file *file)
{
    try_module_get(THIS_MODULE);
    return 0;
}

static int lifo_release(struct inode *inode, struct file *file)
{
    module_put(THIS_MODULE);
    return 0;
}

static ssize_t lifo_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{
    struct lifo_char *node;
    ssize_t bytes_read = 0;
    char *temp_buffer;
    int i, ret;
    
    // Handle non-blocking mode
    if ((file->f_flags & O_NONBLOCK) && list_empty(&lifo_stack))
        return 0;  // Return EOF for empty LIFO
        
    // Wait until LIFO has data
    if (wait_event_interruptible(lifo_read_queue, !list_empty(&lifo_stack)))
        return -ERESTARTSYS;
        
    mutex_lock(&lifo_mutex);
    
    // Double check LIFO after acquiring lock
    if (list_empty(&lifo_stack)) {
        mutex_unlock(&lifo_mutex);
        return 0;  // Return EOF for empty LIFO
    }
    
    // Calculate how many bytes to read (min of requested count and available chars)
    bytes_read = min(count, total_size);
    if (bytes_read == 0) {
        mutex_unlock(&lifo_mutex);
        return 0;
    }
    
    // Allocate temporary buffer for reading
    temp_buffer = kmalloc(bytes_read, GFP_KERNEL);
    if (!temp_buffer) {
        mutex_unlock(&lifo_mutex);
        return -ENOMEM;
    }
    
    // Read characters from LIFO into temporary buffer
    for (i = 0; i < bytes_read && !list_empty(&lifo_stack); i++) {
        node = list_first_entry(&lifo_stack, struct lifo_char, list);
        temp_buffer[i] = node->data;
        list_del(&node->list);
        kfree(node);
        total_size--;
    }
    
    // Copy data to user space
    ret = copy_to_user(buf, temp_buffer, bytes_read);
    kfree(temp_buffer);
    
    if (ret) {
        mutex_unlock(&lifo_mutex);
        return -EFAULT;
    }
    
    mutex_unlock(&lifo_mutex);
    return bytes_read;
}

static ssize_t lifo_write(struct file *file, const char __user *buf, size_t count, loff_t *offset)
{
    struct lifo_char *new_node;
    char *temp_buffer;
    size_t bytes_to_write = count;
    int i;
    
    if (count == 0)
        return 0;
    
    // Limit the size of what we can write at once
    if (bytes_to_write > LIFO_MAX_SIZE)
        bytes_to_write = LIFO_MAX_SIZE;
    
    // Allocate temporary buffer
    temp_buffer = kmalloc(bytes_to_write, GFP_KERNEL);
    if (!temp_buffer)
        return -ENOMEM;
    
    // Copy data from user space to temporary buffer
    if (copy_from_user(temp_buffer, buf, bytes_to_write)) {
        kfree(temp_buffer);
        return -EFAULT;
    }
    
    mutex_lock(&lifo_mutex);
    
    // Check if we have enough space
    if (total_size + bytes_to_write > LIFO_MAX_SIZE) {
        mutex_unlock(&lifo_mutex);
        kfree(temp_buffer);
        return -ENOSPC;  // No space left
    }
    
    // Add each character to the LIFO stack individually
    for (i = 0; i < bytes_to_write; i++) {
        // Create new node for each character
        new_node = kmalloc(sizeof(struct lifo_char), GFP_KERNEL);
        if (!new_node) {
            mutex_unlock(&lifo_mutex);
            kfree(temp_buffer);
            return i > 0 ? i : -ENOMEM;  // Return how many we managed to write
        }
        
        // Store the character
        new_node->data = temp_buffer[i];
        
        // Add node to the beginning of the list (LIFO behavior)
        list_add(&new_node->list, &lifo_stack);
        total_size++;
    }
    
    mutex_unlock(&lifo_mutex);
    kfree(temp_buffer);
    
    // Wake up any reading processes
    wake_up_interruptible(&lifo_read_queue);
    
    return bytes_to_write;
}

static void __exit lifo_exit(void)
{
    struct lifo_char *node, *tmp;
    
    // Free all nodes in the LIFO
    list_for_each_entry_safe(node, tmp, &lifo_stack, list) {
        list_del(&node->list);
        kfree(node);
    }
    
    // Clean up devices and resources
    device_destroy(lifo_class, MKDEV(MAJOR(dev_number), 1));
    device_destroy(lifo_class, dev_number);
    cdev_del(&lifo_write_cdev);
    cdev_del(&lifo_read_cdev);
    class_destroy(lifo_class);
    unregister_chrdev_region(dev_number, 2);
    
    printk(KERN_INFO "New LIFO Driver: unloaded successfully\n");
}

static int __init lifo_init(void)
{
    int ret;
    
    printk(KERN_INFO "New LIFO Driver: initializing\n");
    
    // Initialize LIFO list
    INIT_LIST_HEAD(&lifo_stack);
    
    // Allocate device numbers with 0 for dynamic allocation
    ret = alloc_chrdev_region(&dev_number, 0, 2, "lifo");
    if (ret < 0) {
        printk(KERN_ERR "New LIFO Driver: failed to allocate device numbers\n");
        return ret;
    }
    
    // Create device class
    lifo_class = class_create(CLASS_NAME);
    if (IS_ERR(lifo_class)) {
        printk(KERN_ERR "New LIFO Driver: failed to create device class\n");
        unregister_chrdev_region(dev_number, 2);
        return PTR_ERR(lifo_class);
    }
    
    // Initialize and register read device
    cdev_init(&lifo_read_cdev, &lifo_read_fops);
    ret = cdev_add(&lifo_read_cdev, dev_number, 1);
    if (ret < 0) {
        printk(KERN_ERR "New LIFO Driver: failed to add read device\n");
        class_destroy(lifo_class);
        unregister_chrdev_region(dev_number, 2);
        return ret;
    }
    
    // Create read device node
    lifo_read_device = device_create(lifo_class, NULL, dev_number, NULL, DEVICE_NAME_READ);
    if (IS_ERR(lifo_read_device)) {
        printk(KERN_ERR "New LIFO Driver: failed to create read device node\n");
        cdev_del(&lifo_read_cdev);
        class_destroy(lifo_class);
        unregister_chrdev_region(dev_number, 2);
        return PTR_ERR(lifo_read_device);
    }
    
    // Initialize and register write device
    cdev_init(&lifo_write_cdev, &lifo_write_fops);
    ret = cdev_add(&lifo_write_cdev, MKDEV(MAJOR(dev_number), 1), 1);
    if (ret < 0) {
        printk(KERN_ERR "New LIFO Driver: failed to add write device\n");
        device_destroy(lifo_class, dev_number);
        cdev_del(&lifo_read_cdev);
        class_destroy(lifo_class);
        unregister_chrdev_region(dev_number, 2);
        return ret;
    }
    
    // Create write device node
    lifo_write_device = device_create(lifo_class, NULL, MKDEV(MAJOR(dev_number), 1), NULL, DEVICE_NAME_WRITE);
    if (IS_ERR(lifo_write_device)) {
        printk(KERN_ERR "New LIFO Driver: failed to create write device node\n");
        cdev_del(&lifo_write_cdev);
        device_destroy(lifo_class, dev_number);
        cdev_del(&lifo_read_cdev);
        class_destroy(lifo_class);
        unregister_chrdev_region(dev_number, 2);
        return PTR_ERR(lifo_write_device);
    }
    
    printk(KERN_INFO "New LIFO Driver: initialized successfully with major number %d\n", MAJOR(dev_number));
    printk(KERN_INFO "New LIFO Driver: created devices /dev/%s and /dev/%s\n", DEVICE_NAME_READ, DEVICE_NAME_WRITE);
    
    return 0;
}

module_init(lifo_init);
module_exit(lifo_exit);