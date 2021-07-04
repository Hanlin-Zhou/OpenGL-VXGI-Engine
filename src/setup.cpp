#include <setup.h>

bool read_obj(const std::string& filename,
	Eigen::MatrixXd& N,
	Eigen::MatrixXd& V,
	Eigen::MatrixXd& UV,
	Eigen::MatrixXd& F) {
	return true;
}

void init_view_proj(Eigen::Affine3f& view,
	Eigen::Matrix4f& proj,
	Eigen::Vector3f& cam_pos,
	const float fov,
	const float width,
	const float height) {
	view = Eigen::Affine3f::Identity() * Eigen::Translation3f(Eigen::Vector3f(0, 0, -10));

	proj.setZero();
	float n = 0.01;
	float f = 100;
	float top = tan(fov / 360.0 * M_PI) * n;
	float bot = -top;
	float right = top / height * width;
	float left = -right;

	proj(0, 0) = (2.0 * n) / (right - left);
	proj(1, 1) = (2.0 * n) / (top - bot);
	proj(0, 2) = (right + left) / (right - left);
	proj(1, 2) = (top + bot) / (top - bot);
	proj(2, 2) = -(f + n) / (f - n);
	proj(3, 2) = -1.0;
	proj(2, 3) = -(2.0 * f * n) / (f - n);

	cam_pos.setZero();
	cam_pos(2) = -10;
	/*cam_up.setZero();
	cam_pos(1) = 1;
	cam_pos.setZero();
	cam_pos(0) = 1;*/
}