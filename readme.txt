*******************************************************************************
ola2pd 0.02 6/1/2013

(C) 2012-2013 Santi Noreña belfegor<AT>gmail.com

Based on dmxmonitor by Dirk Jagdmann doj<AT>cubic.org
and dmxmonitor_ola by Simon Newton nomis52<AT>gmail.com
 
This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*******************************************************************************

ola2pd is an external for Pure Data that reads one DMX512 universe from the Open Lighting Arquitecture daemon. OLA supports a lot of of lighting protocols (ArtNet, Pathport, ACN, ShowNet, SandNet) and USB-DMX devices (Enttec Open DMX Pro, Velleman, Robe,...) 

Ola2pd has been developed with the flext headers from Thoma Grill. The binary file included has been tested in Ubuntu Precise 12.04 and Debian Wheezy.

*******************************************************************************

Install:

- Install and set up OLA following the project instructions from http://code.google.com/p/linux-lighting

- Copy the file ola2pd.pd_linux in a folder in the Pure Data paths. ~/pd-extenals is a good place, or in the folder of the patch you are using.  

- If the binary doesn't work in you distribution, you can try compiling it yourself. You need having compiled and installed the flext headers http://grrrr.org/research/software/flext/, the Pure Data sources, and the OLA headers. See build.txt in the flext package for details. Basically you must run the flext build tool (build.sh) in the ola2pd folder.

- For Ubuntu there is an aditional compilation step. Ubuntu's gcc has a option making buils.sh doesn't link correctly the OLA library. You can copy the last command of the build.sh terminal output and change the option -lola to the end of the command. For me, i copy:

g++ -L/usr/include/ola -lola  -pthread -shared  -Wl,-S -L/home/santi/PMS/pd-0.43-2/bin -L/usr/lib -o pd-linux/release-multi/ola2pd.pd_linux pd-linux/release-multi/main.opp -lflext-pd_t

and i change to: 

g++ -L/usr/include/ola -pthread -shared  -Wl,-S -L/home/santi/PMS/pd-0.43-2/bin -L/usr/lib -o pd-linux/release-multi/ola2pd.pd_linux pd-linux/release-multi/main.opp -lflext-pd_t -lola

After you can run

strip --strip-unneeded pd-linux/release-multi/ola2pd.pd_linux

to make the binary smaller. After, the binary is ready to work

*******************************************************************************

Usage:

- The ola daemon must be running in the system and set up with at least one input universe.
- olad listens on the network interface with lower IP address, tipically 2.x.x.x. You can see if valid data is arriving pointing one web browser to localhost:9090. Click in the universe you want and click in the "DMX Monitor" tab. You should see the arriving data. 
- Before ola2pd begins output values, we can configure the listening universe with the message [universe x), where x is the number of the OLA universe we want to listen. The object begins listening with [open) message. We can change the listening universe closing the connection with a [close) message, sending the new universe and reopening the node.  
- ola2pd outputs a list with the 512 channel of the universe. We can manipulate with [list split] and [unpack].
- See ola2pd-help.pd 

*******************************************************************************

Credits:

- Simon Newton for OLA and ola_dmxmonitor, this is a modification of that code.

*****************************

SVN:

http://code.google.com/p/puremediaserver/externals/ola2pd

Contact:

belfegor@gmail.com

*****************************
