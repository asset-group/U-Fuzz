from numpy import tensordot
import pyshark
import os
import json
import time
import subprocess
from wdissector import *
import ctypes
from itertools import chain, combinations
from binascii import hexlify
from colorama import Fore, Back, Style
# input of the usser
# packet sequence number
# name of the state
dictionary_haha ={}
@ctypes.CFUNCTYPE(UNCHECKED(c_ubyte), POINTER(proto_tree), c_ubyte, POINTER(c_ubyte))
# this test function will call the wdissector to output every layer, fieldname and value of a packet
def test(subnode, field_type, pkt_buf):
    global dictionary_haha
    # dictionary_haha.clear()
    # test_dictionary = dictionary_haha.copy()
    # dic = {}
    if field_type == 0:
        field_name = packet_read_field_abbrev(subnode.contents.finfo)
        decoded_field_name = str(field_name.decode())
        # print(field_name.decode())
    #   print(decoded_field_name)
        layer = decoded_field_name.split('.')[0]
        value = packet_read_field_uint32(subnode.contents.finfo)
        if layer in dictionary_haha.keys():
                dictionary_haha[layer][decoded_field_name]= value
        else:
                dictionary_haha[layer]={}
                dictionary_haha[layer][decoded_field_name]=value
        
        # print("This is dic",dictionary_haha)
        

        #   print("value",value)
        #   print("String",value_to_string(field_name.decode(),value))
    elif field_type == 1:
            pass
            # value = packet_read_field_abbrev(subnode.contents.finfo)
            # print("This is the value",value.decode())
    elif field_type == 2:
            pass
            layer_name = packet_read_field_abbrev(subnode.contents.finfo)
        #   print('-->', layer_name.decode())
    # print("This is dic",dictionary_haha)
    # dictionary_haha = test_dictionary
    # print("This is dic",dictionary_haha)
    
    return 0


# This function will convert the hex value to the string
def value_to_string(field_name, integer_value):
       hfi = packet_get_header_info(field_name)
       if hfi != ctypes.c_void_p:
              str_val = packet_read_value_to_string(integer_value, hfi)
              if str_val:
                     return str_val.decode()

       return None
# This function will convert the hex array to c_array
def convert_hex_to_c_array(raw_bytes):
    cmd = 'python2 packet_hex_to_c_array.py '+raw_bytes
    test = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
    subprocess_return = test.stdout.read()
    output = subprocess_return.decode()
    # print("This is the type of the dic",type(output))
    return output
    # return os.system('python2 packet_hex_to_c_array.py '+raw_bytes)

# help to get the encapsulation type for each capture
def get_encap_type(c):
    my_packet = c[4]
    full_encap_type = my_packet.frame_info.protocols
    print(full_encap_type)
    encape_type = full_encap_type.split(":")[0]
    return encape_type

# This function will help to get the dictionary whcih has the key of packet seq number
# value of the raw bytes of that packet
def get_raw_bytes_dic(c):
    dic = {}
    for idx,pkt in enumerate(c):
        raw_bytes = pkt.get_raw_packet()
        raw_bytes = raw_bytes.hex()
        # encap_type = get_encap_type(c)
        # dic[idx+1] = {}
        # dic[idx+1] = raw_bytes
        converted = convert_hex_to_c_array(raw_bytes)
        converted_lst = converted.split(',')
        new_set = [x.replace('\n', '') for x in converted_lst]
        # print("This is new_set",new_set)
        final_lst = []
        for x in new_set:
            if x != ' ':
                x_to_int = int(x,16)
                # x_to_hex = hex(x_to_int)
                final_lst.append(x_to_int)
            # print("This is the list",converted.split(','))
            # print("This is the list",final_lst)
        dic[idx+1] = final_lst
            # dic[idx+1]["encap_type"] = encap_type
        # print("This is dic",dic)
    return dic

