#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <set>
#include <string>
#include <utility>
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

char letter_enrichment[5] = {NOTHING_SPECIAL, IS_HEAD, GO_LEFT, GO_RIGHT, GO_STAY};
// ((id)1) for example

string mark_with_head(string letter) { // todo czy to nie zmienia oryginału ?
    letter[letter.size() - 1] = IS_HEAD;
    return letter;
}

vector<string> vec(const string &letter) {
    return {1, letter};
}

string bracketize(const string &s) {
    return "(" + s + ")";
}

char dir_to_en(string dir) {
    if (dir.length() != 1) {
        cerr << "SOMETHING IS WRONG" << endl;
        exit(1);
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
    cerr << "SOMETHING IS WRONG" << endl;
    exit(1);
}

string enrich(const string &s, char en) {
    return bracketize(bracketize(s) + en);
}

string merge(const string &a, const string &b) {
    return bracketize(bracketize(a) + '-' + bracketize(b));
}

string merge(const string &a, const string &b, const string &c) {
    return bracketize(bracketize(a) + '-' + bracketize(b) + '-' + bracketize(c));
}

string merge(const string &a, const string &b, const string &c, const string &d) {
    return bracketize(bracketize(a) + '-' + bracketize(b) + '-' + bracketize(c) + '-' + bracketize(d));
}


TuringMachine two_tape_to_one_tape(TuringMachine &two_tape_machine) {
    vector<string> input_alphabet = two_tape_machine.input_alphabet;
    transitions_t transitions;

    string HASH = "(HASH)"; // TODO randomize names until unique
    string create_state_1 = "(create_state_1)";
    string create_state_2 = "(create_state_2)";
    string create_state_3 = "(create_state_3)";
    string return_state = "(return_state)";
    string return_from_start_1 = "(return_from_start_1)";
    string return_from_start_2 = "(return_from_start_2)";
    string mark_return_state = "(mark_return_state)";

    // special: start
    for (const auto &letter: input_alphabet) {
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
                merge(return_from_start_1, INITIAL_STATE, BLANK),// TODO must be unique
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
    for (auto transition: two_tape_machine.transitions) { // TODO HANDLE ACCEPT/REJECT SEPARATELY
        string q1 = transition.first.first;
        string c1 = transition.first.second[0];
        string c2 = transition.first.second[1];
        string q2 = get<0>(transition.second);
        string c1p = get<1>(transition.second)[0];
        string c2p = get<1>(transition.second)[1];
        string d1 = string(1, get<2>(transition.second)[0]);
        string d2 = string (1, get<2>(transition.second)[1]);

        string c1_with_dir = enrich(c1p, dir_to_en(d1));
        {
            auto letter_before = vec(enrich(c1, NOTHING_SPECIAL));
            auto letter_after = vec(c1_with_dir);

            // 1
            transitions[make_pair(merge(q1, c2), letter_before)] =
                    make_tuple(merge(q2, c2p, string(1,dir_to_en(d2))), letter_after, HEAD_RIGHT);
        }
        ///////////////////////////////
        auto state = merge(q2, c2p, string(1,dir_to_en(d2)));
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
            auto letter_after = vec(c2p);
            // 3
            transitions[make_pair(state, letter_before)] =
                    make_tuple(merge(mark_return_state, q2, c2p, string(1,dir_to_en(d2))), letter_after, d2);
        }

        ///////////////////////////////
        for (const auto &letter: two_tape_machine.working_alphabet()) {
            auto letter_before = vec(enrich(letter, NOTHING_SPECIAL));
            auto letter_after = vec(enrich(letter, IS_HEAD));

            // 4
            transitions[make_pair(merge(mark_return_state, q2, c2p, string(1,dir_to_en(d2))), letter_before)] =
                    make_tuple(merge(return_state, q2, c2p), letter_after, HEAD_LEFT);

        }

        ///////////////////////////////
        for (const auto &letter: two_tape_machine.working_alphabet()) {
            auto letter_before = vec(enrich(letter, NOTHING_SPECIAL));

            // 5
            transitions[make_pair(merge(return_state, q2, c2p), letter_before)] =
                    make_tuple(merge(return_state, q2, c2p), letter_before, HEAD_LEFT);

            letter_before = vec(enrich(letter, IS_HEAD));

            // 5 (marked, but ignore mark)
            transitions[make_pair(merge(return_state, q2, c2p), letter_before)] =
                    make_tuple(merge(return_state, q2, c2p), letter_before, HEAD_LEFT);
        }

        // 5 (hash)
        transitions[make_pair(merge(return_state, q2, c2p), vec(HASH))] =
                make_tuple(merge(return_state, q2, c2p), vec(HASH), HEAD_LEFT);

        ///////////////////////////////
        transitions[make_pair(merge(return_state, q2, c2p), vec(c1_with_dir))] =
                make_tuple(merge(q2, c2p), vec(c1p), d1);
    }

    return {1, two_tape_machine.input_alphabet, transitions};
}

