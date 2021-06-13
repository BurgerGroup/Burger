import os
import string
import datetime

main = "taskset -c ../../build/release-cpp11/bin/chat_loadtest 127.0.0.1 8888 "
## valgrind
##  --tool=helgrind / --tool=drd  
## --tool=memcheck --leak-check=full
# main = "valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all ../../build/chat_loadtest 127.0.0.1 8888 "

# main = "../../build/chat_loadtest 127.0.0.1 8888 "
clientsNum = [10, 100, 10000] 
# clientsNum = [20000]  
iterations = 5

# https://www.cnblogs.com/juandx/p/4962089.html
output = open("result_delta_time_with_preCo.txt", "a")

print("Expriment time: " + datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S') + "\n\n", file=output)  

for clients in clientsNum :  # 客户端个数
    command = main + str(clients)
    average_begin = 0.0
    average_delta = 0.0 

    for i in range(iterations) :  # 迭代次数
        print(i)
        f = os.popen(command)    
        data = f.readlines()    
        f.close() 
        begin_time = 0.0
        end_time = 0.0

        for line in data:   
            try: 
                percentage = int(line[3:6].replace(" ", ""))
            except ValueError:
                continue
            else:
                if percentage == 0 :
                    begin_time = float(line[-9:])
                    average_begin += begin_time

                if percentage == 100 :
                    end_time = float(line[-9:])
                    average_delta += end_time - begin_time

    average_begin /= iterations
    average_delta /= iterations
    print("Result of " + str(clients) + " clients (Iteration " + str(iterations) + " times) :", file=output)
    print("Average Begin time: " + ("%.6f" % average_begin), file=output)
    print("Average Delta time: " + ("%.6f" % average_delta) + "\n", file=output)  
    
