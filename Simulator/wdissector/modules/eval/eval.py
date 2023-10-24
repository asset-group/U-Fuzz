#!/usr/bin/env python3

import sys
import os
import time
import json
import glob
from analyse_log import analyse_log
from cov import get_cov


WDMAPPER_PATH = '../../bin/'
CONFIG_PATH = '../../configs/bt_config.json'
REF_MODEL_PATH = '../../configs/models/bt/sdp_rfcomm_query.json'
MAX_ITERATIONS = 1000


# calculate standard deviation of array of numbers
def get_std_dev(numbers):
    mean = sum(numbers) / len(numbers)
    return (sum([(x - mean) ** 2 for x in numbers]) / len(numbers)) ** 0.5


# match string 'Iteration:' in file and return the number of iterations
def get_fitness_stats(folder_path):
    iterations = 0
    acc_fitness = 0
    fitness = []
    # List all files that starts with events in the folder
    event_files = glob.glob(folder_path + '/events.*')

    for file_name in event_files:
        with open(file_name, 'rb') as f:
            for line in f:
                if b'Fitness=' in line and b'Recovering' not in line:
                    ft = float(line.decode().split('=')[1].split(' ')[0])
                    if ft < 0:  # In case of recovering related error
                        continue
                    if ft == 1000000:
                        break
                    fitness.append(ft)
                    acc_fitness += ft
                    iterations += 1

    if len(fitness) == 0:
        return 0, 0

    # return a tuple of average fitness and standard deviation
    return (acc_fitness / iterations, get_std_dev(fitness))


def get_states_count(log_path):
    # List all files that starts with events in the folder
    with open(log_path) as f:
        extract_next_lmp_line = False
        extract_next_non_lmp_line = False

        lmp_states = -1  # exclude default idle state
        non_lmp_states = 0
        for line in f:
            if 'LMP' in line:
                extract_next_lmp_line = True
            elif 'Layer:' in line:
                extract_next_non_lmp_line = True

            elif extract_next_non_lmp_line == True:
                extract_next_non_lmp_line = False
                non_lmp_states += int(line.split(':')[1].split(',')[0])

            elif extract_next_lmp_line == True:
                extract_next_lmp_line = False
                lmp_states += int(line.split(':')[1].split(',')[0])

    return lmp_states + non_lmp_states, lmp_states, non_lmp_states


def gen_model(eval_name, capture_file, show_output=True, output_name='wdmapper.txt', model_name=None):
    # Generate ref json model for coverage analysis
    print('Generating ref json model for ' + capture_file + ' ...')
    os.system('ln -sf ' + WDMAPPER_PATH + ' bin')

    if model_name is None:
        model_name = eval_name

    wdmapper_cmd = WDMAPPER_PATH + 'wdmapper' + \
        ' -c ' + CONFIG_PATH + \
        ' -i ' + capture_file + \
        ' -o ' + model_name
    print(wdmapper_cmd + '.json')

    if show_output:
        os.system('unbuffer ' + wdmapper_cmd +
                  '.json | tee ' + eval_name + '/' + output_name)
    else:
        os.system('unbuffer ' + wdmapper_cmd +
                  '.json > ' + eval_name + '/' + output_name)
    # Remove colors from saved wdmapper.txt
    os.system('sed -i \'s/\x1b\[[0-9;]*[mK]//g\' ' +
              eval_name + '/' + output_name)


