#include "STL_MACRO.h"
#include "stoper.h"
//http://comopt.ifi.uni-heidelberg.de/software/TSPLIB95/


//attack symmetric TSP
//ATT distance function:
//xd = abs(x[i] - x[j]);
//yd = abs(y[i] - y[j]);
//dij = nint(xd + yd);

//Compute length of a tour given by `order[]` of nodes.
float length(vector< vector<float> > &G, vi &order) {
  int N = G.size();
  float len = 0;
  for(int t=1; t<N; ++t)
    len += G[order[t-1]][order[t]];
  return len;
}

//Take reference and swap nodes `i` and `j` in order.
void swap_nodes(vi &order, int i, int j) {
  int t = order[i];
  order[i] = order[j];
  order[j] = t;
}



//Returns shortest length and its node order.
pair<float, vi> Optimal(vector<vector<float> > G) {
  int N = G.size();
  vi order(N);
  REP(i,N)
    order[i] = i;
  vi best;
  float best_len = 1e10;   //infinity
  do {
    float len = length(G,order);
    if (len < best_len) {
      best_len = len;
      vi tmp = order;
      best.swap(tmp);
    }
  } while( next_permutation(order.begin() + 1, order.end()) );
  return MP(best_len, best);
}

//declaration
pair<float,vi> Refine(vector< vector<float> > &G, vi order);

pair<float, vi> Random1(vector<vector<float> > G, int count) {
  srand(time(0));
  int N = G.size();
  vi order(N);
  REP(i,N)
    order[i] = i;
  vi best;
  float best_len = 1e10;   //infinity
  REP(turn, count) {
    random_shuffle(order.begin() + 1, order.end());    //random order
    float len = length(G, order);
    if (len < best_len) {
      best_len = len;
      vi tmp = order;
      best.swap(tmp);

    //------ Turn on / off local minimization by pair swapping.
      pair<float,vi> res = Refine(G, best);
      best_len = res.first;
      best.swap(res.second);
    ///-----
    }
  }
  return MP(best_len, best);
}

// Attempt to refine solution by finding best order among orders
// created from given "order" by swapping two sites.
pair<float,vi> Refine(vector< vector<float> >& G, vi order) {
  int N = G.size();
  vi bestorder;
  float best_len = 1e8;    //infinity
  for(int i=1; i<N; ++i)
    for(int j=1; j<N; ++j)
    {
      if (i==j) continue;
      vi ord = order;       //shuffled order
      swap_nodes(ord, i, j);
      float len = 0;
      for(int t=1; t<N; ++t)
        len += G[ord[t-1]][ord[t]];
      if (len < best_len) {
        best_len = len;
        vi tmp = ord;
        bestorder.swap(tmp);
      }
    }
  return MP(best_len, bestorder);
}



////////////////////////////
// Best algorithm.
////////////////////////////
//
//  Uses only `length()` and `swap_nodes()` functions.
//  Algorithm starts from random order, and attempts random swaps of nodes.
//  After each swap: if solution is better than best it is accepted as best.
//  If it is worse than best, than with some probability it is accepted.
//  Probability drops exponentially with (length_current - lenght_best) / t,
//  where "t" is the temperature.
//
//  Temperature is reduced by multiplying it with "factor" every "tmp_reduce"
//  swaps of nodes.
//
pair<float, vi> Anneal(vector<vector<float> > &G, int count, int tmp_reduce,
    float amplitude, float factor) {
  srand(time(0));
  int N = G.size();
  //some initial order
  vi order(N);
  REP(i,N)
    order[i] = i;
  random_shuffle(order.begin() + 1, order.end());
  vi best_order(order);
  float best_len = length(G, order);
  float t = 1.;

  REP(turn, count) {
    int i = rand() % N;
    int j = rand() % N;
    if (i==0 || j==0) continue;
    vi order(best_order);
    swap_nodes(order, i, j);      //make second swap if not accepted
    float len = length(G, order);
    if (len < best_len) {
      best_len = len;
      best_order.swap(order);
    } else {
      float p = (float)(rand() % 1000) / 1000;
      if (exp((len - best_len) / t) < amplitude * p) {
        best_len = len;
        best_order.swap(order);
      }
    }
    if (turn % tmp_reduce == 0) t *= factor;
  }
  return MP(best_len, best_order);
}


int main() {
  int N = 19;                                   //adjustable
  int x[] = {0,1,2,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4};      //triangle data //max N=16
  int y[] = {0,0,0,0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14};

  vector<float> empty(N);
  vector<vector<float> > G(N,empty);       //table of distances

  REP(i,N)
    REP(j,N)
      G[i][j] = round(sqrt((x[i]-x[j])*(x[i]-x[j]) + (y[i]-y[j])*(y[i]-y[j])));

  printf("Distance table for N=%i nodes\n", N);
//  PrintMatrix(G,N,N);       //connections
  //complete search thru all points, "0" fixed?

  PosixStoper xx;
  ////// Complete search
//  pair<float,vi> optimal = Optimal(G);
//  xx.Snap();
//
//  printf("Optimal tour (length=%3.2f):", optimal.first);
//  PrintContainer(optimal.second);
//  xx.PrintDt();

  //////Fully random search
  xx.Snap();
  int count = 1e6;
  pair<float,vi> rnd = Random1(G, count);
  xx.Snap();
  printf("Best random (%i) tour (length=%3.2f):", count, rnd.first);
  PrintContainer(rnd.second);
  xx.PrintDt();

  ////// Simulated Annealing
  xx.Snap();
  int count_a = 1e5;
  //synopsis: number of swaps (1e4), amplitude (=100),
  //          temp. adjustment period (100), temp. multiplicator (0.99)
  // every "adj.period" t*= multiplicator.
  pair<float,vi> rnd_a = Anneal(G, count_a, 100, 100, 0.99);
  xx.Snap();
  printf("Best anneal (%i) tour (length=%3.2f):", count_a, rnd_a.first);
  PrintContainer(rnd_a.second);
  xx.PrintDt();


}

