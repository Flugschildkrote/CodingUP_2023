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


int main(void)
{
    int posHistory[3]{ 0, 0, 0 };

    constexpr int32_t TARGET_HEIGHT_CM = 32400;

    int32_t x = 1170;
    int32_t y = 290;

    int32_t pos = 0;
    int32_t day = 0;
    do {

        // day
        pos += x;
        x -= 2;
        std::cout << "Fin du jour " << day << " : " << pos << std::endl;

        // night
        pos -= y;

        posHistory[2] = posHistory[1];
        posHistory[1] = posHistory[0];
        posHistory[0] = pos;

        if (((day+1) % 5) == 0) {
            pos = posHistory[2];
        }

        std::cout << "Fin de la nuit " << day << " : " << pos << std::endl;


        day++;
    } while (pos < TARGET_HEIGHT_CM);


    std::cout << day << std::endl;

    return EXIT_SUCCESS;
}

