#include "reportmanagerAGH.h"
#include "tools/collision.h"


using namespace std;

inline bool ContainsRP(t_Recepteur_P* recepteurTest, std::vector<t_Recepteur_P*>* rpList)
{
	for(std::size_t idRP=0;idRP<rpList->size();idRP++)
	{
		if(rpList->at(idRP)==recepteurTest)
			return true;
	}
	return false;
}

ReportManagerAGH::ReportManagerAGH(t_ParamReport& _paramReport)
:ReportManager(_paramReport)
{
}

ReportManagerAGH::~ReportManagerAGH()
{
	//this->SaveAndCloseParticleFile();
	//+delete[] tabEnergyByTimeStep;
}

void ReportManagerAGH::ShadowRayFreeTranslation(CONF_PARTICULE_AGH& particleInfos, const vec3& nextPosition)
{
	vec3 direction(nextPosition - particleInfos.position);

	//Si le prochain tetrahèdre contient un ou des recepteurs ponctuel
	if (particleInfos.currentTetra->linkedRecepteurP)
	{

		for (std::size_t idrecp = 0; idrecp<particleInfos.currentTetra->linkedRecepteurP->size(); idrecp++)
		{
			t_Recepteur_P* currentRecp = particleInfos.currentTetra->linkedRecepteurP->at(idrecp);
			vec3 closestPointDuringPropagation = currentRecp->position.closestPointOnSegment(particleInfos.position, nextPosition);
			if (closestPointDuringPropagation.distance(currentRecp->position)<*paramReport.configManager->FastGetConfigValue(Core_ConfigurationAGH::FPROP_RAYON_RECEPTEURP) && particleInfos.targetReceiver == currentRecp)
			{
				//Calcul de la longueur de croisement
				l_decimal mu1, mu2;

				if (RaySphere(particleInfos.position, nextPosition, currentRecp->position, *paramReport.configManager->FastGetConfigValue(Core_ConfigurationAGH::FPROP_RAYON_RECEPTEURP), &mu1, &mu2))
				{
					if (mu2<0)
						mu2 = 0;
					else if (mu2>1)
						mu2 = 1;
					if (mu1<0)
						mu1 = 0;
					else if (mu1>1)
						mu1 = 1;
					float norm_dir = (direction).length();
					l_decimal Lintersect = abs(mu2 - mu1)*norm_dir;
					if (Lintersect>0)
					{
						l_decimal cosphi = cos(M_PIDIV2 - direction.angle(currentRecp->orientation));
						const l_decimal energy = particleInfos.energie*Lintersect;
						currentRecp->energy_sum[particleInfos.frequenceIndex][particleInfos.pasCourant] += energy;

						lst_rp_lef[currentRecp->idrp].Lf[particleInfos.pasCourant] += energy*pow(cosphi, 2);
						lst_rp_lef[currentRecp->idrp].Lfc[particleInfos.pasCourant] += energy*fabs(cosphi);

						vec3 particleIntensity((particleInfos.direction / particleInfos.direction.length())*energy);
						lst_rp_lef[currentRecp->idrp].intensity[particleInfos.pasCourant] += veci_t(particleIntensity.x, particleIntensity.y, particleIntensity.z);
						if (timeStepInSourceOutput) {
							lst_rp_lef[currentRecp->idrp].SrcContrib[particleInfos.pasCourant*nbSource + particleInfos.sourceid] += energy;
						}
						else {
							lst_rp_lef[currentRecp->idrp].SrcContrib[particleInfos.sourceid] += energy;
						}
						if (particleInfos.outputToParticleFile && *(this->paramReport.configManager->FastGetConfigValue(Core_ConfigurationAGH::I_PROP_SAVE_RECEIVER_INTERSECTION)))
						{
							//Add intersection to history
							decimal time = particleInfos.pasCourant * *this->paramReport.configManager->FastGetConfigValue(Base_Core_Configuration::FPROP_TIME_STEP) + particleInfos.elapsedTime;
							this->receiverCollisionHistory.push_back(t_receiver_collision_history(time, particleInfos.direction, energy * currentRecp->cdt_vol, currentRecp->idrp));
						}
					}


					if (mu1 > mu2) { mu2 = mu1; }

					if (mu2 < 1)
					{
						particleInfos.energie = 0;
						if (particleInfos.stateParticule == PARTICULE_STATE_ALIVE)
							particleInfos.stateParticule = PARTICULE_STATE_SHADOW_RAY_REACHED_DST;
					}
				}
			}
		}
	}
}


