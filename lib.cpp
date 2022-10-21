#include "libs.h"

void show_console_result(std::vector<bbox_t> const result_vec,
			 std::vector<std::string> const obj_names,
			 int frame_id) {
	if (frame_id >= 0) std::cout << " Frame: " << frame_id << std::endl;
	for (auto &i : result_vec) {
		if (obj_names.size() > i.obj_id)
			std::cout << obj_names[i.obj_id] << " - ";
		std::cout << "obj_id = " << i.obj_id << ",  x = " << i.x
			  << ", y = " << i.y << ", w = " << i.w
			  << ", h = " << i.h << std::setprecision(3)
			  << ", prob = " << i.prob << std::endl;
	}
}

std::vector<std::string> objects_names_from_file(std::string const filename) {
	std::ifstream file(filename);
	std::vector<std::string> file_lines;
	if (!file.is_open()) return file_lines;
	for (std::string line; getline(file, line);) file_lines.push_back(line);
	std::cout << "object names loaded \n";
	return file_lines;
}

void draw_boxes(cv::Mat mat_img, std::vector<bbox_t> result_vec,
		std::vector<std::string> obj_names, int current_det_fps ,
		int current_cap_fps, float thresh) {
	// int const colors[6][3] = { { 1,0,1 },{ 0,0,1 },{ 0,1,1 },{ 0,1,0 },{
	// 1,1,0 },{ 1,0,0 } };

	for (auto &i : result_vec) {
		cv::Scalar color = obj_id_to_color(i.obj_id);
		cv::rectangle(mat_img, cv::Rect(i.x, i.y, i.w, i.h), color, 2);
		if (obj_names.size() > i.obj_id) {
			std::string obj_name = obj_names[i.obj_id];
			if (i.track_id > 0)
				obj_name += " - " + std::to_string(i.track_id);
			cv::Size const text_size = getTextSize(
			    obj_name, cv::FONT_HERSHEY_COMPLEX_SMALL, 1.2, 2,
			    0);
			int max_width = (text_size.width > i.w + 2)
					    ? text_size.width
					    : (i.w + 2);
			max_width = std::max(max_width, (int)i.w + 2);
			// max_width = std::max(max_width, 283);
			std::string coords_3d;
			if (!std::isnan(i.z_3d)) {
				std::stringstream ss;
				ss << std::fixed << std::setprecision(2)
				   << "x:" << i.x_3d << "m y:" << i.y_3d
				   << "m z:" << i.z_3d << "m ";
				coords_3d = ss.str();
				cv::Size const text_size_3d = getTextSize(
				    ss.str(), cv::FONT_HERSHEY_COMPLEX_SMALL,
				    0.8, 1, 0);
				int const max_width_3d =
				    (text_size_3d.width > i.w + 2)
					? text_size_3d.width
					: (i.w + 2);
				if (max_width_3d > max_width)
					max_width = max_width_3d;
			}

			cv::rectangle(
			    mat_img,
			    cv::Point2f(std::max((int)i.x - 1, 0),
					std::max((int)i.y - 35, 0)),
			    cv::Point2f(std::min((int)i.x + max_width,
						 mat_img.cols - 1),
					std::min((int)i.y, mat_img.rows - 1)),
			    color, CV_FILLED, 8, 0);
			putText(mat_img, obj_name, cv::Point2f(i.x, i.y - 16),
				cv::FONT_HERSHEY_COMPLEX_SMALL, 1.2,
				cv::Scalar(0, 0, 0), 2);
			if (!coords_3d.empty())
				putText(mat_img, coords_3d,
					cv::Point2f(i.x, i.y - 1),
					cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8,
					cv::Scalar(0, 0, 0), 1);
		}
	}
	if (current_det_fps >= 0 && current_cap_fps >= 0) {
		std::string fps_str =
		    "FPS detection: " + std::to_string(current_det_fps) +
		    " FPS capture: " + std::to_string(current_cap_fps)+
		    " " + std::to_string(thresh);
		putText(mat_img, fps_str, cv::Point2f(10, 20),
			cv::FONT_HERSHEY_COMPLEX_SMALL, 1.2,
			cv::Scalar(50, 255, 0), 1);
	}
}

float getMedian(std::vector<float> &v) {
	size_t n = v.size() / 2;
	std::nth_element(v.begin(), v.begin() + n, v.end());
	return v[n];
}

std::vector<bbox_t> get_3d_coordinates(std::vector<bbox_t> bbox_vect,
				       cv::Mat xyzrgba) {
	return bbox_vect;
}
