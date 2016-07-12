#pragma once

#include "CalculationCore.h"

class NextEventEstimationCore : public  CalculationCore
{
public:
	NextEventEstimationCore(t_Mesh& _sceneMesh, t_TetraMesh& _sceneTetraMesh, CONF_CALCULATION &_confEnv, Core_Configuration &_configurationTool, ReportManager* _reportTool);
protected:
	void Movement(CONF_PARTICULE &configurationP) override;
	void FreeParticleTranslation(CONF_PARTICULE &configurationP, const vec3 &translationVector) override;
};