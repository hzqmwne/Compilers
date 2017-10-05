#!/bin/bash
#============some output color
#RED_COLOR='\E[1;31m'  
#GREEN_COLOR='\E[1;32m' 
#YELOW_COLOR='\E[1;33m' 
#BLUE_COLOR='\E[1;34m'  
#PINK='\E[1;35m'      
#RES='\E[0m'
RED_COLOR=''  
GREEN_COLOR='' 
YELOW_COLOR='' 
BLUE_COLOR=''  
PINK=''      
RES=''

BIN=a.out
#PROJDIR=lab3
REFOUTDIR=./refs-2
TESTCASEDIR=./testcases
#DIFFOPTION="-w -B"
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

if [[ $1 = "syntax" ]]; then
	for tcase in `ls $TESTCASEDIR`
	do		
		if [ ${tcase##*.} = "tig" ]; then
			tfileName=${tcase##*/}
			./$BIN $TESTCASEDIR/$tfileName >& _tmp.txt
			ret_value=$?
			# fix bison version diff
			if [ $tfileName = "test49.tig" ]; then
				if [[ $ret_value = 0 ]]; then
					echo -e "${BLUE_COLOR}[*_*]$ite: This case should be syntax error [$tfileName]${RES}"				
					rm -f _tmp.txt .tmp.txt	
					exit 234
				fi
				continue
			fi

			# normal case
			if [[ $ret_value != 0 ]]; then
				echo -e "${BLUE_COLOR}[*_*]$ite: Syntax Error [$tfileName]${RES}"
				rm -f _tmp.txt .tmp.txt	
				exit 234
			fi
		fi	
	done
else
	for tcase in `ls $TESTCASEDIR`
	do		
		if [ ${tcase##*.} = "tig" ]; then
			tfileName=${tcase##*/}
			./$BIN $TESTCASEDIR/$tfileName >& _tmp.txt
			ret_value=$?
			# fix bison version diff
			if [ $tfileName = "test49.tig" ]; then
				if [[ $ret_value = 0 ]]; then
					echo -e "${BLUE_COLOR}[*_*]$ite: This case should be syntax error [$tfileName]${RES}"				
					rm -f _tmp.txt .tmp.txt	
					exit 234
				fi
				continue
			fi

			# normal case
			if [[ $ret_value != 0 ]]; then
				echo -e "${BLUE_COLOR}[*_*]$ite: Syntax Error [$tfileName]${RES}"
				rm -f _tmp.txt .tmp.txt	
				exit 234
			fi
		fi	
	done
	echo -e "${GREEN_COLOR}[^_^]$ite: Pass Part A${RES}"
	# echo "TOTAL SCORE: 50"

	for tcase in `ls $TESTCASEDIR`
	do		
		if [ ${tcase##*.} = "tig" ]; then
			tfileName=${tcase##*/}
			

			./$BIN $TESTCASEDIR/$tfileName 0 >& _tmp.txt
			# fix bison version diff
			if [ $tfileName = "test49.tig" ]; then
				#echo $tfileName
				#cat _tmp.txt
				grep 'test49.tig:5.18: syntax error' _tmp.txt >& .tmp.txt
				if [ -s .tmp.txt ]; then
					grep 'test49.tig:5.18: parse error' _tmp.txt >& .tmp.txt
					if [ -s .tmp.txt ]; then
						cat _tmp.txt
						echo -e "${BLUE_COLOR}[*_*]$ite: Output Mismatch [$tfileName]${RES}"
						rm -f _tmp.txt .tmp.txt	
						exit 234
					fi
				fi
				continue
			fi

			# normal case
			diff $DIFFOPTION _tmp.txt $REFOUTDIR/${tfileName%.*}.out >& .tmp.txt
			if [ -s .tmp.txt ]; then
				cat .tmp.txt
				echo -e "${BLUE_COLOR}[*_*]$ite: Output Mismatch [$tfileName]${RES}"
				rm -f _tmp.txt .tmp.txt	
				exit 234
			fi
		fi	
	done

fi

rm -f _tmp.txt .tmp.txt	
if [[ $1 = "syntax" ]]; then	
echo -e "${GREEN_COLOR}[^_^]$ite: Pass Part A${RES}"
else
echo -e "${GREEN_COLOR}[^_^]$ite: Pass Lab3${RES}"
echo "TOTAL SCORE: 100"
fi
