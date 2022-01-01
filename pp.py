fp = open("aaa", "r");
lines = fp.readlines();
tot = 0.0;
for line in lines:
  if (len(line) < 3):
    continue;
  if (line[-2] == '>'):
    print(tot / 4.0);
    tot = 0.0;
    print(line);
    continue;
  if (line[-2] == 's'):
    tot += float(line[10:-2])
print(tot / 4.0);
