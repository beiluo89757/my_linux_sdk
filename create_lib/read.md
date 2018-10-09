###1 create static libs(.a)
- Ubuntu 16.04: 
执行命令：
`gcc -c program.c bill.c fred.c`
`ar crv libfoo.a bill.o fred.o`
`ranlib libfoo.a`
`gcc -o program program.o libfoo.a`
`./program`
