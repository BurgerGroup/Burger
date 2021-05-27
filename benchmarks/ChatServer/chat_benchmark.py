import os
main = "../../build/debug-cpp11/chat_loadtest 127.0.0.1 8888 "
## valgrind
##  --tool=helgrind / --tool=drd  
## --tool=memcheck --leak-check=full
# main = "valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all ../../build/chat_loadtest 127.0.0.1 8888 "
# main = "../../build/chat_loadtest 127.0.0.1 8888 "
clientsNum = [10, 100, 1000, 10000] 

# https://www.cnblogs.com/juandx/p/4962089.html

output = open("result.txt", "a")
for x in clientsNum :
    command = main + str(x)
    f = os.popen(command)    
    data = f.readlines()    
    f.close() 
    print("Result of " + str(x) + " clients :\n", file=output)
    for line in data:   
        print(line, file=output)  
    