#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <utility>
#include <vector>
#define OPENCV
#include "../include/rules.hpp"
/*
struct bbox_t {
    unsigned int x, y, w, h;       // (x,y) - top-left corner, (w, h) - width & height of bounded box
    float prob;                    // confidence - probability that the object was found correctly
    unsigned int obj_id;           // class of object - from range [0, classes-1]
    unsigned int track_id;         // tracking id for video (0 - untracked, 1 - inf - tracked object)
    unsigned int frames_counter;   // counter of frames on which the object was detected
    float x_3d, y_3d, z_3d;        // center of object (in Meters) if ZED 3D Camera is used
};
 * */

std::vector<std::string> objects_names_from_file(std::string const filename) {
    std::ifstream file(filename);
    std::vector<std::string> file_lines;
    if (!file.is_open()) return file_lines;
    for (std::string line; getline(file, line);)
        file_lines.push_back(line);
    std::cout << "object names loaded \n";
    return file_lines;
}

TEST(rule, init) {
	AfterDetectHook hook;
	bbox_t p1 { .x = 289, .y = 96, .w = 10, .h = 10 };
	std::vector<bbox_t> vi;
	vi.push_back(p1);
	hook.run(vi);
	EXPECT_TRUE(vi.empty());

	p1.x = 0;
	vi.push_back(p1);
	hook.run(vi);
	EXPECT_FALSE(vi.empty());
}

TEST(rule, remove_inside_excluded_area) {
	AfterDetectHook hook;
	bbox_t p1 { .x = 289, .y = 92, .w = 10, .h = 10 };
	bbox_t p2 { .x = 189, .y = 96, .w = 10, .h = 10 };
	bbox_t p3 { .x = 789, .y = 97, .w = 10, .h = 10 };
	bbox_t p4 { .x = 289, .y = 94, .w = 10, .h = 10 };
	p1.obj_id = p2.obj_id = p3.obj_id = p4.obj_id = 0;
	std::vector<bbox_t> vi;
	vi.push_back(p1);
	vi.push_back(p2);
	vi.push_back(p3);
	vi.push_back(p4);
	hook.run(vi);

	EXPECT_EQ(2, vi.size());

	EXPECT_EQ(vi.at(1).y, 97); 
	EXPECT_EQ(vi.at(0).y, 96); 
}
