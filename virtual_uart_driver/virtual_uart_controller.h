#ifndef VIRTUAL_UART_CONTROLLER_H_
#define VIRTUAL_UART_CONTROLLER_H_

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/cdev.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/mdio.h>
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/kdev_t.h>
#include <linux/interrupt.h>
#include <linux/syscalls.h>
#include <linux/mount.h>
#include <linux/device.h>
#include <linux/genhd.h>
#include <linux/namei.h>
#include <linux/shmem_fs.h>
#include <linux/ramfs.h>
#include <linux/sched.h>
#include <linux/vfs.h>
#include <linux/pagemap.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/dcache.h>
#include <linux/spi/spi.h>
#include <linux/i2c.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/kfifo.h>
#include <linux/serial_reg.h>
#include <linux/serial_core.h>

#define MAX_VIRTUAL_UART 6
#define VIRTUAL_XMIT_SIZE   512
#define VIRTUAL_UART_NAME "vttyU"

#define VIRTUAL_UART_MIN_SPEED 57600
#define VIRTUAL_UART_MAX_SPEED 1152000
#define VIRTUAL_UART_TYPE "virtual_uart"
#define PORT_JERRY  128

/*platform传递资源参数的数据结构*/
struct virtual_uart_port_platform_config
{
    int port_index;
};

/*虚拟串口相关的数据结构，内部包含uart_port类型变量，并包括rx_enable_flag、tx_enable_flag，用于表示
读写使能，而工作队列则用于模拟中断处理函数，完成数据发送操作*/
struct virtual_uart_port {
    struct uart_port port;
    bool tx_enable_flag;
    bool rx_enable_flag;
    struct work_struct work;
    spinlock_t write_lock;
    char uart_type[24];
};

#endif
