#!bin/sh
rm -rf  cscope.files cscope.files tags



find `pwd` \( -name '*.c' -o -name '*.cpp' -o -name '*.cc' -o -name '*.h' -o -name '*.s' -o -name '*.S' \) -print > cscope.files
cscope -Rbq -i cscope.files
ctags -R
