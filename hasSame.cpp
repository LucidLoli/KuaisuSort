#include <bits/stdc++.h>

std::set<int> input;

int main() {
  uint t; uint i = 0; uint c = 0;
  while (scanf("%x", &t) > 0) {
    if (input.find(t) == input.end()) {
      input.insert(t);
    }
    else {
      printf("[%d] %x\n", i, t);
      c++;
      if (c == 32) break;
    }
    i++;
  }
  return 0;
}
