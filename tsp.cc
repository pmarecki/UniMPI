#include "STL_MACRO.h"
#include "stoper.h"
//http://comopt.ifi.uni-heidelberg.de/software/TSPLIB95/


//attack symmetric TSP
//ATT distance function:
//xd = abs(x[i] - x[j]);
//yd = abs(y[i] - y[j]);
//dij = nint(xd + yd);


//Shortest length and its node order.
pair<double, vi> Optimal(vector<vector<double> > G) {
  int N = G.size();
  vi order(N);
  REP(i,N)
    order[i] = i;
  vi best;
  double best_len = 1e10;   //infinity
  do {
    double len = 0;
    for(int t=1; t<N; ++t)
      len += G[order[t-1]][order[t]];
    if (len < best_len) {
      best_len = len;
      vi tmp = order;
      best.swap(tmp);
    }
  } while( next_permutation(order.begin() + 1, order.end()) );
  return MP(best_len, best);
}

//declaration
pair<double,vi> Refine(vector< vector<double> > G, vi order);

pair<double, vi> Random1(vector<vector<double> > G, int count) {
  srand(time(0));
  int N = G.size();
  vi order(N);
  REP(i,N)
    order[i] = i;
  vi best;
  double best_len = 1e10;   //infinity
  REP(turn, count) {
    random_shuffle(order.begin() + 1, order.end());    //random order
    double len = 0;
    for(int t=1; t<N; ++t)
      len += G[order[t-1]][order[t]];
    if (len < best_len) {
      best_len = len;
      vi tmp = order;
      best.swap(tmp);

    //------ Turn on / off local minimization by pair swapping.
      pair<double,vi> res = Refine(G, best);
      best_len = res.first;
      best.swap(res.second);
    ///-----
    }
  }
  return MP(best_len, best);
}

// Attempt to refine solution by finding best order among orders
// created from given "order" by swapping two sites.
pair<double,vi> Refine(vector< vector<double> > G, vi order) {
  int N = G.size();
  vi bestorder;
  double best_len = 1e10;    //infinity
  for(int i=1; i<N; ++i)
    for(int j=1; j<N; ++j)
    {
      if (i==j) continue;
      vi ord = order;       //shuffled order
      int t = ord[i];
      ord[i] = ord[j];
      ord[j] = t;
      double len = 0;
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



int main() {
  int N = 11;                                   //adjustable
  int x[] = {0,1,2,3,4,4,4,4,4,4,4,4,4,4};      //triangle data //max N=14
  int y[] = {0,0,0,0,0,1,2,3,4,5,6,7,8,9};

  vector<double> empty(N);
  vector<vector<double> > G(N,empty);       //table of distances

  REP(i,N)
    REP(j,N)
      G[i][j] = round(sqrt((x[i]-x[j])*(x[i]-x[j]) + (y[i]-y[j])*(y[i]-y[j])));

  printf("Distance table for N=%i nodes\n", N);
  PrintMatrix(G,N,N);       //connections
  //complete search thru all points, "0" fixed?

  PosixStoper xx;
  pair<double,vi> optimal = Optimal(G);
  xx.Snap();

  printf("Optimal tour (length=%3.2f):", optimal.first);
  PrintContainer(optimal.second);
  xx.PrintDt();

  xx.Snap();
  int count = 1e6;
  pair<double,vi> rnd = Random1(G, count);
  xx.Snap();
  printf("Best random (%i) tour (length=%3.2f):", count, rnd.first);
  PrintContainer(rnd.second);
  xx.PrintDt();


}

