#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <stack>
#include <sstream>
#include <climits>
#include <deque>
#include <set>
#include <utility>
#include <queue>
#include <map>
#include <cstring>
#include <algorithm>
#include <iterator>
#include <string>
#include <cassert>
#include <unordered_set>
#include <unordered_map>

using namespace std;

void gen_ass()
{
	for (int i = 0; i < 32; ++i) {
		printf("\"sd $%d, %d(\%%0)\\n\\t\"\n", i, i * 8);
	}
}

void gen_cp0()
{
	std::ifstream infile("/home/maritns3/core/loongson-dune/3day/kvm/cp0.h");
	string line;
	while (std::getline(infile, line)) {
    std::istringstream iss(line);
    string x;
    iss >> x;
    iss >> x;

    if(x.substr(0, 3) == "KVM"){
      string g = x.substr(strlen("KVM_REG_MIPS_CP0_"));
      printf(
        "{ .reg = {.id = %s},"
        "  .name = \"%s\","
        "  .v = 0x12345678"
        "},\n", x.c_str(), g.c_str());

    }
	}
}

int main(int argc, char *argv[])
{
	gen_cp0();

	return 0;
}
