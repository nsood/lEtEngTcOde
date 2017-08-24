#!/bin/bash

rm -f files_info
rm -f file_header
rm -f content.bin
rm -f wWrelease.bin

if [ $# -gt 0 ]
then
		exit 0
fi

touch file_header
touch files_info
touch content.bin

flen=0
filename=""

for subdir in content/*
do 
		if test -d $subdir
		then
				for file in $subdir/*
				do
					if test -f $file
					then
						filename=`basename $file`
						flen=`cat $file | wc -c`
						cat $file >> content.bin
						echo -n "$filename $flen;" >> files_info 
					fi
				done
		fi
done


flen=`cat files_info | wc -c`

flen_len=${#flen}
for ((i=$flen_len;i<4;i++));do
		flen="0"$flen
done

echo $flen
echo -n "wwcx$flen" >> file_header 

cat file_header files_info content.bin >> wWrelease.bin

rm -f files_info
rm -f file_header
rm -f content.bin
