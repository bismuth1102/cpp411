/**
 * CSE 411
 * Fall 2018
 * Programming Assignment #2
 *
 * The purpose of this assignment is to get you thinking about what it means for
 * something to happen "before" something else.  We will explore this notion of
 * "before" using a variation on BlockChain technologies.
 *
 * The main idea in a BlockChain is that the provenance of a piece of
 * information becomes increasingly difficult to forge over time.  That is,
 * every time data (a "block") is added to the chain, that data reinforces the
 * fact that the data that precedes it in the chain is, indeed, present and
 * immutable.
 *
 * In this project, we will make "clockchain".  The idea behind "clockchain" is
 * that a server (the "clockchain server") will periodically insert timestamps
 * into the chain.  A user can also add blocks of data to the chain.  If we
 * trust that the timestamps are, indeed, inserted at the time they claim to
 * have been inserted, then we can look at a block of data, and the next clock,
 * and conclude that the block of data had to have been inserted before the
 * clock's time.
 *
 * Clockchain could be a useful idea for an inventor: she could use it to prove
 * that she had an idea before someone else, by showing that her idea is in the
 * clockchain before someone else's idea.  In that case, one might only insert a
 * hash of the idea into the clockchain (because the idea might need to be kept
 * secret), but that's fine... the inventor could still prove that she had the
 * idea by presenting the idea and showing that it generates the hash value that
 * is in the chain.
 *
 * There are several steps to this project.
 *
 * - First, you should read this reference:
 *   https://davenash.com/2017/10/build-a-blockchain-with-c/.  Then you should
 *   implement it, but you must use OpenSSL's SHA256 algorithm, not the one that
 *   Dave uses in the tutorial.
 *
 * - Second, you should extend your implementation so that it is possible to
 *   display blocks from the blockchain.
 *
 * - Third, you should transform the program so that it is an always-running
 *   server, and so that it can receive connections on a port (specified by the
 *   -p command-line option), read data over those connections, and then put
 *   that data into the chain.  Note that you will need to design a reasonable
 *   protocol for how to send data, so that it includes the sender ID, the
 *   content, etc.
 *
 * - Fourth, you should ensure that every block gets correctly persisted to a
 *   file, so that the program can be stopped and started without losing data.
 *   You will need an intelligent file format for this, since blocks have
 *   variable length.
 *
 * - Fifth, you should add a timer to your server, so that a new block is added
 *   every minute.  This will provide the proof of "before" for entries.
 *
 * This assignment is intentionally under-specified.  You should ask questions
 * on Piazza to get more information about exactly what I expect of your
 * program.
 *
 * Your program should use advanced features of C++ whenever appropriate.  It
 * should compile with g++ version 7.  You should organize your code into files
 * in a way that is appropriate.  You should comment your code.
 *
 * Turn-in will be via bitbucket.
 */

#include <iostream>
#include <libgen.h>
#include <unistd.h>

/** Print some helpful usage information */
void usage(const char *progname) {
  using std::cout;
  cout << "ClockChain Server\n";
  cout << "  Usage: " << progname << " [options]\n";
  cout << "    -p <int> : Port on which to listen (default 41100)\n";
  cout << "    -h       : print this message\n";
}

int main(int argc, char *argv[]) {
  // Config vars that we get via getopt
  int port = 41100;       // random seed
  bool show_help = false; // show usage?

  // Parse the command line options:
  int o;
  while ((o = getopt(argc, argv, "p:h")) != -1) {
    switch (o) {
    case 'h':
      show_help = true;
      break;
    case 'p':
      port = atoi(optarg);
      break;
    default:
      show_help = true;
      break;
    }
  }

  // Print help and exit
  if (show_help) {
    usage(basename(argv[0]));
    exit(0);
  }

  // Print the configuration... this makes results of scripted experiments
  // much easier to parse
  std::cout << "p = " << port << std::endl;

  // Time for you to start writing code :)
}