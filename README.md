A sample Linux kernel module that saves the current time to `/tmp/current_time`
every minute in the hh:mm format, e.g., 11:58 or 22:07.

# Debugging with VMware Fusion

The following is partly based on this post, which also describes how to
configure lldb:
http://ddeville.me/2015/08/using-the-vmware-fusion-gdb-stub-for-kernel-debugging-with-lldb

I was able to attach with lldb as described in the post, but decided to
configure gdb instead.  lldb seems to be poorly suited for debugging the
Linux kernel.

Two Linux VMs are required: a debugger and a debuggee.  gdbserver won't work with
gdb configured as something like "x86_64-apple-darwin16.6.0", even if it's
built for multiple targets (the `file` command in gdb will work on vmlinux, but
debugging won't be possible).  `gdb --version` should return, e.g.,
"x86_64-redhat-linux-gnu".

Quit VMware and add these lines to the debuggee's .vmx file, e.g.,
`~/Documents/Virtual\ Machines.localized/Fedora\ 64-bit.vmwarevm/Fedora\ 64-bit.vmx`:
```
debugStub.listen.guest64 = "TRUE"                          # for 64-bit guests
debugStub.listen.guest64.remote = "TRUE"                   # allows remote connection
debugStub.port.guest64 = "8086"                            # port to listen on
debugStub.listen.guest32 = "TRUE"                          # same for 32-bit guests
debugStub.listen.guest32.remote = "TRUE"
debugStub.port.guest32 = "8032"
# debugStub.hideBreakpoints= "TRUE"                        # for hardware breakpoints
```

There might be an error when starting the debuggee VM.  If so, stop all
VMware-related services and try again, or just reboot.

The debugger VM will connect to the host on port 8086, so set the Network Adapter
option to NAT or similar.  The debuggee can be connected to the Host-only network.

Build the kernel with debugging options, for example (there's more):
```
CONFIG_DEBUG_INFO=y
CONFIG_READABLE_ASM=y
```

Modify the module:
* add  `__attribute__((optimize("O0")))` to the relevant module functions
* remove `__init` and `__exit` for setting breakpoints on the module entry
  and cleanup functions.

Start debugging:
```
debuggee$ make
debugger$ gdb -q -x cmd.gdb
debuggee1$ dmesg --follow
debuggee2$ sudo insmod current_time.ko
```
