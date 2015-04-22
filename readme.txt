**************************************************
********** Hacking construction kit **************
**************************************************

version 2.0

The goal of this library is to help you to make worms, bots, or hacking tools.
It was written for educational and recreational purposes only.
You're free to use this library in your softwares.
Most of the code come from Metasploit, agobot, caberp... or internet.
Most of the code is badly coded and not optimize at all, but it's only the first version.

Here is some examples of what you can do with this library:

* SSL/TLS connection (client and server)
* ms08-067 exploit
* OS fingerprinting
* a keylogger
* inject DLLs in others processes
* take snapshots of webcam or screen
* search for files on hard drives
* zip files in archive
* send files to server with HTTP POST or FTP
* a crypter
* code/decode with RC4, RSA, base 64 or more simpler encoder
* make a command & controll client
* send files other a remote shell (nc -L 127.0.0.1 -p xxx -e cmd.exe)
* create random IPs, ping IPs, find TCP open port
* install and remove services
* get infos on OS
* manage processes
* encode/decode strings in .exe files
* a simple motion detection with webcam
* record sound

Installation:
compile with /MTd or /MT (no DLLs)

Pre-compilation:
You must have Python 3 in your path, it'll be used to encode strings in .cpp

Compilation:
Add to include directory "hacking construction kit"

Link:
Add "hacking construction kit".lib
If needed, add openssl, zlib, or ntdll
See generic_test as example for compilation and link options

Project template
If you add your project to the solution, you should use properties_template_debug and _release as template for your project.
It'll add the correct options for compile and link.

FAQ



