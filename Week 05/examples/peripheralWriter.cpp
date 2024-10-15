#include <linux/cdev.h>     /* Char device handling */
#include <linux/errno.h>    /* Standard error codes */
#include <linux/fs.h> 	    /* File system structures for drivers */
#include <linux/init.h>     /* Macros for module init and exit */
#include <linux/kernel.h>   /* Kernel-specific functions like printk() */
#include <linux/module.h>   /* Macros for modules, like version info */
#include <linux/uaccess.h>  /* For moving data between user and kernel space */
#include "peripheralWriter.h"  /* Custom header file (not shown here) */

MODULE_DESCRIPTION("Peripheral Writer Linux driver"); // Human-readable description of the module
MODULE_LICENSE("GPL"); // Indicates the license for the code, GNU Public License (GPL)

#define NO_CHANNELS 4  // The peripheral has 4 channels
#define BUF_LEN 512    // Each channel has a buffer size of 512 bytes

// Function declarations for registering/unregistering the device
int register_device(void);
void unregister_device(void);

// Function prototypes for handling file operations like read, write, open, close, and ioctl
static ssize_t peripheral_writer_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
static ssize_t peripheral_writer_write(struct file *filp, const char __user *buf, size_t len, loff_t *off);
static int peripheral_writer_open(struct inode *inode, struct file *file);
static int peripheral_writer_close(struct inode *inode, struct file *file);
static long peripheral_writer_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

/*===============================================================================================*/
/* 
 * This function is executed when the module is loaded. It registers the device with the kernel.
 */
static int peripheral_writer_init(void)
{
    int result = 0;
    printk( KERN_NOTICE "Peripheral-Writer: Initialization started\n" );

    // Call the function to register the device with the kernel
    result = register_device();
    return result;  // Return the result (0 for success, negative for failure)
}

/*===============================================================================================*/
/* 
 * This function is executed when the module is unloaded. It unregisters the device from the kernel.
 */
static void peripheral_writer_exit(void)
{
    printk( KERN_NOTICE "Peripheral-Writer: Exiting\n" );
    
    // Call the function to unregister the device from the kernel
    unregister_device();
}

/*===============================================================================================*/
/* 
 * Specifies which functions to call for initialization and cleanup.
 */
module_init(peripheral_writer_init); // Kernel will call peripheral_writer_init() on module load
module_exit(peripheral_writer_exit); // Kernel will call peripheral_writer_exit() on module unload

/*===============================================================================================*/
/* 
 * Structure that defines file operations (open, read, write, ioctl, close).
 * These functions handle system calls from user space to the device.
 */
static struct file_operations simple_driver_fops =
{
    .owner = THIS_MODULE,  // Specifies the module that "owns" this structure
    .read = peripheral_writer_read,  // Read operation (user-space to kernel-space)
    .write = peripheral_writer_write,  // Write operation (kernel-space to user-space)
    .open = peripheral_writer_open,  // Called when a file descriptor to the device is opened
    .release = peripheral_writer_close,  // Called when the file descriptor is closed
    .unlocked_ioctl = peripheral_writer_ioctl  // Handles ioctl system calls
};

/*===============================================================================================*/
/* 
 * Global variables:
 * - device_file_major_number: stores the major number assigned to the device.
 * - device_name: name of the device.
 * - peripheralChannelIndex: tracks which of the 4 channels is active.
 * - peripheralChannel: an array of 4 buffers, each representing a channel.
 * - peripheralInfo: stores information about the peripheral's channels and buffer size.
 */
static int device_file_major_number = 0;
static const char device_name[] = "Peripheral-Writer";
static int peripheralChannelIndex;  // Current channel index
static char peripheralChannel[NO_CHANNELS][BUF_LEN];  // Buffers for the 4 channels
static PERIPHERAL_INFO peripheralInfo;  // Struct holding peripheral info (e.g., num_channels)

/*===============================================================================================*/
/* 
 * Register the device with the kernel when the module is loaded.
 */
