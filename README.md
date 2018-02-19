# asm-cforth
一个接近asm forth的高效cforth

An efficient cforth close to asm forth

forth是一种堆栈机,使用后缀表达式,冒号是定义函数的符号,以栈为参数传递和传回返回值的通道, 在栈上运算数据

forth is a stacker, using the suffix expression, the colon is the symbol that defines the function, the stack passes and returns the channel for the return value, and the data is evaluated on the stack

1 2 +

显示3 

returns 3

: x	if 22 22 else 33 33 endif ;

1 2 > x

显示 33 33

returns 33 33

	:s

	switch 

		1 case 11 break 
	
		2 case 22 break
	
		3 case 33 break 
	
		default  55 
	
	ends ;

1 s 显示11

2 s 显示22

switch的另一种用法


	switch

		1 2 < if 11 break
  
		1 2 == if 22 break
  
	ends


其他情况请详看system.f里的示例
PS:system.f里只能编译,不能解释

Other cases, please look at the example in system.f
PS: system.f can only be compiled, can not explain

