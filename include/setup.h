#include <iostream>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/Dense>
#include <MathConstants.h>

//read obj file into eigen matrices

bool read_obj(const std::string& filename,
	Eigen::MatrixXd& N,
	Eigen::MatrixXd& V,
	Eigen::MatrixXd& UV,
	Eigen::MatrixXd& F);

void init_view_proj(Eigen::Affine3f& view,
	Eigen::Matrix4f& proj,
	Eigen::Vector3f& cam_pos,
	const float fov,
	const float width,
	const float height);

