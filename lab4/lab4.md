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
	<h2>Lab4 Type Checking</h2>
	<p>
	<b>Due:</b> Nov 14, 12:00 am, 2017</br> 
	</p>
<hr>

<h2>Introduction</h2>
<p>
In this lab you need to provide type checking for Tiger programs. <br/>Please read <b>chapter 5</b> of your textbook carefully before you start.
</p>

<h2>Getting started</h2>
<p>
Download the orginal environment <a href="https://ipads.se.sjtu.edu.cn/courses/compilers/handout-2017/lab4.tar.gz">here</a>.
<pre>
$ tar xf lab4.tar.gz
$ cd lab4/
</pre>

We will reuse your lexer and parser in previous labs. Please copy it to the working directory:
<pre>
$ cp lab3/tiger.lex ./tiger.lex
$ cp lab3/tiger.y ./tiger.y
</pre>
</p>
<p>
We suggest you put all your implementation in "semant.c".
</p>

<h2>Testing</h2>
<p>
The format of a type error message is shown below: 
<pre>
[FILENAME]<b>:</b>[LINE NO]<b>.</b>[COLUMN NO]<b>:</b>[ERROR MESSAGE]

(for example) test10.tig:2.21:while body must produce no value 
</pre>

We've provided a script file called <b>gradeMe.sh</b> for testing. <br/> 

<pre>
$ ./gradeMe.sh 
[^_^]: Pass Lab4
</pre> 

The smiling face implies that you have passed all the test on Tiger code files in <b>/testcases</b>. 
</p>

<h2>Handin procedure</h2>
<p>
Submit your code to TA. You should archive your code as a gzipped tar file with the following command before submission:
<pre>
$ cd lab4/
$ make handin
</pre>
And rename the id.name.tar.gz with your student number. </br></br>
You will receive full credit if your code passes the test in <b>gradeMe.sh</b> when running on our machines. We will use the timestamp of your <b>last</b> submission for the purpose of calculating late days.

</p>
