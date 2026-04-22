#include "src.hpp"
#include <iostream>
int main(){
    Pokedex p("pokedex_test.txt");
    try {
        p.pokeAdd("Squirtle",7,"water");
        p.pokeAdd("Charmander",4,"fire");
        p.pokeAdd("Garchomp",445,"ground#dragon");
        std::cout << p.pokeFind(7) << "\n";
        std::cout << p.typeFind("ground") << "\n";
        std::cout << p.attack("dragon", 445) << "\n";
        std::cout << p.catchTry() << "\n";
    } catch (const BasicException &e) {
        std::cout << e.what() << "\n";
    }
    return 0;
}