# This function will output the full dictionary of the capture file
def get_dic(c):
    global dictionary_haha
    fieldname_value_dic = get_raw_bytes_dic(c)
    final_dic = {}
    pkt_summary_dic = {}
    # print("This is the fieldname_value_dic",fieldname_value_dic)
    packet_cleanup()
    for key, value in fieldname_value_dic.items():
        test_pkt = value
        test_pkt = (ctypes.c_ubyte * len(test_pkt))(*test_pkt)
        # packet_set_direction(1)
        packet_dissect(test_pkt, len(test_pkt))
        s = packet_summary()
        # print(s)
        # decoded_summary = ((s.decode()).split(','))[0]
        decoded_summary = ((s.decode()))
        pkt_summary_dic[key] = decoded_summary
        # print(decoded_summary)
        dictionary_haha.clear()
        packet_navigate(1,0,test)
        testit_dic = dictionary_haha.copy()
        dictionary_haha.clear()
        # print(packet_summary())
        # print("111111:",testit_dic)
        # print("222222",dictionary_haha)
        # final_dic.append(dictionary_haha)
        final_dic[key]=testit_dic
    f = open("dictionary.txt", "w")
    f.write(str(final_dic))
    w = open("State_dic.txt", "w")
    w.write(str(pkt_summary_dic))
    # print("This is the pkt_summary_dic",pkt_summary_dic)
    # print("This is the full dic",final_dic)
    return final_dic,pkt_summary_dic

# This function will auto generate the dict which has the key as the state_name 
# and the value as the packet list
def auto_gen_state_dic(pkt_sum_dic):
    state_dic = {}
    for key,value in pkt_sum_dic.items():
        if state_dic.get(value)==None:
            state_dic[value] = []
            state_dic[value].append(int(key))
        else:
            # state_dic[value] = []
            state_dic[value].append(int(key))
    # print("This is the state_dic",state_dic)
    # flag_multi_cap = input(Fore.RED+"Do you want to upload another cap (y/n)?: ")
    # while flag_multi_cap == 'y':
    #     cap_dir = input(Fore.RED+"Please enter the directory of the new cap: ")
    #     c_new = pyshark.FileCapture(cap_dic,include_raw=True, use_json=True)
    #     cap_dic_new, pkt_sum_dic_new = get_dic(c_new)
    #     for key2,value2 in pkt_sum_dic_new.items():
    #         if state_dic.get(value2)==None:
    #             state_dic[value2] = []
    #             state_dic[value2].append(int(key2))
    #         else:
    #             # state_dic[value] = []
    #             state_dic[value2].append(int(key2))
    #     flag_multi_cap=input(Fore.RED+"Do you want to upload another cap (y/n)?: ")
    # print("This is the combined state dic",state_dic)
    f = open("Pkt_summary.txt","w")
    for key,value in state_dic.items():
        x = []
        for i in value:
            x.append(hex(i))
        print(Fore.GREEN+"This is the state name: "+str(key)+"This is the packet list: "+ str(value))
        # This block allows people to edit each pkt list one by one
        # flag_state_name = input(Fore.RED+"Do you think this packet name is good enough (y/n)?: ")
        # if flag_state_name == 'n':
        #     new_name = input(Fore.RED+"Please enter the new name: ")
        #     key = new_name
        # flag_pkt_lst = input(Fore.RED+"Do you think this packet list is sufficient enough (y/n)?: ")
        # while flag_pkt_lst == 'n':
        #     pkt = input(Fore.RED+"Please add new pkt (enter the seq of the pkt): ")
        #     value.append(int(pkt))
        #     value.sort()
        #     flag_pkt_lst = input(Fore.RED+"Do you think this packet list is sufficient enough (y/n)?: ")
        lline = str(key) + " : " + str(value) + "\n"
        f.write(lline)
    # f = open("Pkt_summary.txt","w")
    # f.write(str(state_dic))
    print(Fore.GREEN+"write the potential state with pkt list dic to file Pkt_summary.txt")
    return state_dic

# Ths filter will out put the pkt list whcih contains the pkt after the filter applied
def test_filter(fil,c):
    # global dictionary_haha]
    packet_cleanup()
    # d = pyshark.FileCapture('mqtt_packets_copy.pcapng',include_raw=True, use_json=True)
    field_n_v_dic = get_raw_bytes_dic(c)
    # print("This is fieldname_value_dic",field_n_v_dic)
    pkt_lst = []
    # final_dic = {}
    # print("This is the fieldname_value_dic",fieldname_value_dic)
    for key, value in field_n_v_dic.items():
        # packet_cleanup()
        wifi_pkt = value
        # print("This is the length of the wifi_packet",len(wifi_pkt))
        # print("This is teh wifi_pkt",wifi_pkt)
        # Here got problem
        # print(hexlify(bytes(value)))
        # print(packet_summary().decode())
        wifi_pkt = (ctypes.c_ubyte * len(wifi_pkt))(*wifi_pkt)
        # wifi_pkt = (ctypes.c_ubyte * len(value))(*value)


        # print("THis is teh wifi_pkt_2",wifi_pkt)
        packet_set_direction(1)
        packet_register_condition(fil,0)
        packet_set_condition(0)
        packet_dissect(wifi_pkt, len(wifi_pkt))
        s = packet_summary()
        # print(s)
        # dictionary_haha.clear()
        packet_navigate(1,0,test)
        f= packet_read_condition(0)
        # print("THis is f",f)
        if f == 1:
            pkt_lst.append(key)
        # testit_dic = dictionary_haha.copy()
        # dictionary_haha.clear()
        # print(packet_summary())
        # print("111111:",testit_dic)
        # print("222222",dictionary_haha)
        # final_dic.append(dictionary_haha)
        # final_dic[key]=testit_dic
    # f = open("dictionary.txt", "w")
    # f.write(str(fieldname_value_dic))
    # print("This is the full dic",final_dic)
    return pkt_lst

