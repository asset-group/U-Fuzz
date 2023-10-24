#!/usr/bin/env python3

import pandas as pd
import numpy as np
import pcapng
from pcapng import FileScanner
from pcapng.blocks import EnhancedPacket


def analyse_log(capture_file, output_file, max_iterations=1000, save_all_iterations=False, show_output=True):
    save_to_file = output_file is not None

    last_iteration = 0
    count_iteration = 0
    count_crashes = 0
    count_unique_crashes = 0
    count_deadlocks = 0
    count_anomalies = 0
    count_unique_anomalies = 0
    start_timestamp = 0
    current_timestamp = 0
    anomaly_states = []
    crashes_states = []
    deadlock_states = []
    iterations = []
    if not save_all_iterations:
        iterations = []
    else:
        iterations = np.zeros((max_iterations, 7), dtype=int)

    def update_stats(iterations):

        if not save_all_iterations:
            if len(iterations) and (iterations[-1][0] == count_iteration):
                # Ovewrite existing iteration info
                iterations[-1][0] = count_iteration
                iterations[-1][1] = count_crashes
                iterations[-1][2] = count_unique_crashes
                iterations[-1][3] = count_deadlocks
                iterations[-1][4] = count_unique_anomalies
                iterations[-1][5] = count_unique_crashes + count_deadlocks
                iterations[-1][6] = current_timestamp
            else:
                # Append iteration info
                iterations.append([count_iteration,
                                   count_crashes,
                                   count_unique_crashes,
                                   count_deadlocks,
                                   count_unique_anomalies,
                                   count_unique_crashes + count_deadlocks,
                                   current_timestamp])
        else:
            iterations[count_iteration - 1][0] = count_iteration
            iterations[count_iteration - 1][1] = count_crashes
            iterations[count_iteration - 1][2] = count_unique_crashes
            iterations[count_iteration - 1][3] = count_deadlocks
            iterations[count_iteration - 1][4] = count_unique_anomalies
            iterations[count_iteration -
                       1][5] = count_unique_crashes + count_deadlocks
            iterations[count_iteration - 1][6] = current_timestamp

    print('Processing pcapng file...')
    with open(capture_file, 'rb') as fp:
        scanner = FileScanner(fp)

        try:
            for pkt in scanner:
                if isinstance(pkt, EnhancedPacket):
                    # Update timestamp
                    if start_timestamp == 0:
                        start_timestamp = int(pkt.timestamp)
                    current_timestamp = int(pkt.timestamp - start_timestamp)
                    if pkt.packet_payload_info[2][4] == 0x0a:
                        comment = pkt.packet_payload_info[2][6:]
                        if b'Started' in comment:
                            if count_iteration and save_all_iterations:
                                update_stats(iterations)

                            last_iteration = count_iteration
                            count_iteration += 1
                            if count_iteration > max_iterations:
                                count_iteration -= 1
                                update_stats(iterations)
                                break

                        elif b'Crash' in comment:
                            count_crashes += 1
                            if comment not in crashes_states:
                                crashes_states.append(comment)
                                count_unique_crashes += 1
                                if not save_all_iterations:
                                    update_stats(iterations)

                        elif b'not responding' in comment:
                            if comment not in deadlock_states:
                                deadlock_states.append(comment)
                                count_deadlocks += 1
                                if not save_all_iterations:
                                    update_stats(iterations)

                        elif not save_all_iterations and count_iteration == 1:
                            # Save first iteration
                            update_stats(iterations)

                    else:
                        comment = pkt.options.get('opt_comment', '')

                        if 'Invalid' in comment:
                            count_anomalies += 1
                            if comment not in anomaly_states:
                                anomaly_states.append(comment)
                                count_unique_anomalies += 1
                                if not save_all_iterations:
                                    update_stats(iterations)
        except pcapng.exceptions.TruncatedFile:
            print('File is truncated')

        if not save_all_iterations and (last_iteration != count_iteration):
            # Save last iteration
            update_stats(iterations)

    csv_frame = pd.DataFrame(iterations,
                             columns=['Iteration', 'T. Crashes', 'Crashes', 'Deadlocks', 'Anomalies', 'Crashes + Deadlocks', 'Timestamp'])

    if show_output:
        print(csv_frame)
    if save_to_file:
        output_file = output_file.split('.')[0] + '.csv'
        csv_frame.to_csv(output_file, index=False)
        print("Done. Saved to " + output_file)

    return csv_frame


if __name__ == '__main__':

    max_iterations = 1000
    print("eval_1")
    eval_1 = analyse_log("eval_1/capture_bluetooth.pcapng",
                         "eval_1.csv", max_iterations)
    print("eval_5")
    eval_5 = analyse_log("eval_5/capture_bluetooth.pcapng",
                         "eval_5.csv", max_iterations)
    # print("eval_10")
    # eval_10 = analyse_log("eval_10/capture_bluetooth.pcapng", "eval_10.csv", max_iterations)
    print("eval_15")
    eval_15 = analyse_log("eval_15/capture_bluetooth.pcapng",
                          "eval_15.csv", max_iterations)
    print("eval_30")
    eval_30 = analyse_log("eval_30/capture_bluetooth.pcapng",
                          "eval_30.csv", max_iterations)
    print("eval_45")
    eval_45 = analyse_log("eval_45/capture_bluetooth.pcapng",
                          "eval_45.csv", max_iterations)
    print("eval_60")
    eval_60 = analyse_log("eval_60/capture_bluetooth.pcapng",
                          "eval_60.csv", max_iterations)
