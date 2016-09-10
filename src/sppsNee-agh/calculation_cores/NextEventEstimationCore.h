#include "CalculationCoreAGH.h"


#ifndef __NextEventEstimationCore__
#define __NextEventEstimationCore__

class NextEventEstimationCore : public  CalculationCore
{

public:
	NextEventEstimationCore(t_Mesh& _sceneMesh, t_TetraMesh& _sceneTetraMesh, CONF_CALCULATION &_confEnv, Core_ConfigurationAGH &_configurationTool, ReportManagerAGH* _reportTool);

protected:
	virtual void Movement(CONF_PARTICULE_AGH &configurationP) override;
	virtual void FreeParticleTranslation(CONF_PARTICULE_AGH &configurationP, const vec3 &translationVector) override;
};

#endif