int register_device(void)
{
    int result = 0;

    printk( KERN_NOTICE "Peripheral-Writer: register_device() is called.\n" );

    // Register a character device (major number 0 lets the kernel assign one)
    result = register_chrdev(0, device_name, &simple_driver_fops);
    
    if (result < 0)  // If registration fails, log the error and return
    {
        printk( KERN_WARNING "Peripheral-Writer: can't register character device with errorcode = %i\n", result );
        return result;
    }

    // Store the assigned major number in the global variable
    device_file_major_number = result;
    printk( KERN_NOTICE "Peripheral-Writer: registered character device with major number = %i and minor numbers 0...255\n",
            device_file_major_number );

    // Initialize the channel index and peripheral info structure
    peripheralChannelIndex = 0;
    peripheralInfo.num_channels = NO_CHANNELS;  // Set number of channels (4)
    peripheralInfo.size_channel = BUF_LEN;  // Set size of each channel (512 bytes)

    return 0;  // Return success
}

/*===============================================================================================*/
/* 
 * Unregister the device from the kernel when the module is unloaded.
 */
void unregister_device(void)
{
    printk( KERN_NOTICE "Peripheral-Writer: unregister_device() is called\n" );
    
    // If the device has a valid major number, unregister it
    if (device_file_major_number != 0)
    {
        unregister_chrdev(device_file_major_number, device_name);
    }
}

/*===============================================================================================*/
/* 
 * This function is called when the device is opened by a user-space program (e.g., using open()).
 */
static int peripheral_writer_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Peripheral-Writer: open() is called\n");
    return 0;  // Success
}

/*===============================================================================================*/
/* 
 * This function is called when the device is closed by a user-space program (e.g., using close()).
 */
static int peripheral_writer_close(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Peripheral-Writer: close() is called\n");
    return 0;  // Success
}

/*===============================================================================================*/
/* 
 * This function is called when the device is read from by a user-space program (e.g., using read()).
 */
static ssize_t peripheral_writer_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
    // Copy data from the kernel space buffer to user space
    unsigned long ret = copy_to_user(buf, peripheralChannel[peripheralChannelIndex], BUF_LEN);
    
    // Log the result and the data read from the current channel
    printk(KERN_INFO "peripheral_writer_read: ret:%lu peripheralChannel[%d]:%s\n", ret, peripheralChannelIndex, peripheralChannel[peripheralChannelIndex]);
    
    return BUF_LEN;  // Return the number of bytes read
}

/*===============================================================================================*/
/* 
 * This function is called when the device is written to by a user-space program (e.g., using write()).
 */
static ssize_t peripheral_writer_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
    // Copy data from user space to the current channel buffer in kernel space
    unsigned long ret = copy_from_user(peripheralChannel[peripheralChannelIndex], buf, len);
    
    // Log the result and the data written to the current channel
    printk(KERN_INFO "peripheral_writer_write: ret:%lu peripheralChannel[%d]:%s\n", ret, peripheralChannelIndex, peripheralChannel[peripheralChannelIndex]);
    
    // Increment the channel index and loop back to 0 if the index exceeds 3
    ++peripheralChannelIndex;
    if (peripheralChannelIndex > 3) peripheralChannelIndex = 0;
    
    return len;  // Return the number of bytes written
}

/*===============================================================================================*/
/* 
 * This function is called when the ioctl() system call is invoked from user space.
 * It handles various device-specific operations based on the command (cmd) provided.
 */
static long peripheral_writer_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch(cmd) {
        case PERIPHERAL_WRITER_GET_INFO:
            // Copy peripheral info to user space
            copy_to_user((PERIPHERAL_INFO*) arg, &peripheralInfo, sizeof(peripheralInfo));
            printk(KERN_INFO "num_channels = %d size_channel = %d\n", peripheralInfo.num_channels, peripheralInfo.size_channel);
            break;
        case PERIPHERAL_WRITER_GET_CHANNEL_INDEX:
            // Copy the current channel index to user space
            copy_to_user((int*) arg, &peripheralChannelIndex, sizeof(peripheralChannelIndex));
            printk(KERN_INFO "peripheralChannelIndex = %d\n", peripheralChannelIndex);
            break;
        case PERIPHERAL_WRITER_SET_CHANNEL_INDEX:
            // Copy the channel index from user space to the kernel
            copy_from_user(&peripheralChannelIndex, (int*)arg, sizeof(peripheralChannelIndex));
            printk(KERN_INFO "peripheralChannelIndex = %d\n", peripheralChannelIndex);
            break;
        default:
            return -EINVAL;  // Return invalid argument for unknown commands
    }
    
    return 0;  // Success
}
