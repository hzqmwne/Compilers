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
	<h2>Lab5 Intermediate Code</h2>
	<p>
	<b>Due:</b> Nov 26, 12:00, 2017</br> 
	</p>
<hr>

<h2>Introduction</h2>
<p>
In this lab you need to implement the <b>frame</b> data structure and generate intermediate representation (IR) trees for tiger programs. <br/>Please read <b>chapter 6 & 7</b> of your textbook carefully before you start.
</p>

<h2>Getting started</h2>
<p>
Download the orginal environment <a href="https://ipads.se.sjtu.edu.cn/courses/compilers/handout-2017/lab5.tar.gz">here</a>.
<pre>
$ tar -xf lab5.tar.gz
$ cd lab5/
</pre>

Now look around. Here is a list of files you need to modify: </br>

<b>frame.h</b>:   	basic definitions for frame data structure. </br></br>

<b>x86frame.c</b>:	specialized implementation for frame atop x86 architecture.</br></br>

<b>semant.[ch]</b>: 	you should modify them to generate IR trees while type checking. </br>
One thing to note is that now <b>SEM_transProg</b> returns a list of fragments instead:

<pre>
in semant.h:
- void SEM_transProg(A_exp exp);
+ F_fragList SEM_transProg(A_exp exp);
</pre>

Please check semant.[ch] in <b>handout_lab5/</b> for more details. </br></br>

<b>translate.[ch]</b>:	translating abstract syntax nodes into IR tree nodes. </br></br>

In addition, we will reuse your code in previous labs. Please merge them to the working directory.
Please copy tiger.y and tiger.lex into this lab, and <b>merge</b> any modified file (e.g., semant.c) in lab4 to this directory
</p>

<h2>Hint</h2>
1. Generate the IR during semantic check. Specifically, when traversing the AST in semant.c, you should invoke some functions (defined in translate.c) 
   to generate the corresponding IR for each AST part. </br>
2. In Lab2/3, we provide a final finished project for testing. If necessary, you can refer to the header files.

<h2>Testing</h2>
<p>
Since there is no standard output for IR trees, we only provide a simple test for this lab. A more complete test will be conducted after code generation.

In this lab, we ask you to count the number of fragments (both string fragments and proc fragments) and print out. <br/>

We've provided a script file called <b>gradeMe.sh</b> for testing. <br/> 

<pre>
$ ./gradeMe.sh
Your output:
0
Expected output:
19
[*_*]: Output Mismatch [merge.tig]
</pre>
The output suggests that merge.tig has 19 fragments in all.


<pre>
$ ./gradeMe.sh 
[^_^]: Pass Lab5
</pre> 

The smiling face implies that you have passed all the test on Tiger code files in <b>/testcases</b>. 
</p>

<h2>Handin procedure</h2>
<p>
Submit your code to TA. You should archive your code as a gzipped tar file with the following command before submission:
<pre>
$ cd handout_lab5/
$ make handin
</pre>
And rename the id.name.tar.gz with <b>your student number</b>(e.g., 5150379000.tar.gz). </br></br>
You will receive full credit if your code passes the test in <b>gradeMe.sh</b> when running on our machines. We will use the timestamp of your <b>last</b> submission for the purpose of calculating late days.

</p>
</body>
</html>
