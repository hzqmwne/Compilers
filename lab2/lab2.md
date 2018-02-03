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
	<h2>Lab2 Lexical Analysis</h2>
	<p>
	<b>Due:</b> Oct 7, 12:00 a.m, 2017 <font color="red">At noon!</font></br> 
	</p>
<hr>

<h2><font color="red">Update</font></h2>
<p>
<a href="https://ipads.se.sjtu.edu.cn/courses/compilers/handout-2017/update.tar.gz">Here</a> are the two new testcases (test50.out and test51.out). Please copy them into directory named "refs".
</p>

<h2>Introduction</h2>
<p>
In this lab you will implement a lexical analyzer for the Tiger language with Lex. Please read <b>chapter 2</b> of your textbook carefully before you start.
</p>

<h2>Getting started</h2>
<p>
Download the orginal environment <a href="https://ipads.se.sjtu.edu.cn/courses/compilers/handout-2017/lab2.tar.gz">here</a>.
<pre>
$ tar -xf lab2.tar.gz
$ cd ~/lab2
</pre>
</p>

<h2>Guide</h2>
<p>
0. You are only allowed to modify <b>tiger.lex</b> exclude the first several lines.
</p>
<p>
1. This lab is the first step to build our tiger compiler. So you should <b>carefully manage</b> your code including the following labs. For example, using GIT or cloud storage.
</p>

<p>
2. We recommend using <b>states</b> to handle comments (please refer to page 32-33 in the text book). Specifically, tiger language allows <b>nested comments</b>.
This one is legal:
<pre>
/* This is comment. /*nested comment*/*/
</pre>
While this one is illegal:
<pre>
/* This is comment. /*nested comment*/
</pre>
</p>

<p>
3. When handing an "ID", you should use <b>"String"</b> function to allocate the object.
</p>

<p>
4. Besides, you should turn in <b>documentation</b> for the following points: 
</p>
<p>
<li> how you handle comments
<li> how you handle strings
<li> error handling
<li> end-of-file handling
<li> other interesting features of your lexer
</p>
<p>
Send your <b>documentation</b> to <b>TA </b> with the name of `studentID_name_lab2_document.pdf`
</p>

<h2>Testing</h2>
<p>
<b>Warning:</b> Before testing, please make sure that your compiler generates no warnings. Our testing environment might be different from yours, so warnings may be evil sometimes. Watch out! 

<hr>
We've provided a script file called <b>gradeMe.sh</b> for testing. After programming, try running:
<pre>
$ ./gradeMe.sh
[^_^]: Pass
</pre> 
The smiling face implies that you have passed all the test on Tiger code files in <b>/testcases</b>. <b>gradeMe.sh</b> compares your output with the files in <b>/refs</b>, so if there's something wrong with your answer, you can check the content in <b>/refs</b>.
</p>

<h2>Final Test</h2>
<p>
Since our tiger compiler is built step by step, there may exist some hidden bugs in current step.
Therefore, we provide a final test for uncovering potentail bugs. </br>
Although this test does not affect your score of this lab, it can help you find bugs as early as possible.
</p>
<p>
In case of some compatible issues, we provide a virutal machine (runs on VMware) for the final test. </br>
It is <b>not</b> necessary to coding inside the VM (Nevertheless, it has a workable environment.)
You can download it from <a href="https://jbox.sjtu.edu.cn/l/71KeTu">JBox</a>. VM user: tmac ; password: 123 
<pre>
For testing,
$ copy tiger.lex into VM at first
$ cp tiger.lex ~/final-tiger
$ cd final-tiger
$ ./gradeMe.sh
</pre>
</p>


<h2>Handin procedure</h2>
<p>
Submit your code to TA. You should archive your code as a gzipped tar file with the following command before submission:
<pre>
$ cd lab2
$ make handin
$ change the name of tar to: e.g. 5150379000.tar.gz (Do not add other characters!)
</pre>
You will receive full credit if your code passes the test in <b>gradeMe.sh</b> when running on our machines. We will use the timestamp of your <b>last</b> submission for the purpose of calculating late days.
</p>
</div>
</body>
</html>
