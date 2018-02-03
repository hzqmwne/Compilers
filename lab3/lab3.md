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
	<h2>Lab3 Syntax Analysis</h2>
	<p>
	<b>Due:</b> Oct 22, 12:00 a.m, 2017</br> 
	</p>
<hr>

<h2>Introduction</h2>
<p>
In this lab you will implement a parser for the Tiger language with Yacc. <br/>This lab is divided into two parts: in Part A you need to implement a parser for tiger programs (chap 3), while in Part B you are required to add semantic actions for constructing abstract syntax trees (chap 4).
</p>

<h2>Getting started</h2>
<p>
Download the orginal environment <a href="https://ipads.se.sjtu.edu.cn/courses/compilers/handout-2017/lab3.tar.gz">here</a>.
<pre>
$ tar -xf lab3.tar.gz
$ cd lab3/
</pre>

We will use your lexer in lab2 to handle lexical analysis. Please copy it to the working directory:
<pre>
$ cp lab2/tiger.lex ./tiger.lex
</pre>

Note that the included headers in tiger.lex should be modified to make your compiler happy. You should delete <b>tokens.h</b> and add <b>symbol.h</b>, <b>absyn.h</b> and <b>y.tab.h</b>.
<pre>
  #include &lt;string.h>
  #include "util.h"
+ #include "symbol.h"
- #include "tokens.h"
  #include "errormsg.h"
+ #include "absyn.h"
+ #include "y.tab.h"
</pre>

</p>

<h2>Guide</h2>
<p>
0. You can <b>ONLY</b> modify tiger.lex and tiger.y.</br>
1. <b>Precedence rules</b> are very helpful in resolving conflicts, but you should be careful to use them. </br>
2. You can <b>ONLY</b> construct the AST with APIs provided by <b>absyn.h</b>. Please <b>DON'T</b> modify the APIs!</br>
3. For this and the following labs, when meeting an empty string, you should set the corresponding value to "" instead of "(null)". (please modify your tiger.lex)</br>
4. When encountering a varible declaraion without a specified type, please fill the type field with "" instead of NULL.</br>
</p>

<h2>Testing</h2>
<p>
We've provided a script file called <b>gradeMe.sh</b> for testing. <br/>
For part A, try running:
<pre>
$ ./gradeMe.sh syntax
[^_^]: Pass Part A
</pre> 

For part B (the whole lab), try running:
<pre>
$ ./gradeMe.sh
[^_^]: Pass Lab3
</pre>

The smiling face implies that you have passed all the test on Tiger code files in <b>/testcases</b>. In part A, <b>gradeMe.sh</b> checks if your parser can handle all the tokens and generate a correct return value. In part B, <b>gradeMe.sh</b> compares your output with files in <b>/refs-2</b>, so if there's something wrong with your answer, you can check the files in <b>/refs-2</b>.
</p>

<h2>Final Test</h2>
<p>
This step is only for finding the bugs as early as possible.</br>
If your environment can run the final_tiger directly, you can kick the virtual machine away.</br>
For the final test, you need to download the new <a href="https://ipads.se.sjtu.edu.cn/courses/compilers/handout-2017/lab3/gradeMe.sh">gradeMe.sh</a> and <a href="https://ipads.se.sjtu.edu.cn/courses/compilers/handout-2017/lab3/Makefile">Makefile</a>.</br>
<pre>
$ copy these two files into the final_tiger in virtual machine.
$ copy lab3/tiger.lex into ./final_tiger
$ copy lab3/tiger.y into ./final_tiger
$ ./gradeMe.sh
</pre>


</p>


<h2>Handin procedure</h2>
<p>
Submit your code to TA. You should archive your code as a gzipped tar file with the following command before submission:
<pre>
$ cd handout_lab3/
$ make handin
</pre>
And rename the lab3.tar.gz with your student number. </br></br>
You will receive full credit if your code passes the test in <b>gradeMe.sh</b> when running on our machines. We will use the timestamp of your <b>last</b> submission for the purpose of calculating late days.

</p>

</div>
</body>
</html>
