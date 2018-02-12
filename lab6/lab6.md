<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html>
<head>
</head>

<body onload="on_resize()" onclick="on_resize()">
<div id="header">
<span class="MainTitle">Compilers</span>
<span class="SubTitle">2017</span>
</div>

<div id="content">
	<h2>Lab6: A Workable Tiger Compiler</h2>
	<p>
	<b>Due:</b> Dec 31, 12:00, 2017</br> 
	<p>Try your best and do not miss the deadline! No cheat otherwise all the related students will fail this course!</p> 
	</p>
<hr>

<h2>Introduction</h2>
<p>
Lab 6 is the final part of the whole project. In part A you need to implement the instruction selection phase to generate x86 machine codes with infinite registers for tiger programs. <br/>Please read <b>chapter 8 & 9</b> of your textbook carefully before you start.
</p>

<h2>PartA: Instruction Selection</h2>
<h2>Getting started</h2>
<p>
Download the orginal environment <a href="https://ipads.se.sjtu.edu.cn/courses/compilers/handout-2017/lab6.tar.gz">here</a>.
<pre>
$ tar -xf lab6.tar.gz
$ cd lab6/
</pre>

For this final project, we have implemented a Main module for a phase-by-phase compilation. Please read code in main.c carefully.

<pre>
(main.c, line 45)
F_tempMap = Temp_empty();
</pre>

The variable <b>F_tempMap</b> is a global mapping between temporiaies and registers, which will be helpful for output and later phases (like register allocation). You can define it in any file you want but do not forget to define it. <br/>

In addition, there are other files you need to care about: </br></br>

<b>parse.c</b>: We have removed the main function in parse.c. Be careful when merging with your previous code (if you have modified parse.c).</br></br>

<b>canon.[ch]</b>: The Canonicalization module (described in Chapter 8). We have already provide the implementation. You might want to see what actually happened in those files.</br></br>

<b>codegen.[ch]</b>: You need to implement the code generator by completing the function <b>F_codegen</b> in <b>codegen.c</b>. </br></br>

<b>assem.[ch]</b>: It contains data structures for an instruction and basic logic for output. </br></br>

Other files will reuse your code in previous labs. Please feel free to modify them if needed.
<pre>
$ cd lab6
$ ./copy_lab5.sh path_to_your_lab5 (default path is ../lab5)
</pre>
This script will copy tiger.[lex,y], frame.h, x86frame.c, semant.[c,h] and translate.[c,h] into current directory. Please copy other modified files if necessary.

<b>Hint:</b></br>
Please note that integer division is a little weird in x86 architechture. This might be helpful: <a href="http://x86.renejeschke.de/html/file_module_x86_id_137.html">http://x86.renejeschke.de/html/file_module_x86_id_137.html</a>

</p>


<h2>PartB: Register Allocation and Make the Final Compiler</h2>
<p>
In part B you need to implement liveness anlaysis and register allocation, and run compiled tiger programs on real machine! <br/>Please read <b>chapter 10, 11, 12</b> of your textbook carefully before you start. <br/><br/>

<b style="color: red">WARNING!</b> We recommend working on Linux. If you use other operating systems, you need to tackle with problems like linking on your own and test your code in the provided virtual machine. We will only evaluate your project in Linux environment.
</p>

Now look around again, here are some files you need to care about: </br></br>

<b>main.c</b>: It has been modified to invoke the liveness analyzer and the register allocator. </br></br>

<b>graph.[ch]</b>: The data structures for a graph. </br></br>

<b>flowgraph.[ch]</b>: You need to modify them to construct flow graphs for instructions. </br></br>

<b>liveness.[ch]</b>: You need to modify them to implement the Liveness module.</br></br>

<b>color.[ch]</b>: You need to modify them to handle graph coloring. </br></br>

<b>regalloc.[ch]</b>: You need to modify them for registor allocation. </br></br>

<b>escape.[ch]</b>: You need to implement the Escape module if you want to get full score. </br></br>

<b>runtime.c</b>: For convenience, we have added a new external function, <b>printi</b>, to print out integers. Please add it to your global environment. </br></br>

</br><b>Hint:</b></br>
0. Please feel free to modify any file if you want except for Makefile and test files. You can refer to all the headers provided before (in previous final test). <br/>
1. gdb is a powerful helper especially when your generated executable crashes. <br/>
2. bugs could occur in any phase. You can use methods in <b>printtree.c</b> to find if you have problems in IR trees, and use methods in <b>assem.c</b> to print out the assembly code with/without register allocation. <br/>
3. Each file has explicit function. Implement them step by step. Read the textbook and slides first when encountering problems.<br/>
4. You may find the bugs in previous labs (e.g., incorrect IR).

</p>

<h2>Linking</h2>
An interesting part is to see how our compiled assmbly files will be linked with the standard library. <br/>
Firstly, you need to provide some hints for linker. You can check the example file <b>hello.tig.s</b> for details. <br/>

After generating a valid and annotated assembly file, try running:
<pre>
gcc -Wl,--wrap,getchar -m32 hello.tig.s runtime.c -o hello.out
</pre>

Note that the option <b>-m32</b> is to generate 32-bit executable on 64-bit OS (you may also need to install some supportive libraries to generate 32-bit executable). If your OS is 32-bit, please remove it. </br></br>

On debian or ubuntu: apt-get install libc6-dev-i386

Now you can run the executable:
<pre>
$ ./hello.out
$ Hello world
</pre>

<h2>Testing</h2>
<p>
We've provided a script file called <b>gradeMe.sh</b> for testing. It will use your compiler to generate machine code for programs in <b>testcases</b>, link with standard library, execute it and compare with standard output. If it fails, please check the files in <b>refs-6b/</b>.

<pre>
$ ./gradeMe.sh 
Your score: 100 
</pre> 

<b>100</b> implies that you have passed all the test on Tiger code files in <b>/testcases</b>. 
</p>

<h2>Score and bonus</h2>
The score of this lab consists of three parts. <br/><br/>

<b>Correctness (60%)</b> You can get most scores if you pass all tests, but we will ask you about this part during interview. <br/><br/> 
<b>Escape analysis (20%)</b> The generated code should be much shorter with an escape analysis and an advanced register allocation. We will check it with code inspection and asking questions during interview.<br/><br/>
<b>Coalesce (20%)</b> Coalesce phase removes unnecessary moves but complicates the whole graph coloring phase. We will check it with code inspection and asking questions during interview. <br/><br/>


<h2>Handin procedure</h2>
<p>
Submit your code to TA. You should archive your code as a gzipped tar file with the following command before submission:
<pre>
$ cd lab6/
$ make handin
</pre>
And rename the id.tar.gz with your student number. </br>
</br></br> 
You will receive all the <b>correctness</b> credit if your code passes all testcases in <b>gradeMe.sh</b> when running on our machines. 
</p>

</body>
</html>