# get all the comination of the filter
def get_combin(list_name,number):
    s = list(list_name)
    return list(combinations(s,number))

# create the dic with the key of the statename, the value of all the combination of the common field and value.
def generate_all_combination_of_filter(dic,n_of_filter):
    # initial_filter_dic = {}
    # final_filter_dic = {}
    # for key1,value1 in dic.items():
    initial_filter_lst =[]
    for key2,value2 in dic.items():
        # Filter = target_layer_name+"."+key2+"=="+value2
        Filter = key2 + "=="+str(value2)
        # print("this is filter",Filter)
        # including the just the name of the field, can disable this if the filter list is too long to get the combination
        initial_filter_lst.append(key2)
        initial_filter_lst.append(Filter)
    # for key3,value3 in initial_filter_dic.items():
    final_filter_lst = []
    combination_lst = get_combin(initial_filter_lst,n_of_filter)
    # print("This is the combination lst",combination_lst)
    # for x in get_combin(initial_filter_lst,n_of_filter):
    for x in combination_lst:
        final_filter_lst.append("&&".join(x))
    # print("This is the initial filter lst:",initial_filter_lst)
    # print("This is the final filter lst:",final_filter_lst)
    return final_filter_lst

# add '' to the value, used for the excutation of command
def change_to_string(value):
    return "'"+str(value)+"'"

# copy the generated configuration file to the config folder of the correct dirctory
def cp_to_config(dest_directory,target_name):
    os.system("cp test.json "+dest_directory+"/"+target_name)

# generate the png for the state mapper
def generate_stateMapper():
    cap_location = input(Fore.BLUE+"Please enter the directory of the capture file including the file name : ")
    config_location = input(Fore.BLUE+"Please enter the directory of the config file including the file name : ")
    target_name = input(Fore.BLUE+"Please enter the target name of the output file : ")
    command = "sudo ../../bin/wdmapper "+"-i "+cap_location+" -c "+config_location+" -o "+target_name
    print(command)
    os.system(command)

# using the wdmapper function convert the hex value to string
def value_to_string(field_name, integer_value):
       hfi = packet_get_header_info(field_name)
       if hfi != ctypes.c_void_p:
              str_val = packet_read_value_to_string(integer_value, hfi)
              if str_val:
                     return str_val.decode()
       return None

