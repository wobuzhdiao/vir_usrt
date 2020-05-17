# virt_uart

#### 介绍
模拟一个uart driver，支持多个uart port。涉及的知识点包括：
1. uart子系统uart driver、uart port的创建；
2. sysfs属性文件的创建，用于模拟外部向虚拟串口发送数据

#### 软件架构
软件架构说明
	本次开发的虚拟串口提供的功能如下：
	1. 提供两个串口实例
	2. 串口名称的前缀为vttyU
	3. 为了验证串口收发，提供了loopback机制，即应用程序向虚拟串口写入数据后，数据再回环至应用程序；
	4. 在/sys目录下提供数据写入属性文件，可向虚拟串口中写入数据，用以模拟串口接收数据的功能
	本次开发的代码涉及的模块包括：
	1. 创建两个platform device，分别对应两个虚拟串口的platform device；
	2. 创建一个platform driver，在platform driver的probe接口中，完成虚拟串口的注册，主要是完成uart_add_one_port
           接口完成虚拟串口的注册；在platform driver的remove接口中，完成虚拟串口的注销；
	3. 在串口驱动的初始化函数中，调用uart_register_driver，uart driver的注册。

数据结构说明

在虚拟串口驱动中，定义了数据结构virtual_uart_port，该数据结构中包含了uart_port。
并定义了tx_enable_flag、rx_enable_flag，分别用于控制串口收发，因是虚拟串口所以使用这两个变量进行表示，若是真是
的串口控制器，则不需要这两个变量，而只需要在uart_ops->startup、uart_ops->stop_rx、uart_ops->stop_tx中关闭中断即可，
这两个变量由自旋锁write_lock进行保护。
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

#### 安装教程

1.  make
2.  make install
3.  在./images目录下即为生成的驱动及测试程序

#### 使用说明

1.  insmod virtual_uart_platform_dev.ko
2.  insmod virtual_uart_controller.ko
    当执行以上两步后，即生成了串口字符设备文件，名称为/dev/vttyU0 、/dev/vttyU1,然后就可以以访问串口的方式，访问这两个设备；
    另外若要模拟虚拟串口从外部接收的数据，返回应用程序，可向文件/sys/class/tty/vttyU0/uart_receive_buff 
    /sys/class/tty/vttyU1/uart_receive_buff中写入数据即可。

#### 参与贡献



