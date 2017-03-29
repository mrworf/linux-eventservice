EventService

This tool was created to find a quick and easy solution to remotely control a
Raspberry Pi 3 running Plex Media Player. Unlike most other solutions used by
multiRemote, it's UDP based. This makes the tool much smaller and portable and
less likely to slow down the device it's running on.

Known issues

This tool has absolutely NO SECURITY and is NOT meant to run on an open network
connection such as the internet. It will accept any input which is 6 bytes long
and starting with 0xDEADBEEF and ending with a uint16 containing the actual KEY
code to issue.

A future improvement would be using MD5 or SHA1 and a seed to hash the content
to avoid spoofing. This would only delay the inevitable, so please, don't use
this over the internet :)

Also, the tool has to run as root since it injects events into the linux input
subsystem. Nasty, but 100% compatible with all possible devices and apps.

How to build

Use you favourite GCC flavor and ask it to compile it. For example,

```
gcc main.c -o ev
```

or for ARM when cross compiling

```
arm-linux-gnueabihf-gcc main.c -o ev_arm
```

Where do I find the codes?

http://lxr.free-electrons.com/source/include/uapi/linux/input-event-codes.h is kinda convenient