def get_filter(rel_filter,pkt_lst, filter_dic, ful_dic,state_name,c,stte_dic):
    empty_or_not = False
    # print("This is the full dic",ful_dic)
    # This real_lst contains the dic for all the packet which in the interested list
    # real_lst = []
    flag = False
    # for i in pkt_lst:
        # real_lst.append(ful_dic[i])
    # print("This is real_lst",real_lst)
    # print("This is filter dic",filter_dic)
    i = len(list(filter_dic))-1
    if len(list(filter_dic))==0:
        print("Something wrong with the input packet list, ending the process: ")
    else:
        # now the fillter hunter will hunt for filter form the bottom layer then go up and stop at the layer before the frame (layer 2)
        while (i>=0 and flag==False):
            final_filter_lst = generate_all_combination_of_filter(filter_dic[i],1)
            # print("This is the final filter lst_1",final_filter_lst)
            # for key, value in filter_dic.items():
            print(Fore.BLUE+"Start to check each filter one by one")
            for fil in final_filter_lst:
                if 'length' in fil or 'extension_bit' in fil or 'optional_bit' in fil:
                    print('Ignore Useless filter \n')
                    pass
                else:
                    # tem_lst contains the dictionary for all the packet after the filter is applied
                    tem_lst = []
                    # potential_filter = target_layer_name+"."+key
                    # if value_to_string(potential_filter,int(value,16))!=None:
                    # fil = target_layer_name+"."+key+"=="+str(value)
                    print(Fore.GREEN+"This is the filter: ", fil)
                    tem_pkt_lst = test_filter(fil,c)
                    # print("THis is teh tem_pkt_lst",tem_pkt_lst)
                    if tem_pkt_lst == pkt_lst:
                        rel_filter[state_name] = fil
                        flag = True
                        print(Fore.RED+"got it",fil)
                        break
                    else:
                        pass
            if flag == False:
                print(Fore.BLUE+"start chekc the combination of the two filters ")
                final_filter_lst = generate_all_combination_of_filter(filter_dic[i],2)
                # print("This is the final filter lst_2",final_filter_lst)
                for fil in final_filter_lst:
                    if 'length' in fil or 'extension_bit' in fil or 'optional_field_bit' in fil:
                        print('Ignore Useless filter \n')
                        pass
                    else:
                        print(Fore.GREEN+"This is the filter: ", fil)
                        # tem_lst contains the dictionary for all the packet after the filter is applied
                        tem_lst = []
                        # potential_filter = target_layer_name+"."+key
                        # if value_to_string(potential_filter,int(value,16))!=None:
                        # fil = target_layer_name+"."+key+"=="+str(value)
                        # print("This is the filter: ", fil)
                        tem_pkt_lst = test_filter(fil,c)
                        if tem_pkt_lst == pkt_lst:
                            rel_filter[state_name] = fil
                            flag = True
                            print(Fore.RED+"got it",fil)
                            break
                        else:
                            pass
            i = i-1
        if flag == False:
            print(Fore.BLUE+"Can not find any filter for this state")
            del stte_dic[state_name]

    # return rel_filter


# convert the dic to josn format
def write_json(new_data):
    with open('test.json', 'r+') as file:
        # First we load existing data into a dict.
        file_data = json.load(file)
        # Join new_data with file_data inside emp_details
        file_data["config"]["StateMapper"]["Mapping"].append(new_data)
        # Sets file's current position at offset.
        file.seek(0)
        # convert back to json.
        json.dump(file_data, file, indent=4)

# create a list which contains the interested packet number based on the input from the user
def lst_packet():
    mode = input(Fore.RED+"Do you want to create the lst by range , sequence number, list (r/s/l): ")
    pkt_lst = []
    if mode == 's':
        pkt = input(Fore.RED+"Enter the packet sequence number (enter 0 to exit): ")
        while pkt != '0':
            pkt_lst.append(int(pkt))
            pkt = input(Fore.RED+"Enter the packet sequence number (enter 0 to exit): ")
        print(Fore.GREEN+"This is the packet list: ", pkt_lst)
    elif mode == 'r':
        end_flag = 'y'
        while end_flag == 'y':
            pkt_range = input(Fore.RED+"Enter the packet range format(xx-xx): ")
            lst = pkt_range.split('-')
            for i in range(int(lst[0]), int(lst[1])+1):
                pkt_lst.append(i)
            pkt_lst.sort()
            print(Fore.GREEN+"This is the packet list: ", pkt_lst)
            end_flag = input(Fore.RED+"Do you want to continuesly input packet? y/n : ")
    elif mode=='l':
        end_flag = 'y'
        while end_flag == 'y':
            targeted_lst = input(Fore.RED+"Enter the target packet list format (1,2,3,4,5): ").split(',')
            for i in targeted_lst:
                pkt_lst.append(int(i))
            pkt_lst.sort()
            print(Fore.GREEN+"This is the packet list: ", pkt_lst)
            end_flag = input(Fore.RED+"Do you want to continuesly input packet? y/n : ")
    return pkt_lst

