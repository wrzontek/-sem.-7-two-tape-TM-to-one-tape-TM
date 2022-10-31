#include <cstdlib>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <algorithm>
#include "turing_machine.h"

#define BLANK "_"
#define INITIAL_STATE "(start)"
#define ACCEPTING_STATE "(accept)"
#define REJECTING_STATE "(reject)"

// for enriching letters
#define NOTHING_SPECIAL '0'
#define IS_HEAD '1'
#define GO_LEFT '2'
#define GO_RIGHT '3'
#define GO_STAY '4'


using namespace std;

// https://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c
std::string random_bracketized_string(size_t length) {
    auto randchar = []() -> char {
        const char charset[] =
                "0123456789"
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[rand() % max_index];
    };
    std::string str(length, 0);
    std::generate_n(str.begin(), length, randchar);
    return "(" + str + ")";
}

char letter_enrichment[5] = {NOTHING_SPECIAL, IS_HEAD, GO_LEFT, GO_RIGHT, GO_STAY};
char letter_enrichment_no_mark[4] = {NOTHING_SPECIAL, GO_LEFT, GO_RIGHT, GO_STAY};
char letter_enrichment_directions[3] = {GO_LEFT, GO_RIGHT, GO_STAY};
char letter_enrichment_no_directions[2] = {NOTHING_SPECIAL, IS_HEAD};
// ((id)1) for example

vector<string> vec(const string &letter) {
    return {1, letter};
}

string bracketize(const string &s) {
    return "(" + s + ")";
}

char dir_to_enrichment(string dir) {
    if (dir.length() != 1) {
        throw std::runtime_error("dir_to_enrichment called with illegal argument");
    }
    if (dir.at(0) == HEAD_LEFT) {
        return GO_LEFT;
    }
    if (dir.at(0) == HEAD_RIGHT) {
        return GO_RIGHT;
    }
    if (dir.at(0) == HEAD_STAY) {
        return GO_STAY;
    }
    throw std::runtime_error("dir_to_enrichment called with illegal argument");
}

string enrichment_to_dir(char enrichment) {
    if (enrichment == GO_LEFT) {
        return string(1, HEAD_LEFT);
    }
    if (enrichment == GO_RIGHT) {
        return string(1, HEAD_RIGHT);
    }
    if (enrichment == GO_STAY) {
        return string(1, HEAD_STAY);
    }
    throw std::runtime_error("enrichment_to_dir called with illegal argument");
}

string separator = random_bracketized_string(15);
string HASH = random_bracketized_string(25);
string create_state_1 = random_bracketized_string(25);
string create_state_2 = random_bracketized_string(25);
string create_state_3 = random_bracketized_string(25);
string return_state = random_bracketized_string(25);
string return_from_start_1 = random_bracketized_string(25);
string return_from_start_2 = random_bracketized_string(25);
string mark_return_state = random_bracketized_string(25);
string extend_state = random_bracketized_string(25);
string move_state = random_bracketized_string(25);

// more readable but more likely to collide
//    string separator = "-";
//    string HASH = "(HASH)";
//    string create_state_1 = "(create_state_1)";
//    string create_state_2 = "(create_state_2)";
//    string create_state_3 = "(create_state_3)";
//    string return_state = "(return_state)";
//    string return_from_start_1 = "(return_from_start_1)";
//    string return_from_start_2 = "(return_from_start_2)";
//    string mark_return_state = "(mark_return_state)";
//    string extend_state = "(extend_state)";
//    string move_state = "(move_state)";

string enrich(const string &s, char en) {
    return bracketize(bracketize(s) + en);
}

string merge(const string &a, const string &b) {
    return bracketize(bracketize(a) + separator + bracketize(b));
}

string merge(const string &a, const string &b, const string &c) {
    return bracketize(bracketize(a) + separator + bracketize(b) + separator + bracketize(c));
}