def eval(eval_name, calc_coverage=True, print_results=True, print_model_stats=False,
         ref_model_path=REF_MODEL_PATH, calculate_fitness=True, mapper_output=None, show_output=True):

    coverage = 0
    target_model_transitions = 0

    capture_file = eval_name + '/capture_bluetooth.pcapng'

    # Analyse capture file and return number of crashes and anomalies
    print('Reading capture file \'' + capture_file + '\' ...')
    iterations = analyse_log(capture_file, eval_name +
                             '.csv', MAX_ITERATIONS,
                             show_output=show_output)

    gen_model(eval_name, capture_file, show_output)

    if calc_coverage:
        # Calculate coverage using default reference model
        print('Calculating coverage for ' + eval_name + '.json ...')
        coverage, target_model_transitions = get_cov(
            ref_model_path, eval_name + '.json', True)

    # Calculate total time
    timestamp_array = iterations['Timestamp']
    total_time = timestamp_array[len(timestamp_array) - 1]
    time_formated = time.strftime('%H h. %M min.', time.gmtime(total_time))

    # Calculate time to first crash or deadlock
    vuln_arrays = iterations['Crashes + Deadlocks']
    time_to_vuln = 0
    i = 0
    for vuln in vuln_arrays:
        if vuln > 0:
            time_to_vuln = timestamp_array[i]
            break
        i += 1
    if time_to_vuln >= 60:
        time_vuln_formated = time.strftime(
            '%H h. %M min.', time.gmtime(time_to_vuln))
    elif len(vuln_arrays) > 0:
        time_vuln_formated = '< 1 min.'
    else:
        time_vuln_formated = 'N.A (No Vulnerability Found)'

    # Calculate time to first anomaly
    anoamly_arrays = iterations['Anomalies']
    time_to_anomaly = 0
    i = 0
    for anomaly in anoamly_arrays:
        if anomaly > 0:
            time_to_anomaly = timestamp_array[i]
            break
        i += 1
    if time_to_anomaly >= 60:
        time_anomaly_formated = time.strftime(
            '%H h. %M min.', time.gmtime(time_to_anomaly))
    elif len(anoamly_arrays) > 0:
        time_anomaly_formated = '< 1 min.'
    else:
        time_anomaly_formated = 'N.A (No Anomalies Found)'

    # Evaluation summary
    iterr_array = iterations['Iteration']
    max_iterations = iterr_array[len(iterr_array) - 1]
    crashes = vuln_arrays[len(vuln_arrays) - 1]
    anomalies = anoamly_arrays[len(anoamly_arrays) - 1]

    f = open(CONFIG_PATH)
    cfg = json.load(f)
    rsel = cfg['config']['Fuzzing']['DefaultDuplicationProbability']
    dt = cfg['config']['Fuzzing']['MaxDuplicationTime']

    # Analyse fitness of each iteration
    if calculate_fitness:
        fitness_stats = get_fitness_stats(eval_name)

    if mapper_output is None:
        mapper_output = eval_name + '/wdmapper.txt'

    states_count, lmp_states, non_lmp_states = get_states_count(mapper_output)

    if print_results:
        print('------------------- Timing and Coverage --------------------')
        print('Total Time: ' + time_formated)
        print('1st Vulnerability: ' + time_vuln_formated)
        print('1st Non-compl.: ' + time_anomaly_formated)
        print('Model Coverage: %d (%s)%%' %
              (target_model_transitions, coverage))
        print('-------------------- Evaluation Summary --------------------')
        print('Iterations = ' + str(max_iterations))
        print('Rsel = ' + str(rsel))
        print('Dt = ' + str(dt) + 'ms')
        print('Crashes (C) = ' + str(crashes))
        if calculate_fitness:
            print('Average Transitions (Std. Dev.): %d (%d)' %
                  (fitness_stats[0], fitness_stats[1]))
        print('Anomalies (A) = ' + str(anomalies))

    if print_model_stats:
        print('------------------------ Model Stats -----------------------')
        print('Total States:' + str(states_count))
        print('LMP States:' + str(lmp_states))
        print('Non-LMP States:' + str(non_lmp_states))

    return (max_iterations, crashes, anomalies, states_count, lmp_states, non_lmp_states, coverage)


if __name__ == '__main__':

    if len(sys.argv) < 2:
        print('Usage: python3 eval.py <log folder name>')
        sys.exit(1)

    script_path = sys.path[0]
    os.chdir(script_path)

    eval_name = sys.argv[1]

    eval(eval_name, calculate_fitness=True)

    sys.exit(0)