# allow the user to create their own state
def create_state(cap_dic,d,finalFileName):
    # flag = input("Do you want to generate basic states automatically (y/n) ?")
    rel_filter = {}
    tim = time.localtime()
    current_time = time.strftime("%H:%M:%S", tim)
    # os.system("cp bthost_mqtt_config.json test.json")
    # os.system("cp wifi_templete_config.json test.json")
    # os.system("cp zigbee_config_templete.json test.json")
    # os.system("cp bthost_config.json test.json")
    os.system("cp 5gnr_gnb_templete_config.json test.json")


    # create a state dictionary for all new state with the key as the state name and value as the filter list
    state_dic = {}
    flg = input(Fore.RED+"Do you want to create a new state? (y/n): ")
    while flg == 'y':
        pkt_lst = lst_packet()
        fil_dic_dic = filter_dic(pkt_lst, cap_dic)
        state_name = input(Fore.RED+"Enter the name of the state: ")
        state_dic[state_name] = fil_dic_dic[0]
        # The error maybe because of the encodeing of the string
        # get_filter(rel_filter,pkt_lst,fil_dic,cap_dic,'capture_wifi_ap_eap_peap.pcapng',target_layer_name,state_name,d)
        # get_filter(rel_filter,pkt_lst,fil_dic,cap_dic,'zigbee.pcapng',target_layer_name,state_name,d)
        # get_filter(rel_filter,pkt_lst,fil_dic_dic,cap_dic,finalFileName,state_name,d)
        # print("This is the state_dic: ", state_dic)
        get_filter(rel_filter,pkt_lst,fil_dic_dic,cap_dic,state_name,d,state_dic)
        # print("This is the state_dic: ",state_dic)

        # get_filter(rel_filter,pkt_lst,fil_dic,cap_dic,'mqtt_packets.pcapng',target_layer_name,state_name)
        # print("This is the real filter",rel_filter)
        flg = input(Fore.RED+"Do you want to create a new state? (y/n): ")
    # transfer the first element of the filter list to the json format.
    for key, value in state_dic.items():
        for key1, value1 in value.items():
            # create the filter dictionary for each state
            jsn_dic = {"AppendSummary": False}
            # jsn_dic["Filter"] = target_layer_name+"."+key1+"=="+str(value1)
            jsn_dic["Filter"] = rel_filter[key]
            jsn_dic["LayerName"] = key
            # jsn_dic["StateNameField"]= jsn_dic["Filter"]
            for key2,value2 in value.items():
                # print("This is key2",key2)
                state_name_field = key2
                # print("This is value 2",value2)
                if value_to_string(state_name_field,value2)!=None:
                    print(Fore.GREEN+"got statename field")
                    jsn_dic["StateNameField"] = state_name_field
                    break
            if "StateNameField" not in jsn_dic:
                jsn_dic["StateNameField"]=key1
            print(Fore.GREEN+"This is the jsn_dic", jsn_dic)
            time.sleep(0.2)
            write_json(jsn_dic)
            jsn_dic = {}
            break
        # else:
        #     continue
    targetFileName = "test.json"
    # cp_to_config("/home/asset/Desktop/work/wireless-deep-fuzzer-wdissector-zigbee/configs",targetFileName)
    os.system("mv test.json "+str(current_time)+".json")
    # print("This is the filter dictionary: ", state_dic)
    return state_dic
# This function will output the list of target layer for later filter inspection (start from layer number 3)
def find_target_layer_name(pkt_lst,cap_dic):
    # This list contains the length of the target packet's dictonary which indicates the number of layers of that packet
    # tgt_pkt_len=[]
    target_layer_lst = []
    tgt_pkt_len_dic = {}
    for pkt in pkt_lst:
        lenth = len(list(cap_dic[pkt]))
        # print("This is the number of layers of pkt: ",pkt ," ", lenth)
        tgt_pkt_len_dic[pkt]=lenth
        # tgt_pkt_len.append(lenth)
    # shortess_pkt_len = min(tgt_pkt_len)
    # print("this is the tgt dic: ",tgt_pkt_len_dic)
    # This line will output the pkt number which has the minimum number of layers
    pkt_stest = min(tgt_pkt_len_dic,key = tgt_pkt_len_dic.get)
    # print("This is the target pkt: ", pkt_stest)
    tgt_pkt_lst = list(cap_dic[pkt_stest])
    # change here to 1 means the packet need to have minimum two layers
    if (tgt_pkt_len_dic[pkt_stest])<1:
        print("There are not enough layers, something wrong with the packet input")
    else:
        # the target_layer_lst will append any layers from layer 2 (transportation layer) whcih means it will skip the frame layer
        for i in range(1,(tgt_pkt_len_dic[pkt_stest])):
            target_layer_lst.append(tgt_pkt_lst[i])
    print("This is the target layer name list: ", target_layer_lst)
    return target_layer_lst

