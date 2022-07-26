movq %rsp,%rax
movq %rax,%rdi
#%rsi=0xa 10
popq %rax
lea    (%rdi,%rsi,1),%rax #解题关键
movq %rax,%rdi
