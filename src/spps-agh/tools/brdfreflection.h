#include "coreTypes.h"
#include "data_manager/Core_ConfigurationAGH.h"
#include "sppsNeeAGHTypes.h"

class BRDFs
{
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
	static void evaluatePhongAtPoint(int n, float diffusion, float solidAngle, vec3 target, vec3 position, vec3 faceNormal, vec3 specular, double& result);

	static double calculatePhongNormalizationFactor(const vec3 & faceNormal, const vec3 & specularDirection, const int & n);
};