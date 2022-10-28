#include <iostream>
#include <cstdlib>
#include <fstream>
#include "turing_machine_converter.cpp"

using namespace std;

static bool verbose = true;

static void print_usage(const string &error) {
    cerr << "ERROR: " << error << "\n"
         << "Usage: tm_translator <input_file> <output_file>\n";
    exit(1);
}


int main(int argc, char *argv[]) {
    string two_tape_filename;
    string one_tape_filename;
    int ok = 0;
    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (ok == 0)
            two_tape_filename = arg;
        else if (ok == 1)
            one_tape_filename = arg;
        else
            print_usage("Too many arguments");
        ++ok;
    }
    if (ok != 2)
        print_usage("Not enough arguments");

    FILE *f = fopen(two_tape_filename.c_str(), "r");
    if (!f) {
        cerr << "ERROR: File " << two_tape_filename << " does not exist\n";
        return 1;
    }
    TuringMachine tm = read_tm_from_file(f);

    TuringMachine one_tape_tm = two_tape_to_one_tape(tm);

    std::ofstream file;
    file.open(one_tape_filename);
    one_tape_tm.save_to_file(file);

}