string merge(const string &a, const string &b, const string &c, const string &d) {
    return bracketize(
            bracketize(a) + separator + bracketize(b) + separator + bracketize(c) + separator + bracketize(d));
}


TuringMachine two_tape_to_one_tape(TuringMachine &two_tape_machine) {
    vector<string> input_alphabet_plus_blank = two_tape_machine.input_alphabet;
    input_alphabet_plus_blank.emplace_back(BLANK);
    transitions_t transitions;

    // special: start
    for (const auto &letter: input_alphabet_plus_blank) {
        auto letter_before = vec(letter);
        auto letter_after = vec(enrich(letter, IS_HEAD));

        transitions[make_pair(INITIAL_STATE, letter_before)] = make_tuple(create_state_1, letter_after, HEAD_RIGHT);
        ///////////////////////////////

        letter_after = vec(enrich(letter, NOTHING_SPECIAL));

        transitions[make_pair(create_state_1, letter_before)] = make_tuple(create_state_1, letter_after, HEAD_RIGHT);
        ///////////////////////////////

        letter_before = vec(BLANK);
        letter_after = vec(HASH);

        transitions[make_pair(create_state_1, letter_before)] = make_tuple(create_state_2, letter_after, HEAD_RIGHT);
        ///////////////////////////////

        letter_before = vec(BLANK);
        letter_after = vec(enrich(BLANK, IS_HEAD));

        transitions[make_pair(create_state_2, letter_before)] = make_tuple(create_state_3, letter_after, HEAD_RIGHT);
        ///////////////////////////////

        transitions[make_pair(create_state_3, letter_before)] = make_tuple(
                merge(return_from_start_1, INITIAL_STATE, BLANK),
                vec(HASH),
                HEAD_LEFT);
    }

    // special: return from start
    {
        // jump one marked blank (we just created it)
        transitions[make_pair(merge(return_from_start_1, INITIAL_STATE, BLANK), vec(enrich(BLANK, IS_HEAD)))] =
                make_tuple(merge(return_from_start_2, INITIAL_STATE, BLANK), vec(enrich(BLANK, IS_HEAD)), HEAD_LEFT);

        for (const auto &letter: two_tape_machine.working_alphabet()) {
            auto letter_before = vec(enrich(letter, NOTHING_SPECIAL));
            transitions[make_pair(merge(return_from_start_2, INITIAL_STATE, BLANK), letter_before)] =
                    make_tuple(merge(return_from_start_2, INITIAL_STATE, BLANK), letter_before, HEAD_LEFT);

            transitions[make_pair(merge(return_from_start_2, INITIAL_STATE, BLANK), vec(HASH))] =
                    make_tuple(merge(return_from_start_2, INITIAL_STATE, BLANK), vec(HASH), HEAD_LEFT);

            letter_before = vec(enrich(letter, IS_HEAD));
            auto letter_after = vec(enrich(letter, NOTHING_SPECIAL));

            transitions[make_pair(merge(return_from_start_2, INITIAL_STATE, BLANK), letter_before)] =
                    make_tuple(merge(INITIAL_STATE, BLANK), letter_after, HEAD_STAY);
        }
    }

    // general case
    for (auto transition: two_tape_machine.transitions) {
        string q1 = transition.first.first;
        if (q1 == ACCEPTING_STATE || q1 == REJECTING_STATE)
            continue;
        string c1 = transition.first.second[0];
        string c2 = transition.first.second[1];
        string q2 = get<0>(transition.second);
        string c1p = get<1>(transition.second)[0];
        string c2p = get<1>(transition.second)[1];
        string d1 = string(1, get<2>(transition.second)[0]);
        string d2 = string(1, get<2>(transition.second)[1]);

        string c1_with_dir = enrich(c1p, dir_to_enrichment(d1));
        {
            auto letter_before = vec(enrich(c1, NOTHING_SPECIAL));
            auto letter_after = vec(c1_with_dir);

            // 1
            transitions[make_pair(merge(q1, c2), letter_before)] =
                    make_tuple(merge(q2, c2p, string(1, dir_to_enrichment(d2))), letter_after, HEAD_RIGHT);
        }
        ///////////////////////////////
        auto state = merge(q2, c2p, string(1, dir_to_enrichment(d2)));
        for (const auto &letter: two_tape_machine.working_alphabet()) {
            auto letter_before = vec(enrich(letter, NOTHING_SPECIAL));
            // 2
            transitions[make_pair(state, letter_before)] =
                    make_tuple(state, letter_before, HEAD_RIGHT);
        }

        // 2 (hash)
        transitions[make_pair(state, vec(HASH))] =
                make_tuple(state, vec(HASH), HEAD_RIGHT);

        ///////////////////////////////
        for (const auto &letter: two_tape_machine.working_alphabet()) {
            auto letter_before = vec(enrich(letter, IS_HEAD));
            auto letter_after = vec(enrich(c2p, NOTHING_SPECIAL));
            // 3
            transitions[make_pair(state, letter_before)] =
                    make_tuple(merge(mark_return_state, q2, c2p, string(1, dir_to_enrichment(d2))), letter_after, d2);
        }

        ///////////////////////////////
        for (const auto &letter_to_mark: two_tape_machine.working_alphabet()) {
            auto letter_before = vec(enrich(letter_to_mark, NOTHING_SPECIAL));
            auto letter_after = vec(enrich(letter_to_mark, IS_HEAD));

            // 4
            transitions[make_pair(merge(mark_return_state, q2, c2p, string(1, dir_to_enrichment(d2))), letter_before)] =
                    make_tuple(merge(return_state, q2, letter_to_mark), letter_after, HEAD_LEFT);


            ///////////////////////////////
            for (const auto &letter: two_tape_machine.working_alphabet()) {
                letter_before = vec(enrich(letter, NOTHING_SPECIAL));

                // 5
                transitions[make_pair(merge(return_state, q2, letter_to_mark), letter_before)] =
                        make_tuple(merge(return_state, q2, letter_to_mark), letter_before, HEAD_LEFT);

                letter_before = vec(enrich(letter, IS_HEAD));

                // 5 (marked, but ignore mark)
                transitions[make_pair(merge(return_state, q2, letter_to_mark), letter_before)] =
                        make_tuple(merge(return_state, q2, letter_to_mark), letter_before, HEAD_LEFT);
            }


            // 5 (hash)
            transitions[make_pair(merge(return_state, q2, letter_to_mark), vec(HASH))] =
                    make_tuple(merge(return_state, q2, letter_to_mark), vec(HASH), HEAD_LEFT);

            ///////////////////////////////
            // 6
            transitions[make_pair(merge(return_state, q2, letter_to_mark), vec(c1_with_dir))] =
                    make_tuple(merge(q2, letter_to_mark), vec(enrich(c1p, NOTHING_SPECIAL)), d1);
        }
    }

    // special: 1st tape no space
    for (const auto &state: two_tape_machine.set_of_states()) {
        for (const auto &letter: two_tape_machine.working_alphabet()) {
            // 0
            transitions[make_pair(merge(state, letter), vec(HASH))] =
                    make_tuple(merge(move_state, state), vec(enrich(BLANK, GO_STAY)), HEAD_RIGHT);

            for (char enrichment1: letter_enrichment_no_directions) {
                string on_tape_letter = enrich(letter, enrichment1);

                // 1
                transitions[make_pair(merge(move_state, state), vec(on_tape_letter))] =
                        make_tuple(merge(move_state, state, on_tape_letter), vec(HASH), HEAD_RIGHT);

                for (const auto &letter2: two_tape_machine.working_alphabet()) {
                    for (char enrichment2: letter_enrichment_no_directions) {
                        string on_tape_letter2 = enrich(letter2, enrichment2);

                        // 2
                        transitions[make_pair(merge(move_state, state, on_tape_letter), vec(on_tape_letter2))] =
                                make_tuple(merge(move_state, state, on_tape_letter2), vec(on_tape_letter), HEAD_RIGHT);
                    }
                }

                // 2 (hash)
                transitions[make_pair(merge(move_state, state, on_tape_letter), vec(HASH))] =
                        make_tuple(merge(move_state, state, HASH), vec(on_tape_letter), HEAD_RIGHT);

                // 3
                transitions[make_pair(merge(move_state, state, HASH), vec(BLANK))] =
                        make_tuple(merge(return_state, state), vec(HASH), HEAD_LEFT);
            }
            // 4
            for (char enrichment_no_mark: letter_enrichment_no_mark) {
                string on_tape_letter_not_marked = enrich(letter, enrichment_no_mark);

                transitions[make_pair(merge(return_state, state), vec(on_tape_letter_not_marked))] =
                        make_tuple(merge(return_state, state), vec(on_tape_letter_not_marked), HEAD_LEFT);
            }

            string on_tape_letter_marked = enrich(letter, IS_HEAD);

            // 5
            transitions[make_pair(merge(return_state, state), vec(on_tape_letter_marked))] =
                    make_tuple(merge(return_state, state, letter), vec(on_tape_letter_marked), HEAD_LEFT);
        }
    }

    // special: 2nd tape no space
    for (const auto &state: two_tape_machine.set_of_states()) {
        for (const auto &letter: two_tape_machine.working_alphabet()) {
            transitions[make_pair(merge(mark_return_state, state, letter, string(1, GO_STAY)), vec(HASH))] =
                    make_tuple(merge(extend_state, state), vec(enrich(BLANK, IS_HEAD)), HEAD_RIGHT);

            transitions[make_pair(merge(mark_return_state, state, letter, string(1, GO_RIGHT)), vec(HASH))] =
                    make_tuple(merge(extend_state, state), vec(enrich(BLANK, IS_HEAD)), HEAD_RIGHT);
        }

        transitions[make_pair(merge(extend_state, state), vec(BLANK))] =
                make_tuple(merge(return_state, state), vec(HASH), HEAD_LEFT);
    }

    // special: fall off second tape
    for (const auto &state: two_tape_machine.set_of_states()) {
        for (const auto &letter: two_tape_machine.working_alphabet()) {
            transitions[make_pair(merge(mark_return_state, state, letter, string(1, GO_LEFT)), vec(HASH))] =
                    make_tuple(REJECTING_STATE, vec(HASH), HEAD_STAY);
        }
    }

    // return state to base case
    for (const auto &state: two_tape_machine.set_of_states()) {
        for (const auto &state_letter: two_tape_machine.working_alphabet()) {
            for (const auto &tape_letter: two_tape_machine.working_alphabet()) {
                for (char enrichmentDirection: letter_enrichment_directions) {
                    string on_tape_letter_before = enrich(tape_letter, enrichmentDirection);
                    string on_tape_letter_after = enrich(tape_letter, NOTHING_SPECIAL);

                    transitions[make_pair(merge(return_state, state, state_letter), vec(on_tape_letter_before))] =
                            make_tuple(merge(state, state_letter), vec(on_tape_letter_after),
                                       enrichment_to_dir(enrichmentDirection));
                }
            }
        }
    }

    // special: accept/reject
    for (const auto &letter1: two_tape_machine.working_alphabet()) {
        for (const auto &letter2: two_tape_machine.working_alphabet()) {
            auto letter_on_tape = enrich(letter1, NOTHING_SPECIAL);

            transitions[make_pair(merge(ACCEPTING_STATE, letter2), vec(letter_on_tape))] =
                    make_tuple(ACCEPTING_STATE, vec(letter_on_tape), HEAD_STAY);

            transitions[make_pair(merge(REJECTING_STATE, letter2), vec(letter_on_tape))] =
                    make_tuple(REJECTING_STATE, vec(letter_on_tape), HEAD_STAY);
        }
    }

    return {1, two_tape_machine.input_alphabet, transitions};
}

