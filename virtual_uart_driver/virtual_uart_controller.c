#include "virtual_uart_controller.h"

#define VIRTUAL_TTY_DEBUG
#ifdef VIRTUAL_TTY_DEBUG
#define virtual_tty_debug(fmt, argv...) \
    do \
    {\
        printk(fmt,##argv); \
    }while(0)
#else
#define virtual_tty_debug(fmt, argv...)
#endif
static struct uart_driver virtual_uart_driver;


static void virtual_uart_flush_to_port(struct work_struct *work)
{
    struct virtual_uart_port *uart_port = container_of(work, struct virtual_uart_port, work);
    int count = 0;

    struct circ_buf *xmit = &uart_port->port.state->xmit;

    if (uart_port->port.x_char)
    {
        uart_port->port.icount.tx++;
        uart_port->port.x_char = 0;
        return;
    }

    if (uart_circ_empty(xmit) || uart_tx_stopped(&uart_port->port)) 
    {
        uart_port->tx_enable_flag = false;
        return;
    }
    
    count = uart_port->port.fifosize;
    do
    {
        printk("0x%hhx ", xmit->buf[xmit->tail]);
        
        xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
        uart_port->port.icount.tx++;
        if (uart_circ_empty(xmit))
            break;
    }while (--count > 0);
    printk("\n");

    if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
    {
        uart_write_wakeup(&uart_port->port);
    }

    printk("%s:%d\n", __FUNCTION__, __LINE__);

    if(uart_circ_empty(xmit))
        uart_port->tx_enable_flag = false;

    
    printk("%s:%d\n", __FUNCTION__, __LINE__);
}


static unsigned int virtual_uart_tx_empty(struct uart_port *port)
{
    struct virtual_uart_port *uart_port = container_of(port, struct virtual_uart_port, port);
    struct circ_buf *xmit = &uart_port->port.state->xmit;

    /*在正常的驱动开发中，此处应为串口控制器的fifo是否为空*/
    return uart_circ_empty(xmit);
}

static void virtual_uart_stop_tx(struct uart_port *port)
{
    
    struct virtual_uart_port *uart_port = container_of(port, struct virtual_uart_port, port);

    uart_port->tx_enable_flag = false;
}

static void virtual_uart_start_tx(struct uart_port *port)
{
    struct virtual_uart_port *uart_port = container_of(port, struct virtual_uart_port, port);
    unsigned long flags = 0;


    spin_lock_irqsave(&uart_port->write_lock, flags);

    uart_port->tx_enable_flag = true;
    spin_unlock_irqrestore(&uart_port->write_lock, flags);
    schedule_work(&uart_port->work);
    
    printk("%s:%d\n", __FUNCTION__, __LINE__);
}
static void virtual_uart_stop_rx(struct uart_port *port)
{
    struct virtual_uart_port *uart_port = container_of(port, struct virtual_uart_port, port);
    unsigned long flags = 0;

    spin_lock_irqsave(&uart_port->write_lock, flags);
    uart_port->rx_enable_flag = false;
    spin_unlock_irqrestore(&uart_port->write_lock, flags);
}
static void virtual_uart_throttle(struct uart_port *port)
{

}
static void virtual_uart_unthrottle(struct uart_port *port)
{


}
static void virtual_uart_enable_ms(struct uart_port *port)
{


}
static void virtual_uart_break_ctl(struct uart_port *port, int break_state)
{

}

static int virtual_uart_startup(struct uart_port *port)
{
    struct virtual_uart_port *uart_port = container_of(port, struct virtual_uart_port, port);
    unsigned long flags = 0;

    spin_lock_irqsave(&uart_port->write_lock, flags);
    uart_port->rx_enable_flag = true;
    uart_port->tx_enable_flag = true;
    spin_unlock_irqrestore(&uart_port->write_lock, flags);

    return 0;
}

static void virtual_uart_shutdown(struct uart_port *port)
{
    printk("%s:%d \n", __FUNCTION__, __LINE__);
}
static void virtual_uart_set_termios(struct uart_port *port, struct ktermios *termios,
            struct ktermios *old)
{
    unsigned char cval = 0;
    unsigned int baud = 0;

    switch (termios->c_cflag & CSIZE)
    {
        case CS5:
            cval = UART_LCR_WLEN5;
            break;
        case CS6:
            cval = UART_LCR_WLEN6;
            break;
        case CS7:
            cval = UART_LCR_WLEN7;
            break;
        default:
        case CS8:
            cval = UART_LCR_WLEN8;
            break;
    }

    if (termios->c_cflag & CSTOPB)
        cval |= UART_LCR_STOP;
    if (termios->c_cflag & PARENB)
        cval |= UART_LCR_PARITY;
    if (!(termios->c_cflag & PARODD))
        cval |= UART_LCR_EPAR;



    baud = uart_get_baud_rate(port, termios, old, VIRTUAL_UART_MIN_SPEED, VIRTUAL_UART_MAX_SPEED);
    printk("baud = %d cval = %d\n", baud, cval);
}
static const char * virtual_uart_type(struct uart_port *port)
{
    struct virtual_uart_port *uart_port = container_of(port, struct virtual_uart_port, port);

    return uart_port->uart_type;
}
static void virtual_uart_release_port(struct uart_port *port)
{
    printk("%s:%d\n", __FUNCTION__, __LINE__);
}
static void virtual_uart_set_mctrl(struct uart_port *uart_port, unsigned int mctrl)
{
    printk("%s:%d\n", __FUNCTION__, __LINE__);
}
static unsigned int virtual_uart_get_mctrl(struct uart_port *uart_port)
{

    printk("%s:%d\n", __FUNCTION__, __LINE__);

    return 0;
}


static struct uart_ops virtual_uart_ops = {
    .tx_empty	= virtual_uart_tx_empty,
    .stop_tx	= virtual_uart_stop_tx,
    .start_tx	= virtual_uart_start_tx,
    .throttle	= virtual_uart_throttle,
    .unthrottle	= virtual_uart_unthrottle,
    .stop_rx	= virtual_uart_stop_rx,
    .enable_ms	= virtual_uart_enable_ms,
    .break_ctl	= virtual_uart_break_ctl,
    .startup	= virtual_uart_startup,
    .shutdown	= virtual_uart_shutdown,
    .set_termios	= virtual_uart_set_termios,
    .type   = virtual_uart_type,
    .release_port = virtual_uart_release_port,
    .set_mctrl = virtual_uart_set_mctrl,
    .get_mctrl = virtual_uart_get_mctrl
};



/*

S_IRUSR | S_IWUSR

*/
static ssize_t vuart_receive_buff_store(struct device *dev,
                    struct device_attribute *attr,
                    const char *buf, size_t count)
{
    struct tty_port *tty_port = dev_get_drvdata(dev);
    struct uart_state *uart_state = container_of(tty_port, struct uart_state, port);
    struct uart_port *uart_port = uart_state->uart_port;
    struct virtual_uart_port *virtual_uart_port = container_of(uart_port, struct virtual_uart_port, port);
    int i = 0;

    if(count <= 0 || virtual_uart_port == NULL)
        return -EINVAL;

    /*因为此处我们是借助注册至sysfs下的属性文件模拟接收，因此当对属性文件uart_receive_buff进行写操作时
    即会进入该函数的处理中，但此时可能应用程序尚未存在打开该虚拟串口，因此此处判断rx_enable_flag是否为true
    （在执行startup函数时，会同步打开该标签）。即使此处不判断，在tty_buffer将数据刷到线路规程时，也会返回失败
    ，因此tty_port未与tty_ldisc关联*/
    if(!virtual_uart_port->rx_enable_flag)
    {
        return -EACCES;
    }

    for(i = 0; i < count; i++)
    {
        tty_insert_flip_char(tty_port, buf[i], TTY_NORMAL);
    }

    tty_flip_buffer_push(tty_port);

    return count;
}

static DEVICE_ATTR(uart_receive_buff, S_IWUSR, NULL, vuart_receive_buff_store);


static struct attribute *virtual_uart_attrs[] =
{
    &dev_attr_uart_receive_buff.attr,
    NULL
};
static const struct attribute_group virtual_uart_attr_group = 
{
    .attrs = virtual_uart_attrs,
};


static int virtual_uart_port_platform_probe(struct platform_device *platform_dev)
{
    struct virtual_uart_port *port;
    struct virtual_uart_port_platform_config *port_config_datap = NULL;
    int ret;

    port_config_datap = (struct virtual_uart_port_platform_config *)platform_dev->dev.platform_data;
    port = devm_kzalloc(&platform_dev->dev, sizeof(struct virtual_uart_port), GFP_KERNEL);
    if (!port)
        return -ENOMEM;

    
    printk("virtual tty port=%d register\n", port_config_datap->port_index);
    port->port.ops = &virtual_uart_ops;
    port->port.dev = &platform_dev->dev;
    port->port.fifosize = 512;//串口控制器的fifo size
    port->port.type = PORT_JERRY;
    port->port.line = port_config_datap->port_index;
    port->port.attr_group = &virtual_uart_attr_group;
    
    sprintf(port->uart_type, "%s%d", VIRTUAL_UART_TYPE, port->port.line);
    spin_lock_init(&port->write_lock);
    INIT_WORK(&port->work, virtual_uart_flush_to_port);

    ret = uart_add_one_port(&virtual_uart_driver, &port->port);
    if (ret != 0)
    {
        goto exit;
    }

    if(ret == 0)
    {
        platform_set_drvdata(platform_dev, port);
    }
    
    printk("%s:%d port ptr=%p %p\n", __FUNCTION__, __LINE__, port, &port->port);
exit:
    return ret;
} 

static int virtual_uart_port_platform_remove(struct platform_device *platform_dev)
{
    struct virtual_uart_port *port = platform_get_drvdata(platform_dev);

    if(port)
    {
        uart_remove_one_port(&virtual_uart_driver, &port->port);
    }

    platform_set_drvdata(platform_dev, NULL);

    return 0;
}
#if defined(CONFIG_OF)
static const struct of_device_id virtual_uart_of_match[] = {
    { .compatible = "jerry_chg,virtual-uart" },
    {},
};
MODULE_DEVICE_TABLE(of, virtual_uart_of_match);
#endif


static struct platform_driver virtual_uart_port_platform_driver = {
    .driver = {
        .name = "virtual_uart_port_dev",
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(virtual_uart_of_match),
    },
    .probe = virtual_uart_port_platform_probe,
    .remove = virtual_uart_port_platform_remove,
};

static struct uart_driver virtual_uart_driver = {
    .owner		= THIS_MODULE,
    .driver_name	= "virtual-uart",
    .dev_name	= VIRTUAL_UART_NAME,
    .nr		= MAX_VIRTUAL_UART,
    .cons		= NULL,//not support
};

static int __init virtual_uart_init(void)
{
    int ret = 0;

    ret = uart_register_driver(&virtual_uart_driver);
    if (ret != 0)
        return ret;


    ret = platform_driver_register(&virtual_uart_port_platform_driver);
    if(ret != 0)
        uart_unregister_driver(&virtual_uart_driver);

    return ret;
}

static void __exit virtual_uart_exit(void)
{
    platform_driver_unregister(&virtual_uart_port_platform_driver);
    uart_unregister_driver(&virtual_uart_driver);
}
module_init(virtual_uart_init);
module_exit(virtual_uart_exit);

MODULE_DESCRIPTION("Virtual uart Drivers");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("jerry_chg");
