# import time
# import datetime
# import matplotlib.pyplot as plt
# import numpy as np

# lst_ours = ["2023-09-22 17:39:34", "2023-09-22 21:02:13", "2023-09-22 21:53:41", "2023-09-22 01:35:28",
#             "2023-09-22 01:42:48", "2023-09-22 09:38:17", "2023-09-22 09:41:58", "2023-09-22 09:45:57", "2023-09-22 09:52:19",
#             "2023-09-22 09:56:01", "2023-09-22 10:01:45", "2023-09-22 10:07:09", "2023-09-22 10:10:49",	"2023-09-22 10:15:53",
#             "2023-09-22 10:26:34", "2023-09-22 10:31:37", "2023-09-22 10:39:54", "2023-09-22 10:45:10",	"2023-09-22 10:48:42",
#             "2023-09-22 10:53:45", "2023-09-22 10:57:21", "2023-09-22 11:00:52", "2023-09-22 11:04:55",	"2023-09-22 11:11:48",
#             "2023-09-22 11:22:00", "2023-09-22 11:25:35", "2023-09-22 11:29:11", "2023-09-22 11:32:45",	"2023-09-22 11:39:43",
#             "2023-09-22 11:43:39", "2023-09-22 11:47:15", "2023-09-22 11:50:54", "2023-09-22 11:56:10",	"2023-09-22 11:59:53",
#             "2023-09-22 12:03:36", "2023-09-22 12:07:11", "2023-09-22 12:10:52", "2023-09-22 12:14:28",	"2023-09-22 12:18:05",
#             "2023-09-22 12:21:41", "2023-09-22 12:25:16", "2023-09-22 12:29:28", "2023-09-22 12:36:26",	"2023-09-22 12:40:05",
#             "2023-09-22 12:57:06", "2023-09-22 13:00:47", "2023-09-22 13:04:21", "2023-09-22 13:17:59",	"2023-09-22 13:33:39",
#             "2023-09-22 13:43:52", "2023-09-22 13:57:30", "2023-09-22 14:14:26", "2023-09-22 14:28:01",	"2023-09-22 14:35:47",	
#             "2023-09-22 14:52:41", "2023-09-22 15:02:23", "2023-09-22 15:07:24", "2023-09-22 15:13:01",	"2023-09-22 15:17:20",
#             "2023-09-22 15:21:04", "2023-09-22 15:24:51", "2023-09-22 15:28:27", "2023-09-23 06:36:08"]
# lst_thesis = ["2023-09-22 22:03:33"]

# def cvt_timestamp_lst(lst):
#     lst_ts= []
#     lst_num = []
#     for i,ele in enumerate(lst):
#         lst_ts.append(str(time.mktime(datetime.datetime.strptime(ele,"%Y-%m-%d %H:%M:%S").timetuple())))
#         lst_num.append(i+1)
#     # print(lst_ts)
#     return lst_ts,lst_num
# # cvt_timestamp_lst(lst_ours)
# # print(time.mktime(datetime.datetime.strptime("21-09-2023 22:03:33","%d-%m-%Y %H:%M:%S").timetuple()))
# lst_ts_ours,lst_num_ours = cvt_timestamp_lst(lst_ours)
# lst_ts_thesis, lst_num_thesis = cvt_timestamp_lst(lst_thesis)
# plt.plot(lst_ts_ours,lst_num_ours)
# plt.plot(lst_ts_thesis,lst_num_thesis)
# plt.show()

import numpy as np
import matplotlib.pyplot as plt
 
# set width of bar
barWidth = 0.35
fig = plt.subplots(figsize =(12, 8))
 
# set height of bar [libcoap unique, libcoap total, canopus unique, canopus total]
theirs = [0,0,2,102]
ours = [1,31,8,121]
# CSE = [29, 3, 24, 25, 17]
 
# Set position of bar on X axis
br1 = np.arange(len(theirs))
br2 = [x + barWidth for x in br1]
# br3 = [x + barWidth for x in br2]

# Make the plot
plt.bar(br1[0], theirs[0], color ='#edd02a', width = barWidth,
        edgecolor ='grey', label ='Liljedahl Master Thesis libcoap Unique')
plt.bar(br2[0], ours[0], color ='#0b5394', width = barWidth,
        edgecolor ='grey', label ='Universal Fuzzer libcoap Unique')
plt.bar(br1[1], theirs[1], color ='#c9c789', width = barWidth,
        edgecolor ='grey', label ='Liljedahl Master Thesis libcoap Total')
plt.bar(br2[1], ours[1], color ='#570000', width = barWidth,
        edgecolor ='grey', label ='Universal Fuzzer libcoap Total')
plt.bar(br1[2], theirs[2], color ='#dec7d4', width = barWidth,
        edgecolor ='grey', label ='Liljedahl Master Thesis Canopus Unique')
plt.bar(br2[2], ours[2], color ='#1b2329', width = barWidth,
        edgecolor ='grey', label ='Universal Fuzzer Canopus Unique')
plt.bar(br1[3], theirs[3], color ='#7ebaeb', width = barWidth,
        edgecolor ='grey', label ='Liljedahl Master Thesis Canopus Total')
plt.bar(br2[3], ours[3], color ='#435420', width = barWidth,
        edgecolor ='grey', label ='Universal Fuzzer Canopus Total')
# plt.bar(br3, CSE, color ='b', width = barWidth,
#         edgecolor ='grey', label ='CSE')
# Adding Xticks
plt.xlabel('Category', fontweight ='bold', fontsize = 15)
plt.ylabel('Number of crashes', fontweight ='bold', fontsize = 15)
plt.xticks([barWidth/2,1+barWidth/2,2+barWidth/2,3+barWidth/2],
        ["Thesis Libcoap Unique", "Thesis Libcoap Total", "Universal Libcoap Unique", "Universal Libcoap Total"])
 
plt.legend()
# plt.xlim(-0.1,1.5,1)
plt.text(0.015,0.5,"0",fontsize=12,fontweight="bold",color = "r")
plt.text(0.325,1.5,"1",fontsize=12,fontweight="bold",color = "r")
plt.text(0.995,0.5,"0",fontsize=12,fontweight="bold",color = "r")
plt.text(1.315,31.5,"31",fontsize=12,fontweight="bold",color = "r")
plt.text(1.985,2.5,"2",fontsize=12,fontweight="bold",color = "r")
plt.text(2.319,8.5,"8",fontsize=12,fontweight="bold",color = "r")
plt.text(2.94,102.5,"102",fontsize=12,fontweight="bold",color = "r")
plt.text(3.29,121.5,"121",fontsize=12,fontweight="bold",color = "r")

plt.show()