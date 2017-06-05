set disassembly-flavor intel
# set pagination off
layout split

file /mnt/hgfs/fedora_24_x86_64/linux-stable/vmlinux
target remote 192.168.0.197:8086

break do_init_module
commands
set $base = mod->core_layout.base
set $init = mod->init
p $base
p $init
add-symbol-file /mnt/hgfs/fedora_24_x86_64/current-time/current_time.ko $base
# b *$base
# b *$init
b *current_time_loop
b *current_time_init
b *current_time_exit
cont
end

# break do_one_initcall
# command
# p fn
# end

cont
