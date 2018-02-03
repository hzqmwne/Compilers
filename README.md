### SJTU&emsp;SE302&emsp;编译原理与技术(Principle and Technology of Compiler)   
##### 《Modern Compiler Implementation in C》中的Tiger编译器  
&emsp;  
lab1：Straight-line program interpreter（直线式程序解释器）  
lab2：Lexical Analysis（词法分析）  
lab3：Syntax Analysis（语法分析）  
final_tiger：Final Test for lab2 and lab3（对lab2和lab3的深度测试）  
lab4：Type Checking（类型检查）  
lab5：Intermediate Code（中间代码生成）  
lab6：A Workable Tiger Compiler（可用的tiger编译器）  
&emsp;  
#### Usage:  
编译tiger编译器：（需要linux系统并安装有gcc、make、lex、yacc）  
cd lab6  
make  
得到tiger-compiler  
  
编译tiger程序：  
./tiger-compiler hello.tig  
得到hello.tig.s  
  
链接：（对于64位系统，需要安装32位运行库）  
gcc -Wl,--wrap,getchar -m32 hello.tig.s runtime.c -o hello.out  
（如果是32位系统，去掉-m32参数）  
  
运行：  
./hello.out  
&emsp;  
&emsp;  
  
更多信息可参考note.txt  
