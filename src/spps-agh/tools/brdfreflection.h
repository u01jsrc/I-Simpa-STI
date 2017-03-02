#include "coreTypes.h"
#include "data_manager/Core_ConfigurationAGH.h"
#include "sppsNeeAGHTypes.h"


/**
* Surface element used when splitting receiver into subsegments.
* Such approach simplifies BRDF sampling.
*/
struct subSurface
{
public:
	vec3 p1, p2, p3, p4, center;	//geometry of the subsurface element
	double SolidAngle;				//solid angle subattended by the element
	double weight;					//element weight - provides consistency with intersection length checking
};

class BRDFs
{
	static std::vector<subSurface> pixelizedCircle10pt, pixelizedCircle20pt, pixelizedCircle40pt;
public:

	class Initializer
	{
	public:
		Initializer();
	};
	friend class Initializer;
	static Initializer initializer;

	/**
	* Solves reflecion using BRDF specified in simulation configuration
	*
	* @param material Material informations
	* @param faceNormal Normal of scene face
	* @param shadowRay Shadow ray to be casted
	* @param incomingDirection Direction of incoming ray
	* @param configurationTool Simulation configuration
	*/
	static float SolveBRDFReflection(const t_Material_BFreq& material, const vec3& faceNormal,const vec3& targetPoint, const CONF_PARTICULE_AGH &shadowRay, const vec3& incomingDirection, Core_ConfigurationAGH *configurationTool);

	/**
	* Calculates receiver cross-section devided into square elements
	*
	* @param n_elements Number of elements per radius (must be xxx.5)
	* @param pixelSet Output vector of pixels
	*/
	static void calcPixelizedReceivers(float n_elements, std::vector<subSurface>& pixelSet);

	/**
	* Calculates specular reflection direction
	*
	* @param incomingDirection Direction of incoming ray
	* @param faceNormal Normal of scene face
	*/
	static vec3 SolveSpecularReflection(const vec3& incomingDirection, const vec3& faceNormal);

	static float SolveSpecularBRDF(const t_Material_BFreq& material, const vec3& faceNormal, const vec3& targetPoint, const CONF_PARTICULE_AGH & shadowRay, const vec3& incomingDirection, Core_ConfigurationAGH * configurationTool);

	/**
	* Solves reflecion energy using Specular+Lambert BRDF
	*
	* @param material Material informations
	* @param faceNormal Normal of scene face
	* @param shadowRay Shadow ray to be casted
	* @param incomingDirection Direction of incoming ray
	* @param configurationTool Simulation configuration
	*/
	static float SolveSpecularLambertBRDF(const t_Material_BFreq& material,const vec3& faceNormal,const vec3& targetPoint, const CONF_PARTICULE_AGH& shadowRay, const vec3& incomingDirection, Core_ConfigurationAGH* configurationTool);

	/**
	* Solves reflecion energy using Phong BRDF
	* n factor was evaluated experimentaly
	*
	* @param material Material informations
	* @param faceNormal Normal of scene face
	* @param shadowRay Shadow ray to be casted
	* @param incomingDirection Direction of incoming ray
	* @param configurationTool Simulation configuration
	*/
	static float SolvePhongBRDF(const t_Material_BFreq& material, const vec3& faceNormal, const vec3& targetPoint, const CONF_PARTICULE_AGH& shadowRay, const vec3& incomingDirection, Core_ConfigurationAGH* configurationTool);
	static void evaluatePhongAtPoint(float n, float diffusion, float solidAngle, float weight, vec3 target, vec3 position, vec3 faceNormal, vec3 specular, vec3 subFaceNormal, double& result);
};