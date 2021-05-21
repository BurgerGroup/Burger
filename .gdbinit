# @ put file in ~/.gdbinit 
# @ mv ./gdbinit ~/.gdbinit
# @ brief : skip standard .h when gdb
skip -gfi /usr/include/c++/5/*
skip -gfi /usr/include/c++/*/*/*
skip -gfi /usr/include/c++/*/*
skip -gfi /usr/include/c++/*
