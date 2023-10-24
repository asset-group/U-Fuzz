import os 
import random
import time
import subprocess
cmd_lst = ["./coap-client -m get coap://10.13.210.82/test",
           "./coap-client -m get coap://10.13.210.82/.well-known/core",
           "./coap-client -m put -e hhhh coap://10.13.210.82/test",
           "./coap-client -m post -e hhhh coap://10.13.210.82/test"]
cmd_lst_2 = [['./coap-client','-m', 'get', 'coap://10.13.210.82/test'],
             ['./coap-client','-m', 'get', 'coap://10.13.210.82/.well-known/core'],
             ['./coap-client', '-m', 'put', '-e', 'hhhh', 'coap://10.13.210.82/test'],
             ['./coap-client', '-m', 'post', '-e', 'hhhh', 'coap://10.13.210.82/test']]

while(True):
    random_int = random.randint(0,3)
    print("executing: \n",cmd_lst_2[random_int])
    # try:
    p = subprocess.Popen(cmd_lst_2[random_int])
    try:
        p.wait(2)
    except subprocess.TimeoutExpired:
        p.kill
    # os.system(cmd_lst[random_int])
    # except:
    #     continue
    time.sleep(0.2)