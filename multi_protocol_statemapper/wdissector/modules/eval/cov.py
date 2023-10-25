#!/usr/bin/env python3.7

import json

from numpy import r_


def get_cov(ref_name, target_name, print_result=False):
    ref_file = open(ref_name, "r")
    target_file = open(target_name, "r")
    ref_model = json.load(ref_file)
    target_model = json.load(target_file)

    ref_states = ref_model["StatesNodes"]
    ref_transitions = ref_model["StatesTransitions"]
    target_transitions = target_model["StatesTransitions"]
    target_states = target_model["StatesNodes"]

    count_valid_from_transitions = 0
    count_valid_to_transitions = 0
    count_ref_transitions = 0
    count_target_transitions = 0

    # Count number of transitions in target model
    for t_state in target_transitions:
        t_from = target_transitions[t_state]["from_state"]
        if t_from:
            count_target_transitions += len(t_from)

    # For each state in ref model
    for r_state in ref_transitions:
        # Counter number of transitions in ref model
        r_from = ref_transitions[r_state]["from_state"]
        if r_from:
            count_ref_transitions += len(r_from)

        # for each state in target model
        for t_state in target_transitions:
            # Counter number of transitions in target model
            t_from = target_transitions[t_state]["from_state"]

            if t_state == r_state:
                # State exists in both, count transitions
                if r_from and t_from:  # check for null
                    # iterate on target transition and count their existence in ref
                    for t_transition in t_from:
                        if t_transition in r_from:
                            count_valid_from_transitions += 1

    ref_file.close()
    target_file.close()
    coverage = (count_valid_from_transitions / count_ref_transitions) * 100.0

    if print_result:
        print("Reference: %s, Target: %s" % (ref_name, target_name))
        print(
            "States in ref: {}\n"
            "Transitions in ref: {}\n"
            "States in target: {}\n"
            # "Transitions in target: {}\n"
            "Total valid transition of target in ref: {}\n"
            "--> Coverage of target in ref: {}/{} ({:.1f}%)".format(
                len(ref_states),
                count_ref_transitions,
                len(target_states),
                # count_target_transitions,
                count_valid_from_transitions,
                count_valid_from_transitions,
                count_ref_transitions,
                coverage,
            )
        )
        print("-" * 60)

    return str("{:.1f}").format(coverage), count_valid_from_transitions


if __name__ == "__main__":
    get_cov("cov/1_ref.json", "cov/1.json", True)
    get_cov("cov/15_ref.json", "cov/15.json", True)
    get_cov("cov/30_ref.json", "cov/30.json", True)
    get_cov("cov/45_ref.json", "cov/45.json", True)
    get_cov("cov/60_ref.json", "cov/60.json", True)
    get_cov("bthost_ref.json", "bthost.json", True)
