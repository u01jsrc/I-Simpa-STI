#include "brdfreflection.h"


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

std::vector<subSurface> BRDFs::pixelizedCircle10pt;
std::vector<subSurface> BRDFs::pixelizedCircle20pt;
std::vector<subSurface> BRDFs::pixelizedCircle40pt;
BRDFs::Initializer BRDFs::initializer;

BRDFs::Initializer::Initializer()
{
	calcPixelizedReceivers(3.5, pixelizedCircle10pt);
	calcPixelizedReceivers(9.5, pixelizedCircle20pt);
	calcPixelizedReceivers(19.5, pixelizedCircle40pt);
}

vec3 BRDFs::SolveSpecularReflection(vec3 &incomingDirection, vec3 &faceNormal)
{
	vec3 retVal = (incomingDirection - (faceNormal*(incomingDirection.dot(faceNormal)) * 2));
	return retVal / retVal.length();
}

float BRDFs::SolveSpecularLambertBRDF(t_Material_BFreq material, vec3 faceNormal, CONF_PARTICULE& shadowRay, vec3 incomingDirection, Core_Configuration* configurationTool)
{
	vec3 specular = SolveSpecularReflection(incomingDirection, faceNormal);
	vec3 toReceiver = shadowRay.targetReceiver->position - shadowRay.position;
	double specularEnergy = 0, lambertEnergy = 0;
	l_decimal mu1, mu2;
	float receiverRadius = *configurationTool->FastGetConfigValue(Core_Configuration::FPROP_RAYON_RECEPTEURP);

	if (RaySphereIntersection(shadowRay.position, shadowRay.position + specular * 5000, shadowRay.targetReceiver->position, receiverRadius, &mu1, &mu2))
	{
		specularEnergy = (1 - material.diffusion) * abs(mu2 - mu1) * (specular * 5000).length();
	}

	double solidAngle = (M_PI*receiverRadius*receiverRadius) / (toReceiver.length()*toReceiver.length());

	toReceiver.normalize();
	lambertEnergy = material.diffusion*(1 / M_PI)*solidAngle*faceNormal.dot(toReceiver*-1);
	
	return  specularEnergy + lambertEnergy;
}

float BRDFs::SolveBRDFReflection(t_Material_BFreq material, vec3 faceNormal, CONF_PARTICULE &shadowRay, vec3 incomingDirection, Core_Configuration *configurationTool)
{
	switch (*configurationTool->FastGetConfigValue(Core_Configuration::I_PROP_BRDF_REFLECTION_MODEL))
	{
	case 0:
		return SolveSpecularLambertBRDF(material, faceNormal, shadowRay, incomingDirection, configurationTool);

	case 1:
		return 0;

	default:
		return 0;
	}
}

float BRDFs::SolveBRDFPoint(t_Material_BFreq material, vec3 faceNormal, vec3 targetPoint, vec3 incomingDirection)
{
	return 0;
}

void BRDFs::calcPixelizedReceivers(float n_elements, std::vector<subSurface>& pixelSet)
{
	float r2 = n_elements * n_elements;
	float area = r2 * 4;
	int rr = n_elements * 2;

	pixelSet.reserve(r2 * 4);

	float dxy = 1 / (2 * n_elements);

	for (int i = 0; i < area; i++)
	{
		float tx = (i % rr) - n_elements;
		float ty = (i / rr) - n_elements;

		if (tx * tx + ty * ty <= r2) {
			subSurface subFace;
			subFace.center.set(tx / n_elements, ty / n_elements, 0);
			subFace.p1.set(subFace.center.x + dxy, subFace.center.y + dxy, 0);
			subFace.p2.set(subFace.center.x - dxy, subFace.center.y + dxy, 0);
			subFace.p3.set(subFace.center.x - dxy, subFace.center.y - dxy, 0);
			subFace.p4.set(subFace.center.x + dxy, subFace.center.y + dxy, 0);
			subFace.weight = sqrt(1 - (tx * tx + ty * ty) / (n_elements*n_elements));
			pixelSet.push_back(subFace);
		}
	}
}