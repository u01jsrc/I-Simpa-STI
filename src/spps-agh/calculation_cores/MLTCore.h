#include "NextEventEstimationCore.h"

#ifndef __MLTCore__
#define __MLTCore__

class MLTCore : public  NextEventEstimationCore
{
public:
	MLTCore(t_Mesh& _sceneMesh, t_TetraMesh& _sceneTetraMesh, CONF_CALCULATION_AGH &_confEnv, Core_ConfigurationAGH &_configurationTool, ReportManagerAGH* _reportTool);
	bool RunInitialSeed(CONF_PARTICULE_MLT &inputParticle);
	
	float b = 0; //bias value - auto calculated - start with 0!;

	float pLarge = 0.5;
	float pSpecular = 0.5;
	int mutation_number = 25;

protected:
	std::list <CONF_PARTICULE_MLT> Seeds;

	void Mutate(const CONF_PARTICULE_MLT& input_particle, CONF_PARTICULE_MLT& mutetedParticle) const;
	bool MoveToNextReflection(CONF_PARTICULE_MLT& configurationP, double rndDir1, double rndDir2, double rndTravelProbability, double rndMatAbsorbtion, float deltaT, float distanceToTravel);
	void CastShadowRays(CONF_PARTICULE_MLT& input_particle);
	void DoMutations(CONF_PARTICULE_MLT& inputParticle, int totalParticleNumber);
	virtual bool Run(CONF_PARTICULE_AGH configurationP);
	void Movement(CONF_PARTICULE_MLT &configurationP);
	void Follow(CONF_PARTICULE_MLT& propagationParticle);
	void FreeParticleTranslation(CONF_PARTICULE_AGH &configurationP, const vec3 &translationVector) override;
	bool GetNextCollision(CONF_PARTICULE_MLT &configurationP, double &distance);
	void MutationGenerator(double& value) const;
};

#endif