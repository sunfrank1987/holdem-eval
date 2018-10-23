/*This file, written by Andrew H. Pometta, is the main implementation file
for the term-heval program.  It is in the testing phase: at the moment, we are
simply getting proper option input running. */

#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <getopt.h>
#include <stdexcept>
#include <cassert>
#include <cmath>
#include "OMPEval/omp/EquityCalculator.h"
using namespace omp;
using namespace std;

/*OPTIONS:
--debug: debug.  don't use.  prints debug information
-b, --board: board.  one string.  default empty
-d, --dead: dead.  one string  default empty.
--mc, --monte-carlo: mc simulation.  default false.
-e, --margin, --stderr: margin of error.  expects a double. default 0.1%
                        use 0 for infinite, does nothing without --mc
-t, --time: maximum time to calculate for in seconds.  default 30,
            0 for infinite calculation (use at your own risk)
-h, --help: print help information
*/

bool debug = false; //debug flag

/*Fails the program after printing specified error message.  Sets exit status,
which by default is EXIT_FAILURE. */
void fail_prog(string err_report, int status = EXIT_FAILURE){
  cerr << "term-heval: error: " << err_report << "." << endl;
  exit(status);
}

/*Prints specified string if debug flag is specified, otherwise does nothing.
Saves a lot of space putting the check here.*/
void debug_print(string debug_check){
  if (!debug) return;
  cerr << debug_check << endl;
}

/*Takes vector of given strings and returns necessary vector of hand ranges.
Does all error checking and fails out of program if errors are found.
A bad range is considered to be the empty range. */
vector<CardRange> get_ranges_from_argv(const vector<string>& range_strings){
  if (range_strings.size() < 2) fail_prog("less than 2 hand ranges");
  if (range_strings.size() > 10) fail_prog("more than 10 hand ranges");
  vector<CardRange> ranges;
  for (auto i = range_strings.begin(); i != range_strings.end(); ++i){
    CardRange *cr = new CardRange(*i);
    if (cr->combinations().empty()){
      //empty range, or range resulting from bad string.  fail out
      fail_prog("range " + *i + " invalid");
    }
    ranges.push_back(*cr);
    delete cr; //avoid memory leaks.  this doesn't compromise ranges
    debug_print("Range added: " + *i);
  }
  return ranges;
}

/*This function takes a string corresponding to a list of cards and returns
a "cardmask": uint64_t (large int) corresponding to that specific combination.
This is used for setting board and dead, and also checks for errors.
An optional boolean argument, board, is used when checking for the board,
which has stricter standards (i.e. card maximum).*/
uint64_t get_cardmask(string cards, bool board = false){
  //We need to ensure that the string is valid, i.e. check the bitmask.
  uint64_t bitmask = CardRange::getCardMask(cards);
  if (bitmask == 0){
    if (board) fail_prog("invalid board argument");
    else fail_prog("invalid dead argument");
  }
  //if there are more than 10 chars in board, then the board has more than
  //5 cards, which is too large.  it's possible the user inputted too many
  //on accident, but we will simply have to fail out here
  if (board && cards.length() > 10) fail_prog("board has too many cards");
  return bitmask;
}

int main(int argc, char **argv){
  //Option gathering
  uint64_t board = 0; uint64_t dead = 0;
  bool monte_carlo = false;
  double err_margin = 1e-3; double time_max = 30;
  static struct option long_options[] = {
    {"debug", no_argument, 0, '0'},
    {"board", required_argument, 0, 'b'},
    {"dead", required_argument, 0, 'd'},
    {"mc", no_argument, 0, 'm'},
    {"monte-carlo", no_argument, 0, 'm'}, //same as --mc
    {"margin", required_argument, 0, 'e'},
    {"stderr", required_argument, 0, 'e'}, //same as --margin
    {"time", required_argument, 0, 't'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
  };
  int opt_character;
  while ((opt_character = getopt_long(argc, argv, "0b:d:me:t:h", long_options,
    nullptr)) != -1){
    switch(opt_character){
      case '0': //if --debug is set, it should be set first.
        debug = true;
        break;
      case 'b':
        board = get_cardmask(optarg, true);
        break;
      case 'd':
        dead = get_cardmask(optarg, false);
        break;
      case 'm':
        monte_carlo = true;
        break;
      case 'e': //wrapping the case in brackets prevents
      {         //"jump to case label" error
        string cpp_err = optarg;
        try {
          err_margin = stod(cpp_err);
        } catch (out_of_range oor) {
          fail_prog("Error margin out of range");
        } catch (invalid_argument ia) {
          fail_prog("Invalid error margin argument");
        }
        break;
      }
      case 't':
      {
        string cpp_time = optarg;
        try {
          time_max = stod(cpp_time);
        } catch (out_of_range oor) {
          fail_prog("Maximum time out of range (use -t 0 for no time limit)");
        } catch (invalid_argument ia) {
          fail_prog("Invalid maximum time argument");
        }
        break;
      }
      case 'h':
        break;
    }
  }
  //make sure running is non-infinite
  if (monte_carlo && (err_margin == 0) && (time_max == 0)){
    fail_prog("infinite simulation queried (set time limit, error margin"
              " or disable monte-carlo)");
  }

  //put all remaining ranges into vector of strings
  vector<string> range_strs;
  for (int i = optind; i < argc; ++i){
    range_strs.push_back(argv[i]);
  }

  //setting values & final error checking
  vector<CardRange> ranges = get_ranges_from_argv(range_strs);
  EquityCalculator eq;
  eq.setTimeLimit(time_max);

  //running equity calculator
  debug_print("board = " + to_string(board));
  debug_print("dead = " + to_string(dead));
  debug_print("monte_carlo = " + to_string(monte_carlo));
  debug_print("err_margin = " + to_string(err_margin));
  eq.start(ranges, board, dead, monte_carlo, err_margin);
  eq.wait();

  //printing results
  auto r = eq.getResults();
  debug_print("r.progress = " + to_string(r.progress));
  debug_print("r.stdev = " + to_string(r.stdev));
  debug_print("r.time = " + to_string(r.time));
  debug_print("***");
  assert (range_strs.size() == r.players);
  cout << "Equity between " + to_string(r.players) + " players:" << endl;
  //we are traversing 2 lists at the same time, so we just use an int
  for (unsigned int i = 0; i < r.players; ++i){
    cout << range_strs.at(i) << ": " + to_string(r.equity[i]) << endl;
  }
  debug_print("***");
  bool completed = (r.progress >= 1);
  //this is a bootleg way of rounding to the tenths place - there might be
  //a more elegant solution, but this works with the default functions
  //provided in <cmath>
  double prog_percentage = round(r.progress * 1000) / 10;
  double err_percentage = round(r.stdev * 1000) / 10;
  double calc_time = round(r.time * 10) / 10;
  debug_print("completed = " + to_string(completed));
  debug_print("prog_percentage = " + to_string(prog_percentage));
  debug_print("err_percentage = " + to_string(err_percentage));
  debug_print("calc_time = " + to_string(calc_time));
  debug_print("***");

  if (completed){
    cout << "Calculation completed in " << calc_time << " seconds." << endl;
  } else {
    cout << "Calculation timed out after " << calc_time << " seconds: "
            "target margin of error not reached." << endl;
    if (r.enumerateAll){
      cout << "Consider using monte-carlo with --mc" << endl;
    }
  }

  return EXIT_SUCCESS;
}
