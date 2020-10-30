###### 1	Install RPI OS
See RPI website for info
** add a blank text file to root dir called ssh - this will enable you to access via ssh https://www.raspberrypi.org/documentation/remote-access/ssh/README.md
** for wifi https://www.raspberrypi.org/documentation/configuration/wireless/headless.md
###### 2 Terminal	
`sudo raspi-config`
- Option 5 - Interfacing Options and then enable SSH, VNC, Serial etc.
- Option 7 Advanced Options, and then select Option A1 – expand file system
- Select Finish and reboot when prompted
###### 3 Terminal

```
sudo apt update && sudo apt dist-upgrade -y && sudo apt autoremove -y
sudo reboot
```

###### 4 Terminal

```	
sudo -i
apt-get install -y software-properties-common apparmor-utils apt-transport-https avahi-daemon ca-certificates curl dbus jq network-manager socat
systemctl disable ModemManager
systemctl stop ModemManager
curl -fsSL get.docker.com | sh
curl -sL "https://raw.githubusercontent.com/Kanga-Who/home-assistant/master/supervised-installer.sh" | bash -s -- -m raspberrypi4
```

*** if using a rpi3 replace 4 with 3
###### 5 Browser	
http://192.168.x.x:8123   - use the ip of your rpi
** chaneg to your RPI ip + can take up to 20 mins but once installed signup/login
###### 6 Browser	
***Once logged in and set up
- Open 'Supervisor' in side bar
- Add-on store
- Install AppDaemon 4
- Turn on Start on boot in side bar
- Start AppDaemon
- Install File Editor
- Turn on start on boot and show in side bar
- Start File Editor
	
Developed with the help off	
https://community.home-assistant.io/t/installing-home-assistant-supervised-on-raspberry-pi-os/201836	