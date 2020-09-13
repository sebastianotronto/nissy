#!/bin/sh

OUT=./src/helppages.h
DOCDIR=./docs
N=$(ls -1 $DOCDIR | wc -l)

# This deletes the content of the file
echo "/* To generate this help page, use the script makedoc.sh */" > $OUT

echo "" >> $OUT
echo "int Npages = $N;" >> $OUT
echo "" >> $OUT
echo "char *helppages[][10] = {" >> $OUT

for f in $(ls $DOCDIR)
do
  name=$(echo "$f" | sed 's/\..*//')
  echo "" >> $OUT
  echo "{ \"$name\"," >> $OUT

  echo "\"\\" >> $OUT
  sed 's/$/\\n\\/;s/\"/\\\"/g' $DOCDIR/$f >> $OUT
  echo "\"" >> $OUT

  echo "}," >> $OUT
done

echo "};" >> $OUT

