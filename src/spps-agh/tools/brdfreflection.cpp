#include "brdfreflection.h"
#include <iostream>


/**
Calculate the intersection of a ray and a sphere
The line segment is defined from p1 to p2
The sphere is of radius r and centered at sc
There are potentially two points of intersection given by
p = p1 + mu1 (p2 - p1)
p = p1 + mu2 (p2 - p1)
@return Faux if the ray doesn't intersect the sphere.
@see http://local.wasp.uwa.edu.au/~pbourke/geometry/sphereline/
*/
int RaySphereIntersection(const vec3& p1, const vec3& p2, const vec3& sc, double r, double *mu1, double *mu2)
{
	double a, b, c;
	double bb4ac;
	vec3 dp;

	dp.x = p2.x - p1.x;
	dp.y = p2.y - p1.y;
	dp.z = p2.z - p1.z;
	a = dp.x * dp.x + dp.y * dp.y + dp.z * dp.z;
	b = 2 * (dp.x * (p1.x - sc.x) + dp.y * (p1.y - sc.y) + dp.z * (p1.z - sc.z));
	c = sc.x * sc.x + sc.y * sc.y + sc.z * sc.z;
	c += p1.x * p1.x + p1.y * p1.y + p1.z * p1.z;
	c -= 2 * (sc.x * p1.x + sc.y * p1.y + sc.z * p1.z);
	c -= r * r;
	bb4ac = b * b - 4 * a * c;
	if (abs(a) < EPSILON || bb4ac < 0) {
		*mu1 = 0;
		*mu2 = 0;
		return false;
	}

	*mu1 = (-b + sqrt(bb4ac)) / (2 * a);
	*mu2 = (-b - sqrt(bb4ac)) / (2 * a);

	return true;
}

double BRDFs::calculatePhongNormalizationFactor(const vec3& faceNormal, const vec3& specularDirection, const int& n)
{
	double S = 0;
	double d = std::min((float)faceNormal.dot(specularDirection),1.f);
	double c = sqrt(1 - d*d);
	double T = ((n % 2)==0) ? M_PIDIV2 : c;
	double A = ((n % 2) == 0) ? M_PIDIV2 : M_PI - acos(d);
	int i = ((n % 2) == 0) ? 0 : 1;
	while (i <= n - 2) {
		S = S + T;
		T = T*c*c*(i + 1) / (i + 2);
		i = i + 2;
	}
	return 2. * (T + d*A + d*d*S) / (n + 2.);
	
}

double BRDFs::SolveBRDFReflection(const t_Material_BFreq& material, const vec3& faceNormal, const vec3& targetPoint, const CONF_PARTICULE_AGH &shadowRay, const vec3& incomingDirection, Core_ConfigurationAGH *configurationTool)
{
	switch (material.reflectionLaw)
	{
	case REFLECTION_LAW_SPECULAR:
		return SolveSpecularBRDF(material, faceNormal, targetPoint, shadowRay, incomingDirection, configurationTool);

	case REFLECTION_LAW_LAMBERT:
		return SolveSpecularLambertBRDF(material, faceNormal, targetPoint, shadowRay, incomingDirection, configurationTool);

	case REFLECTION_LAW_PHONG:
		return SolvePhongBRDF(material, faceNormal, targetPoint, shadowRay, incomingDirection, configurationTool);

	default:
		return SolveSpecularLambertBRDF(material, targetPoint, faceNormal, shadowRay, incomingDirection, configurationTool);
	}
}

vec3 BRDFs::SolveSpecularReflection(const vec3 &incomingDirection, const vec3 &faceNormal)
{
	vec3 retVal = (incomingDirection - (faceNormal*(incomingDirection.dot(faceNormal)) * 2));
	return retVal / retVal.length();
}

double BRDFs::SolveSpecularBRDF(const t_Material_BFreq& material, const vec3& faceNormal, const vec3& targetPoint, const CONF_PARTICULE_AGH& shadowRay, const vec3& incomingDirection, Core_ConfigurationAGH* configurationTool)
{
	vec3 specular = SolveSpecularReflection(incomingDirection, faceNormal);
	double specularEnergy = 0;
	l_decimal mu1, mu2;
	float receiverRadius = *configurationTool->FastGetConfigValue(Core_ConfigurationAGH::FPROP_RAYON_RECEPTEURP);

	if (RaySphereIntersection(shadowRay.position, shadowRay.position + specular * 1000, targetPoint, receiverRadius, &mu1, &mu2))
	{
		specularEnergy = (1 - material.diffusion) * (abs(mu2 - mu1) * specular.length() * 1000) / (2 * receiverRadius);
	}

	return  specularEnergy;
}

