#!/bin/bash

function doReap(){
	rm -f _tmp.txt .tmp.txt	
}

doReap
make clean >& /dev/null
make > /dev/null
if [[ $? != 0 ]]; then
	echo "[-_-]: Compile Error"		
	echo "SCORE: 0"
	exit 1
fi

test_num=$(( ($RANDOM % 2) ))

if [[ $test_num == 0 ]]; then
	REFOUTPUT=./ref-0.txt
else
	REFOUTPUT=./ref-1.txt
fi

./a.out $test_num >& _tmp.txt
diff -w  _tmp.txt $REFOUTPUT >& .tmp.txt
if [ -s .tmp.txt ]; then
	echo "[*_*]: Output Mismatch"
	cat .tmp.txt
	doReap
	echo "SCORE: 0"
	exit 0
fi

doReap
echo "[^_^]: Pass"
echo "SCORE: 100"


