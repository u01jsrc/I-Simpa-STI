#include "coreTypes.h"
#include "data_manager/core_configuration.h"
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

	static float SolveBRDFReflection(t_Material_BFreq material, vec3 faceNormal, CONF_PARTICULE &shadowRay, vec3 incomingDirection, Core_Configuration *configurationTool);
	/**
		* Samples the BRDF at specified point. Curently using Phong reflection model
		*
		* @param material Face material
		* @param faceNormal Face normal vector
		* @param targetPoint Target point
		* @param incomingDirection Incoming particle direction vector
		*/
	static float SolveBRDFPoint(t_Material_BFreq material, vec3 faceNormal, vec3 targetPoint, vec3 incomingDirection);

	/**
	* Calculates receiver cross-section devided into square elements
	*
	* @param n_elements Number of elements per radius (must be xxx.5)
	* @param pixelSet Output vector of pixels
	*/
	static void calcPixelizedReceivers(float n_elements, std::vector<subSurface>& pixelSet);
	static vec3 SolveSpecularReflection(vec3& incomingDirection, vec3& faceNormal);
	static float SolveSpecularLambertBRDF(t_Material_BFreq material, vec3 faceNormal, CONF_PARTICULE& shadowRay, vec3 incomingDirection, Core_Configuration* configurationTool);
};