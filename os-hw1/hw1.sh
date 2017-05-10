#!/bin/bash

#initiation

cells[1]=' '
cells[2]=' '
cells[3]=' '
cells[4]=' '
cells[5]=' '
cells[6]=' '
cells[7]=' '
cells[8]=' '
cells[9]=' '

gameOver=false
gameDraw=false

function drawDesk {
	clear

	echo '-------'
	echo '|'${cells[1]}'|'${cells[2]}'|'${cells[3]}'|'
	echo '-------'
	echo '|'${cells[4]}'|'${cells[5]}'|'${cells[6]}'|'
	echo '-------'
	echo '|'${cells[7]}'|'${cells[8]}'|'${cells[9]}'|'
	echo '-------'
}

function makeStep {
	if [[ "$step" == "exit" ]]; then
		quit
		return
	fi

	re="^[1-3] [1-3]$"
	if ! [[ $step =~ $re ]]; then
		error=true
		return
	fi
	if [ $myStep == true ]; then
		sign=$signMy
	else
		sign=$signHis
	fi

	row=${step:0:1}
	col=${step:2:1}
	cell=$((($row - 1) * 3 + $col))

	if ! [ "${cells[$cell]}" == " " ]; then
		error=true
		return
	fi

	cells[$cell]=$sign
	
	return
	
}

function checkGameOver {
	if [ "${cells[1]}" != " " ] && [ "${cells[1]}" == "${cells[2]}" ] && [ "${cells[2]}" == "${cells[3]}" ]; then
		gameOver=true
		return
	fi
	if [ "${cells[4]}" != " " ] && [ "${cells[4]}" == "${cells[5]}" ] && [ "${cells[5]}" == "${cells[6]}" ]; then
		gameOver=true
		return
	fi
	if [ "${cells[7]}" != " " ] && [ "${cells[7]}" == "${cells[8]}" ] && [ "${cells[8]}" == "${cells[9]}" ]; then
		gameOver=true
		return
	fi
	if [ "${cells[1]}" != " " ] && [ "${cells[1]}" == "${cells[4]}" ] && [ "${cells[4]}" == "${cells[7]}" ]; then
		gameOver=true
		return
	fi
	if [ "${cells[2]}" != " " ] && [ "${cells[2]}" == "${cells[5]}" ] && [ "${cells[5]}" == "${cells[8]}" ]; then
		gameOver=true
		return
	fi
	if [ "${cells[3]}" != " " ] && [ "${cells[3]}" == "${cells[6]}" ] && [ "${cells[6]}" == "${cells[9]}" ]; then
		gameOver=true
		return
	fi
	if [ "${cells[1]}" != " " ] && [ "${cells[1]}" == "${cells[5]}" ] && [ "${cells[5]}" == "${cells[9]}" ]; then
		gameOver=true
		return
	fi
	if [ "${cells[3]}" != " " ] && [ "${cells[3]}" == "${cells[5]}" ] && [ "${cells[5]}" == "${cells[7]}" ]; then
		gameOver=true
		return
	fi

	for i in {1..9}
	do
		if [ "${cells[$i]}" == " " ]; then
			return
		fi
	done

	gameDraw=true
	return
}

function quit {
	if [ -p $pipe1 ]; then
		rm $pipe1
	fi
	if [ -p $pipe2 ]; then
		rm $pipe2
	fi
}

# Draw game field
drawDesk

pipe1=/tmp/pipe1
pipe2=/tmp/pipe2

if [[ -p $pipe1 ]] && [[ -p $pipe2 ]]; then
	rm $pipe1
	rm $pipe2
fi

if [[ ! -p $pipe1 ]]; then
	mkfifo $pipe1
	output=$pipe2
	input=$pipe1
	signMy='x'
	signHis='o'
	myStep=true
else
	if [[ ! -p $pipe2 ]]; then
		mkfifo $pipe2
	else
		rm $pipe2
		mkfifo $pipe2
	fi
	output=$pipe1
	input=$pipe2
	signMy='o'
	signHis='x'
	myStep=false	
fi

while true
do
	if [[ ! -p $pipe1 ]] && [[ ! -p $pipe2 ]]; then
		break
	fi

	checkGameOver

	if [ $gameDraw == true ]; then
		if [ $myStep == true ]; then
			tput cup 8 0
			echo 'Draw!!!'
		else

			tput cup 8 0
			echo 'Draw!!!'
		fi
		quit
	fi

	if [ $gameOver == true ]; then
		if [ $myStep == true ]; then
			tput cup 8 0
			echo 'You Lost'
		else

			tput cup 8 0
			echo 'You Won'
		fi
		quit
	fi

	error=false

	if [[ ! -p $pipe2 ]]; then
		continue
	fi



	if [ $myStep == true ]; then
		tput sc
		read -p 'Input cell number ' step
		tput rc

		# Check and write input to array
		# Sets 'error' and 'gameOver' flags
		makeStep

		if [ $error == true ]; then
			drawDesk
			continue
		fi

		drawDesk

		echo -n $step > $output

		myStep=false
	else
		read step < $input

		makeStep

		if [ $error == true ]; then
			drawDesk
			continue
		fi

		drawDesk

		myStep=true
	fi

done


