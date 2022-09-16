# training-ns3-samples
 NS-3 (network simulator - 3) sample examples for session for students

Preliminary: basic understand of computer network, wireless communication protocols is required. Make sure you have latest version of NS-3 allinone package is installed into Ubuntu PC/Laptop.

clone the sample programs of NS-3. 
Then past it into scratch folder of NS-3 recent version.


HOW TO INSTALL NS3 IN YOUR LAPTOPS
NS3 version 3.33 needs UBUNTU 20.04 platform. So, without disturbing the functioning of your laptop, simply you can install it on virtual machine creating on windows itself. Go through the instruction POINT 1 for it. 

    1. The complete steps are explained in the link below:
https://youtu.be/x5MhydijWmc
After installing UBUNTU, please follow POINT 2 for ns3 installation.

    2. Please find here the details for NS-3 Installations.
NS3_Installation_steps
sudo apt install build-essential libsqlite3-dev libboost-all-dev libssl-dev git python3-setuptools castxml
sudo apt install gir1.2-goocanvas-2.0 gir1.2-gtk-3.0 libgirepository1.0-dev python3-dev python3-gi python3-gi-cairo python3-pip python3-pygraphviz python3-pygccxml

sudo apt install g++ pkg-config sqlite3 qt5-default mercurial ipython3 openmpi-bin openmpi-common openmpi-doc libopenmpi-dev autoconf cvs bzr unrar gdb valgrind uncrustify doxygen graphviz imagemagick python3-sphinx dia tcpdump libxml2 libxml2-dev cmake libc6-dev libc6-dev-i386 libclang-6.0-dev llvm-6.0-dev automake

Install Additional Python Packages
sudo su 
cd
pip3 install kiwi
exit
cd

Download and Install NS-3 and NetAnim
# download from ns-3 server
wget -c https://www.nsnam.org/releases/ns-allinone-3.33.tar.bz2
# extract tar.bz2
tar -xvjf ns-allinone-3.33.tar.bz2
# go back to home folder
cd
# navigate to ns-3 directory (not the NS-3 all in one)
cd ns-allinone-3.33/ns-3.33/
# Configure the installation
./waf configure --enable-examples 
# Build ns-3 installation
./waf
# to check whether installation was a success
./waf --run hello-simulator
# navigate to netanim dir.
cd ns-allinone-3.33/netanim-3.108/
# configure the build
make clean
# compile the build
qmake NetAnim.pro
# build netanim installation
make
# to execute NetAnim 
./NetAnim
