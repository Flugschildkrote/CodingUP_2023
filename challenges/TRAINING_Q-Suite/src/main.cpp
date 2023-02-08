#include <cstdlib>
#include <iostream>
#include <anubis/Anubis.hpp>
#include <anubis/Tree.hpp>
#include <anubis/net/Http.hpp>
#include <cassert>
#include <stb_image.h>
#include <stb_image_write.h>
#include <fstream>
#include <map>

std::map<int, int> qSuiteResults;
int Q(int n) {
    if (n == 1 || n == 2)
        return 1;

    if (auto it = qSuiteResults.find(n); it != qSuiteResults.end()) {
        return it->second;
    }


    int result = Q(n - Q(n - 1)) + Q(n - Q(n - 2));
    qSuiteResults[n] = result;
    return result;
}


int main(void)
{
    int res = 0;
    for (int i = 1583; i <= 1697; ++i) {
        res += Q(i);
    }

    std::cout << res << std::endl;

    return EXIT_SUCCESS;
}