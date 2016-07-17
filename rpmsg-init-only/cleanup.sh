#! /bin/bash
echo "4a338000.pru1" > /sys/bus/platform/drivers/pru-rproc/unbind 2> /dev/null
echo /dev/null > /var/log/messages
echo /dev/null > /var/log/syslog
echo /dev/null > /var/kern.log