double BRDFs::SolveSpecularLambertBRDF(const t_Material_BFreq& material, const vec3& faceNormal, const vec3& targetPoint, const CONF_PARTICULE_AGH& shadowRay, const vec3& incomingDirection, Core_ConfigurationAGH* configurationTool)
{
	vec3 specular = SolveSpecularReflection(incomingDirection, faceNormal);
	vec3 toReceiver = targetPoint - shadowRay.position;
	double specularEnergy = 0, lambertEnergy;
	l_decimal mu1, mu2;
	float receiverRadius = *configurationTool->FastGetConfigValue(Core_ConfigurationAGH::FPROP_RAYON_RECEPTEURP);

	if (RaySphereIntersection(shadowRay.position, shadowRay.position + specular * 1000, targetPoint, receiverRadius, &mu1, &mu2))
	{
		specularEnergy = (1 - material.diffusion) * (abs(mu2 - mu1) * specular.length()*1000)/(2*receiverRadius);
	}

	double solidAngle = (M_PI*receiverRadius*receiverRadius) / (toReceiver.length()*toReceiver.length());

	toReceiver.normalize();

	//0.66 is normalization factor used to acount for not treating receiver as sphere 
	//- points far from sphere center ar treated with the same weight as ones in the middle,
	//proper weight schould be proportional to length of intersection, but it is hard to be evaluated quickly
	lambertEnergy = material.diffusion*(1 / M_PI)*solidAngle*faceNormal.dot(toReceiver)*0.66;
	
	return  specularEnergy + lambertEnergy;
}

double BRDFs::SolvePhongBRDF(const t_Material_BFreq& material, const vec3& faceNormal, const vec3& targetPoint, const CONF_PARTICULE_AGH& shadowRay, const vec3& incomingDirection, Core_ConfigurationAGH* configurationTool)
{
	vec3 specular = SolveSpecularReflection(incomingDirection, faceNormal);
	vec3 toReceiver = targetPoint - shadowRay.position;
	Matrix3 rotMatrix;
	double energyFactor = 0, solidAngle;	
	float receiverRadius = *configurationTool->FastGetConfigValue(Core_ConfigurationAGH::FPROP_RAYON_RECEPTEURP);

	//calculate Phong exponent based on experiments (used also in dotreflection - remember to change in both places)
	int n = (int) pow(10, -1.7234170470604733*material.diffusion + 2.6245274102195886);

	//receiver is small and/or it is far from reflection point - evaluated as one point
	//	consider taking n into accont - small n means slowly changing Phong function.

	solidAngle = (M_PI*receiverRadius*receiverRadius) / (toReceiver.length()*toReceiver.length());
	evaluatePhongAtPoint(n, material.diffusion, solidAngle, targetPoint, shadowRay.position, faceNormal, specular, energyFactor);
	return energyFactor*0.66;
	
}

void BRDFs::evaluatePhongAtPoint(int n, float diffusion, double solidAngle, vec3 target, vec3 position, vec3 faceNormal, vec3 specular, double &result)
{
	vec3 toSubSurf = target - position;
	
	toSubSurf.normalize();

	double cosFace = faceNormal.dot(toSubSurf);
	double cosPow = specular.dot(toSubSurf);
	double specEnerg;
	if (cosPow > 0){
		double cosPowN = pow(cosPow, n);
		//double scalingFactor = ((n + 2) / (2 * M_PI));
		double scalingFactor = 1. / calculatePhongNormalizationFactor(faceNormal, specular, n);
		specEnerg = (1. - diffusion) * scalingFactor * cosPowN * cosFace * solidAngle;
		if (isnan(cosFace) || isnan(cosPow) || isnan(scalingFactor)) {
			std::cout << "CosFace:" << cosFace << " cosPow:" << cosPow << " scalingFactor:" << scalingFactor << std::endl;
			std::cout << "Norm Factor:" << calculatePhongNormalizationFactor(faceNormal, specular, n) << std::endl;
			std::cout << "Face normal:" << faceNormal.x <<","<< faceNormal.y<<","<< faceNormal.z << " Specular:" << specular.x<<","<< specular.y << "," << specular.z <<" n" << n << std::endl;
		}
	}
	else {
		specEnerg = 0;
	}
		
	result += diffusion*(1. / M_PI)*solidAngle*cosFace;		//diffuse
	result += (1. - diffusion) * specEnerg;					//specular


}