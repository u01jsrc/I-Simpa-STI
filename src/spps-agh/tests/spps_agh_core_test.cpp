#define BOOST_TEST_MODULE spps agh core test
#include <boost/test/included/unit_test.hpp>
#include <tools/dotreflection.h>
#include <tools/brdfreflection.h>
#include <iostream>
#include <math.h>
#include "spps_agh_core_test.h"

using namespace std;
namespace utf = boost::unit_test;

vec3 sphericalToCartesian(double phi, double theta)
{
	double x = sin(theta) * cos(phi);
	double y = sin(theta) * sin(phi);
	double z = cos(theta);
	vec3 v(x, y, z);
	return v;
}

void IntegratePhongModel(core_mathlib::vec3 &specular, double phi_increment, double c_increment, int n, double diffusion, float solidAngle, core_mathlib::vec3 &position, core_mathlib::vec3 &faceNormal, double &result)
{
	specular.normalize();
	for (double phi = 0; phi < 2 * M_PI; phi += phi_increment) {
		for (double c = 0; c < 1; c += c_increment)
		{
			double theta = acos(1 - c);
			vec3 target = sphericalToCartesian(phi, theta);
			BRDFs::evaluatePhongAtPoint(n, diffusion, solidAngle, target, position, faceNormal, specular, result);
		}
	}
}

BOOST_AUTO_TEST_CASE(test_mc_phong_model)
{
	float tolerance = 0.1;

	int phi_steps = 720;
	int c_steps = 360;
	float solidAngle = 2 * M_PI / (phi_steps * c_steps);
	double phi_increment = 2. * M_PI / phi_steps;
	double c_increment = 1. / c_steps;

	//******************** Test diffuse lobe ***************************************
	int n = 0;
	double diffusion = 1;
	vec3 position(0, 0, 0);
	vec3 faceNormal(0, 0, 1);
	vec3 specular(0, 0, 1);
	
	double result = 0;
	IntegratePhongModel(specular, phi_increment, c_increment, n, diffusion, solidAngle, position, faceNormal, result);
	BOOST_CHECK_MESSAGE(abs(1 - result) < tolerance, "Failed test for diffuse lobe, total energy = " << result);

	//********************  Test specular lobe, perpendicular ***************************************
	n = 0;
	diffusion = 0;
	position = vec3(0, 0, 0);
	faceNormal = vec3(0, 0, 1);
	specular = vec3(0, 0, 1);

	result = 0;
	IntegratePhongModel(specular, phi_increment, c_increment, n, diffusion, solidAngle, position, faceNormal, result);
	BOOST_CHECK_MESSAGE(abs(1 - result) < tolerance, "Failed test for n=" << n << " total energy = " << result);

	//********************  Test specular lobe, perpendicular ***************************************
	n = 10;
	diffusion = 0;
	position = vec3(0, 0, 0);
	faceNormal = vec3(0, 0, 1);
	specular = vec3(0, 0, 1);

	result = 0;
	IntegratePhongModel(specular, phi_increment, c_increment, n, diffusion, solidAngle, position, faceNormal, result);
	BOOST_CHECK_MESSAGE(abs(1 - result) < tolerance, "Failed test for n=" << n << " total energy = " << result);

	//********************  Test specular lobe, perpendicular ***************************************
	n = 50;
	diffusion = 0;
	position = vec3(0, 0, 0);
	faceNormal = vec3(0, 0, 1);
	specular = vec3(0, 0, 1);

	result = 0;
	IntegratePhongModel(specular, phi_increment, c_increment, n, diffusion, solidAngle, position, faceNormal, result);
	BOOST_CHECK_MESSAGE(abs(1-result) < tolerance, "Failed test for n=" << n << " total energy = " << result);

	//********************  Test specular lobe, 45 ***************************************
	n = 2;
	diffusion = 0;
	position = vec3(0, 0, 0);
	faceNormal = vec3(0, 0, 1);
	specular = vec3(0, 1, 1);

	result = 0;
	IntegratePhongModel(specular, phi_increment, c_increment, n, diffusion, solidAngle, position, faceNormal, result);
	BOOST_CHECK_MESSAGE(abs(1 - result) < tolerance, "Failed test 45deg for n=" << n << " total energy = " << result);

	//********************  Test specular lobe, 45 ***************************************
	n = 10;
	diffusion = 0;
	position = vec3(0, 0, 0);
	faceNormal = vec3(0, 0, 1);
	specular = vec3(0, 1, 1);

	result = 0;
	IntegratePhongModel(specular, phi_increment, c_increment, n, diffusion, solidAngle, position, faceNormal, result);
	BOOST_CHECK_MESSAGE(abs(1 - result) < tolerance, "Failed test 45deg for n=" << n << " total energy = " << result);

	//********************  Test specular lobe, 45 ***************************************
	n = 50;
	diffusion = 0;
	position = vec3(0, 0, 0);
	faceNormal = vec3(0, 0, 1);
	specular = vec3(0, 1, 1);

	result = 0;
	IntegratePhongModel(specular, phi_increment, c_increment, n, diffusion, solidAngle, position, faceNormal, result);
	BOOST_CHECK_MESSAGE(abs(1 - result) < tolerance, "Failed test 45deg for n=" << n << " total energy = " << result);
}

BOOST_AUTO_TEST_CASE(test_phong_normalization)
{
	int n = 0;
	vec3 faceNormal = vec3(0, 0, 1);
	vec3 specular = vec3(0, 0, 1);
	specular.normalize();

	double normalizationFactor = BRDFs::calculatePhongNormalizationFactor(faceNormal, specular, n);
	BOOST_CHECK_CLOSE(M_PI, normalizationFactor, EPSILON);

	// ************************************************************************
	n = 10;
	faceNormal = vec3(0, 0, 1);
	specular = vec3(0, 0, 1);
	specular.normalize();

	normalizationFactor = BRDFs::calculatePhongNormalizationFactor(faceNormal, specular, n);
	BOOST_CHECK_CLOSE((2 * M_PI)/(n + 2) , normalizationFactor, EPSILON);

	// ************************************************************************
	n = 50;
	faceNormal = vec3(0, 0, 1);
	specular = vec3(0, 0, 1);
	specular.normalize();

	normalizationFactor = BRDFs::calculatePhongNormalizationFactor(faceNormal, specular, n);
	BOOST_CHECK_CLOSE((2 * M_PI) / (n + 2), normalizationFactor, EPSILON);
}