# This function will generate the filter list for the state by refering to the pkt list and the pkt dictionary
def filter_dic(pkt_lst, cap_dic):
    # the user will specify the target layer name for further inspection
    final_target = {}
    tt_layer_lst = find_target_layer_name(pkt_lst,cap_dic)
    # print("This is the number of layer list: ", tt_lst)
    if len(tt_layer_lst)==0:
        print("There is no target layer, ending the process")
    else:
        x=len(tt_layer_lst)-1
        while x >= 0:
            # targer_layer_name = input(Fore.RED+"Enter the target layer name: ")
            targer_layer_name = tt_layer_lst[x]
            print("This is the target layer name",targer_layer_name)
            # this target_lst will contain the dictionary of the target layer of each packet in the packet lsit
            target_lst = []
            # this target will store all the comman key value pair with all the packet in the pkt_lst
            target = {}
            tempory_target = {}
            for pkt in pkt_lst:
                target_lst.append(cap_dic[pkt][targer_layer_name])
            #  if the user wanna assign a new state to a individual packet
            if len(pkt_lst) == 1:
                target = target_lst[0]
            else:
                for item in target_lst[0].items():
                    # print("This is the item",item)
                    if item in target_lst[1].items():
                        target[item[0]] = item[1]
                for i in range(len(target_lst)-2):
                    for item in target_lst[i+2].items():
                        if item in target.items():
                            tempory_target[item[0]] = item[1]
                    target = tempory_target
            final_target[x]=target
            # print("this is the final target dic lst: ",final_target)
            x= x-1

    return final_target

def multi_cap(file1,file2,Out_file):
    # flag_combine= input(Fore.RED+"Enter the file name of the capture file: ")
    cmd = "mergecap -w "+Out_file+" "+file1+" "+file2
    os.system(cmd)
    # flag = input(Fore.RED+"Do you want to inport multiple capture file?(y/n): ")
    # if flag == 'y':
    #     pass
            # if "StateNameField" not in jsn_dic:
            #     jsn_dic["StateNameField"]=key1
if __name__ == '__main__':
    # c = pyshark.FileCapture('capture_wifi_ap_eap_peap.pcapng')
    # c = pyshark.FileCapture('mqtt_packets.pcapng')
    # c = pyshark.FileCapture('mqtt_packets.pcapng',include_raw=True, use_json=True)
    # c = pyshark.FileCapture('capture_wifi_ap_eap_peap.pcapng',include_raw=True, use_json=True)
    flag_multi_cap = input(Fore.RED+"Do you have multiple capture file? (y/n) ")
    while flag_multi_cap == 'y':
        file_name_1 = input(Fore.YELLOW+"Please enter the name of the first file: ")
        file_name_2 = input(Fore.YELLOW+"Please enter the name of the second file: ")
        file_name_out = input(Fore.YELLOW+"Please enter the name of the output file: ")
        multi_cap(file_name_1,file_name_2,file_name_out)
        flag_multi_cap = input(Fore.RED+"Do you have multiple capture file? (y/n) ")
    final_file_name = input(Fore.GREEN+"Please enter the final file name ")
    # c = pyshark.FileCapture('zigbee.pcapng',include_raw=True, use_json=True)
    c = pyshark.FileCapture(final_file_name,include_raw=True, use_json=True)
    # print(colored("hello",'green'))
    encap_type = get_encap_type(c)
    # print("This is encap_type: ", encap_type)
    fuzzing_5g = input(Fore.RED+"Are you fuzzing 5G? (y/n) ")
    if fuzzing_5g=="y":
        init = "encap:1"
    else:
        init = "proto:"+encap_type
    # init = "proto:"+encap_type
    # init = "encap:25"
    # init = "proto:mac-nr-framed"
    # init = "encap:1"
    # print(init)
    # print("\033[1;37;40m This is the encapsulation type {init}\033[0;37;40m \n")
    print(Fore.YELLOW+"This is the encapsulation type",init)
    # wdissector_init(init)
    print(wdissector_init(init))
    # wdissector_init("encap:104")
    wdissector_enable_fast_full_dissection(1)
    print(Fore.YELLOW+"Version: " + wdissector_version_info().decode())
    print(Fore.YELLOW+"Loaded Profile: " + wdissector_profile_info().decode())
    cap_dic,pkt_summ_dic = get_dic(c)
    auto_gen_state_dic(pkt_summ_dic)
    create_state(cap_dic,c,final_file_name)
    generate_stateMapper()