#include <map>
#include <vector>
#include <iostream>

int main()
{
    std::map<std::vector<int>*, int> vec_map;
    std::vector<int> vec1, vec2, vec3;
    vec_map[&vec1] = 1;
    vec_map[&vec2] = 2;
    vec_map[&vec3] = 3;
    std::cout << vec_map.at(&vec1) << std::endl;
    vec1.push_back(1);
    std::cout << vec_map.at(&vec1) << std::endl;
    return 0;
}