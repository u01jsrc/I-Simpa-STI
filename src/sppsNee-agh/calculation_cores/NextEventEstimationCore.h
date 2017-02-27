#include "CalculationCoreAGH.h"


#ifndef __NextEventEstimationCore__
#define __NextEventEstimationCore__

class NextEventEstimationCore : public  CalculationCoreSPPS
{

public:
	NextEventEstimationCore(t_Mesh& _sceneMesh, t_TetraMesh& _sceneTetraMesh, CONF_CALCULATION_AGH &_confEnv, Core_ConfigurationAGH &_configurationTool, ReportManagerAGH* _reportTool);

protected:
	virtual void Movement(CONF_PARTICULE_AGH &configurationP) override;
	virtual void FreeParticleTranslation(CONF_PARTICULE_AGH &configurationP, const vec3 &translationVector) override;
	void GenerateShadowRays(CONF_PARTICULE_AGH& particle, t_Material_BFreq * materialInfo, t_cFace * faceInfo,const double& deltaT,const double& distanceToTravel, std::list<CONF_PARTICULE_AGH>& shadowRays, double* probability = nullptr);
};

#endif