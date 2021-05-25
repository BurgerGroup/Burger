import os
main = "../../build/debug-cpp11/chat_loadtest 127.0.0.1 8888 "

clientsNum = [4] 
for x in clientsNum :
    command = main + str(x)
    # f = os.popen(command)    
    # data = f.readlines()    
    # f.close()    
    # print(data)  
    os.system(command)