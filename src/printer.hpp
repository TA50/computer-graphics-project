#pragma  once

#include <iostream>
#include <glm/glm.hpp>

class Printer {
public:
    static void printArrArr(glm::mat4 arr) {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                auto elem = arr[i][j];
                std::cout << elem << " ";
            }
            std::cout << std::endl;
        }
    }
};
