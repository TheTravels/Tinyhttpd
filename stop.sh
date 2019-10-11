#!/bin/bash

#cd code/nginx-1.17.1/
#make install
#cd /home/obd
#cd Tinyhttpd
#make install IPATH=/home/obd/tools/Tinyhttpd
#HOME_PATH=/home/$(whoami)
#NGINX_PATH=$HOME_PATH/tools/nginx
#HTTPD_PATH=$HOME_PATH/tools/Tinyhttpd
#echo "home path:   " $HOME_PATH
#echo "nginx_path:  " $NGINX_PATH
#echo "server path: " $HTTPD_PATH
kill -s 9 `ps -aux | grep nginx | awk '{print $2}'`
kill -s 9 `ps -aux | grep httpd | awk '{print $2}'`
#nginx -c /home/obd/tools/nginx/conf/nginx.conf
#cd $NGINX_PATH
#nginx -c ./conf/nginx.conf
#cd $HTTPD_PATH
#cp $HTTPD_PATH/httpd /home/public/server/httpd
#httpd -p 10004 -d -N -D /home/public/server/
#httpd -p 10005 -d -N -D /home/public/server/
#httpd -p 10006 -d -N -D /home/public/server/
#httpd -p 10007 -d -N -D /home/public/server/
#httpd -p 10008 -d -N -D /home/public/server/
#httpd -p 10009 -d -N
#httpd -p 10010 -d -N
#httpd -p 10011 -d -N
#httpd -p 10012 -d -N
#httpd -p 10013 -d -N

ps -aux | grep nginx
ps -aux | grep httpd

