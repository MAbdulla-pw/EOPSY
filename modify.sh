#!/bin/sh

 
#VARIABLES
RECURSIVE_MODE=0
MODE=0  #lowercasing:1 uppercasing:2 sed_pattern:3
SED_PATTERN=""
SCRIPTNAME=`basename $0`

 # Error message to be printed if no arguments were passed and an example of the modify.sh arguments 
printErr(){
	echo "\t no arguments were entered"
        echo "Usage:\n\t$0 [-r] [-l|-u] <dir/file names...>\n\t$0 [-r] <sed pattern> <dir/file names...>\n\t$0 [-h]\n "
        
}
 
modifySingleFile(){
		# store file name and drectoy
                CURRENT_NAME=`basename "$1"`
                CURRENT_PATH=`dirname "$1"`
 
                # To avoid modifying the script file (modify.sh)
                if [ "$CURRENT_NAME" = "$SCRIPTNAME" ] ; then
                        return
                fi
				# 1= Lowercase, 2= Uppercase and 3= sed pattern 
                case $MODE in
                        "1") NEW_NAME=`echo "$CURRENT_NAME" |  tr [:upper:] [:lower:]` ;;
                        "2") NEW_NAME=`echo "$CURRENT_NAME" |  tr [:lower:] [:upper:]` ;;
                        "3") NEW_NAME=`echo "$CURRENT_NAME" |  sed -e $SED_PATTERN` ;;
                esac
 
                #New filename is prepared, check if modifying the name won't override anything
				# if it alreasy exist echo message and return else move new name to $1(current file name)
                if [ -f "$CURRENT_PATH/$NEW_NAME" ] ; then
                         `echo "File $NEW_NAME already exists, name modifying failed" 1>&2`
                        return
                else
                        mv "$1" "$CURRENT_PATH/$NEW_NAME"
                fi     
}

modifyNames(){
        if [ $RECURSIVE_MODE -eq 1 ] ; then
                if [ -d "$1" ] ; then
                 # find "$1" -depth -type d | while read -r dir ; do #echo "$dir"
                        find "$1"  -depth -type f | while read -r file ; do #echo "$file"
                                # echo "\t\t\t\t\t3"
                                modifySingleFile "$file"
                        done
                 # done
                else    # $1 is not a directory
                        # echo "\t\t\t\t1"
                modifySingleFile "$1"
                fi
        else            # Recursive mode is off
                # echo "\t\t\t\t2"
                modifySingleFile "$1"
        fi
}
 
if [ $# -eq 0 ]; then			# if arguments value equal 0 printErr function is called
        printErr
fi
 
while [ "$1" != "" ]; do		# While $1 value is not null
        case "x$1" in			# Check the value of the first argument to determine whether its uppercasing, lowercasing or recursive.
                "x-r")  RECURSIVE_MODE=1 ;;
                "x-l")  MODE=1 ;;
                "x-u")  MODE=2 ;;
                "x-h")  printerr
                                exit ;;
                *)
                # current argument is not a parameter, it can be a sed pattern
                if [ $MODE -eq 0 ]; then
                        MODE=3
                        SED_PATTERN="$1"
                else		# mode is not equal zero, thus start modifyNames function.
                        modifyNames "$1"
                fi
                ;;
        esac
shift							# shift next value into $1 untill null

done
