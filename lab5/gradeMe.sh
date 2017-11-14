#!/bin/bash
#============some output color
SYS=$(uname -s)
if [[ $SYS == "Linux" ]]; then
	RED_COLOR='\E[1;31m'  
	GREEN_COLOR='\E[1;32m' 
	YELOW_COLOR='\E[1;33m' 
	BLUE_COLOR='\E[1;34m'  
	PINK='\E[1;35m'      
	RES='\E[0m'
fi


BIN=a.out
#PROJDIR=lab4
REFOUTDIR=./refs-5
TESTCASEDIR=./testcases
DIFFOPTION="-w -B"
ret_value=0
rm -f _tmp.txt .tmp.txt
#echo $PROJDIR/$ite
#pwd
make clean >& /dev/null
make >& /dev/null
#echo $?
if [[ $? != 0 ]]; then
	echo -e "${RED_COLOR}[-_-]$ite: Compile Error${RES}"		
	exit 123
fi	
	for tcase in `ls $TESTCASEDIR/`
	do		
		if [ ${tcase##*.} = "tig" ]; then
			tfileName=${tcase##*/}
			#echo $tfileName
			rm -f _ref.txt _tmp.txt .tmp.txt
			./$BIN $TESTCASEDIR/$tfileName >& _tmp.txt

			diff $DIFFOPTION _tmp.txt $REFOUTDIR/${tfileName%.*}.out >& .tmp.txt
			if [ -s .tmp.txt ]; then
				#cat .tmp.txt
				echo -e "${RED_COLOR}Your output:${RES}"
				cat _tmp.txt
				echo -e "${RED_COLOR}Expected output:${RES}"
				cat $REFOUTDIR/${tfileName%.*}.out 
				echo -e "${BLUE_COLOR}[*_*]$ite: Output Mismatch [$tfileName]${RES}"
				rm -f _tmp.txt .tmp.txt
				exit 234
			fi
		fi
	done
rm -f _tmp.txt .tmp.txt 
echo -e "${GREEN_COLOR}[^_^]$ite: Pass Lab5${RES}"
