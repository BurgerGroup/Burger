import os
main = "../../build/debug-cpp11/chat_loadtest 127.0.0.1 8888 "

clientsNum = [10, 100] 

# https://www.cnblogs.com/juandx/p/4962089.html

output = open("result.txt", "w")
for x in clientsNum :
    command = main + str(x)
    f = os.popen(command)    
    data = f.readlines()    
    f.close() 
    print("Result of " + str(x) + " clients :\n", file=output)
    for line in data:   
        print(line, file=output)  
    