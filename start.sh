#!/bin/bash

#cd code/nginx-1.17.1/
#make install
#cd /home/obd
#cd Tinyhttpd
#make install IPATH=/home/obd/tools/Tinyhttpd
#nginx -c /home/obd/tools/nginx/conf/nginx.conf
#kill -s 9 `ps -aux | grep httpd | awk '{print $2}'`
#kill -s 9 `ps -aux | grep 10004 | awk '{print $2}'`
#kill -s 9 `ps -aux | grep 10005 | awk '{print $2}'`
#kill -s 9 `ps -aux | grep 10006 | awk '{print $2}'`
#kill -s 9 `ps -aux | grep 10007 | awk '{print $2}'`
#kill -s 9 `ps -aux | grep 10008 | awk '{print $2}'`
./build/httpd -p 10004 -d
./build/httpd -p 10005 -d
./build/httpd -p 10006 -d
./build/httpd -p 10007 -d
./build/httpd -p 10008 -d
#httpd -p 10009 -d
#httpd -p 10010 -d
#httpd -p 10011 -d
#httpd -p 10012 -d
#httpd -p 10013 -d