formatGABE::GABE_Object* ReportManagerAGH::GetColStats()
{
	using namespace formatGABE;
	GABE_Data_Integer* statValues=new GABE_Data_Integer(8);
	statValues->SetLabel((CoreString::FromInt(paramReport.freqValue)+" Hz").c_str());
	statValues->Set(0,statReport.partAbsAtmo);
	statValues->Set(1,statReport.partAbsSurf);
	statValues->Set(2,statReport.partAbsEncombrement);
	statValues->Set(3,statReport.partLoop);
	statValues->Set(4,statReport.partLost);
	statValues->Set(5,statReport.partAlive);
	statValues->Set(6,statReport.partShadowRay);
	statValues->Set(7,statReport.partTotal);

	return statValues;
}

void ReportManagerAGH::SaveThreadsStats(const CoreString& filename,const CoreString& filenamedBLvl,std::vector<t_sppsThreadParam>& cols,const t_ParamReport& params)
{
	/////////////////////////////////////
	// 1: Sauvegarde des statistiques des états finaux des particules

	using namespace formatGABE;

	GABE_Data_ShortString* statLbl=new GABE_Data_ShortString(8);

	statLbl->SetString(0,"Particles absorbed by the atmosphere");
	statLbl->SetString(1,"Particles absorbed by the materials");
	statLbl->SetString(2,"Particles absorbed by the fittings");
	statLbl->SetString(3,"Particles lost by infinite loops");
	statLbl->SetString(4,"Particles lost by meshing problems");
	statLbl->SetString(5,"Particles remaining at the end of the calculation");
	statLbl->SetString(6,"Schadow rays number");
	statLbl->SetString(7,"Total");
	uentier nbfreqUsed=0;
	for(std::size_t idfreq=0;idfreq<cols.size();idfreq++)
	{
		if(cols[idfreq].GabeColData)
			nbfreqUsed++;
	}
	GABE exportTab(nbfreqUsed+1);
	exportTab.LockData();
	exportTab.SetCol(0,statLbl);

	uentier currentIndex=1;
	for(std::size_t idfreq=0;idfreq<cols.size();idfreq++)
	{
		if(cols[idfreq].GabeColData)
		{
			exportTab.SetCol(currentIndex,cols[idfreq].GabeColData);
			currentIndex++;
		}
	}

	exportTab.Save(filename.c_str());


	/////////////////////////////////////
	// 2: Sauvegarde des statistiques du niveau sonore en fonction du temps (même forme que pour un récepteur ponctuel)
	GABE exportdBLevelTab(nbfreqUsed+1); //+1 pour la colonne des libellés
	//Création de la colonne des libellés
	GABE_Data_ShortString* statdBLbl=new GABE_Data_ShortString(params.nbTimeStep);
	for(uentier idstep=0;idstep<params.nbTimeStep;idstep++)
		statdBLbl->SetString(idstep,(CoreString::FromInt((int)(params.timeStep*(idstep+1)*1000))+" ms").c_str());
	statdBLbl->SetLabel("SPL");
	exportdBLevelTab.SetCol(0,statdBLbl);
	currentIndex=1;
	for(std::size_t idfreq=0;idfreq<cols.size();idfreq++)
	{
		if(cols[idfreq].GabeSumEnergyFreq)
		{
			exportdBLevelTab.SetCol(currentIndex,cols[idfreq].GabeSumEnergyFreq);
			currentIndex++;
		}
	}
	exportdBLevelTab.Save(filenamedBLvl.c_str());
}
