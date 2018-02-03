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

<h2>Rules</h2>
	<ol>
	 <li><font color="red">Prohibit plagiarism!</font></li>
	 <li>Uploading the lab: you must use <font color="red">`make handin`</font> provided by the Makefile and change the file name with your student ID.</li>
	 <li>You will get <font color="red">0 point</font> if missing the deadline.</li>
	</ol>


<h2>Lab1 Straight-line program interpreter</h2>
	<p>
	<b>Due:</b> Sep 17, 12:00, 2017 <font color="red">At noon!</font></br> 
	If you have any problem about this lab, please contact TA
	</p>
<hr>


<h2>Introduction</h2>
<p>
In this lab you will implement a simple straight-line program interpreter for the programming language described in <b>chapter 1</b> of your text book. Please read it carefully before you start.
</p>
<p>
This lab serves as an introduction to:
</p>
<p>
	&nbsp&nbsp&nbsp&nbsp<b>environments</b> (symbol tables mapping variable-names to information about the variables)
</p>
<p>
	&nbsp&nbsp&nbsp&nbsp<b>abstract syntax</b> (data structures representing the phrase structure of programs)
<p>
<p>
	&nbsp&nbsp&nbsp&nbsp<b>recursion over tree data structures</b> (which is useful in many parts of a compiler)
</p>

<p> Please pay attention to the following things: </p>
	<p>&nbsp&nbsp&nbsp&nbspWrite the interpreter <b>without side effects</b> (Please read your text book carefully)</p>



<h2>Getting started</h2>
<p>
Download the orginal environment <a href="https://ipads.se.sjtu.edu.cn/courses/compilers/handout-2017/lab1.tar.gz">here</a>.
</p>
<p>
Hint: You should first have a quick all the files. You must <font color="red">put all your implementation</font> in the file called `myimpl.c`. Do not modify other files!
<pre>
$ tar -xf lab1.tar.gz
$ cd lab1
</pre>

</p>

<h2>Testing</h2>
<p>
We've provided a script file called <b>gradeMe.sh</b> for testing. After programming, try running:
<pre>
$ ./gradeMe.sh
[^_^]: Pass
</pre> 
The smiling face implies that you have passed all the test points. <b>gradeMe.sh</b> compares your output with <b>ref.txt</b>, so if there's something wrong with your answer, you can check the content in <b>ref.txt</b>.
</p>

<h2>Handin procedure</h2>
<p>
Submit your code to TA. You should archive your code as a gzipped tar file with the following command before submission:
<pre>
$ cd lab1
$ make handin 
$ change the name of tar to: e.g. 5150379000.tar.gz (Do not add other characters!)
</pre>
You will receive full credit if your code passes the test in <b>gradeMe.sh</b> and your interpreter is a '<b>without side effect</b>' interpreter. 
<p></p>
We will use the timestamp of your <b>last</b> submission for the purpose of calculating late days.

</p>
</div>
</body>
</html>
