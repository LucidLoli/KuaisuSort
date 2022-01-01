// C++ Source
// std::sort

#include <bits/stdc++.h>
#include <sys/time.h>

int main()
{
  using namespace std;
  ios::sync_with_stdio(0);
  cout.tie(0);
  cin.tie(0);
  cerr.tie(0);
  vector<int32_t> a;
  int32_t p;
  while (cin >> hex >> p) {
    a.push_back(p);
  }
  cerr << "Data Loaded." << endl;

  timeval start, end;
  gettimeofday(&start, NULL);

  std::sort(a.begin(), a.end());
  gettimeofday(&end, NULL);
  cerr << (end.tv_sec - start.tv_sec + (end.tv_usec - start.tv_usec) / 1000000.0) << " s" << endl;
  cerr << "Printing Data." << endl;
  cout << setiosflags(ios::left);
  for (int32_t i = 0; i < a.size(); i++) {
    cout << hex << a[i];

    if (i % 15 == 14) cout << "\n";
    else cout << " ";
  }

  return 0;
}
