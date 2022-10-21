#include "yolo_v2_class.hpp"
#include <fstream>

void show_console_result(std::vector<bbox_t> const result_vec,
			 std::vector<std::string> const obj_names,
			 int frame_id = -1); 

std::vector<std::string> objects_names_from_file(std::string const filename); 
void draw_boxes(cv::Mat mat_img, std::vector<bbox_t> result_vec,
		std::vector<std::string> obj_names, int current_det_fps = -1,
		int current_cap_fps = -1, float thresh = 0.2); 

float getMedian(std::vector<float> &v);
std::vector<bbox_t> get_3d_coordinates(std::vector<bbox_t> bbox_vect, cv::Mat xyzrgba); 
