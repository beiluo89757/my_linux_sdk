# Basic Information
###1.1 Supported hardware
- R3000 series with new firmware (above 2.0.0)

###1.2 Supported programming languages
- C/C++
- Linux Shell scripts
- Python 2.7.3 (only available for R3000 series)

###1.3 Recommended host operating systems
- Ubuntu 16.04 (change default shell to bash): 
执行命令：
`sudo ln -sf bash /bin/sh`
`ll /bin/sh`
`sudo ln -sf dash /bin/sh`


#config ubuntu

##1 install complier and libs
1.1 sudo apt-get update
2.2 apt-get install -y build-essential make git gcc

##tftp server
###1 install tftp server
1.1 tftp-hpa: client  
`apt-get install tftp-hpa`
1.2 tftpd-hpa:server  
`apt-get install tftpd-hpa`

##2 config tfgp server
`sudo vim /etc/default/tftpd-hpa`	
TFTP_USERNAME="tftp"
TFTP_ADDRESS="0.0.0.0:69"
TFTP_DIRECTORY="tftp根目录" #服务器目录,需要设置权限为777,chomd 777
TFTP_OPTIONS="-l -c -s"

##3 restart tftp server
`sudo service tftpd-hpa restart	`


##compile
- `make /package/iothub_client_sample_mqtt/install VER=1.0.0`
- `make /package/iothub_client_sample_mqtt/clean VER=1.0.0`
- `make /package/iothub_client_sample_mqtt/build VER=1.0.0`

##Transfer config file to board
`tftp -g -r demo 192.168.0.101`

##Set time
`date -s "2018-01-25 16:59"`
`hwclock  -w`