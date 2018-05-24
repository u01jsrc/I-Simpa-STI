#include "reportmanagerAGH.h"
#include "tools/collision.h"
#include <algorithm>


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
	for (int i = 0; i < _paramReport.configManager->recepteur_s_List.size(); i++)
		if (_paramReport.configManager->recepteur_s_List[i]->recordAngle)
		{
			t_angle_energy dataSet;
			dataSet.fill_empty_data(*(static_cast<Core_ConfigurationAGH*>(paramReport.configManager))->FastGetConfigValue(Core_ConfigurationAGH::IPROP_EXTENDED_ANGLE_STATS));
			this->surfIncidenceAngleEnergy.push_back(dataSet);
		}

	for (int i = 0; i < _paramReport.sceneModel->pface_size; i++)
	{
		t_cFace& face = _paramReport.sceneModel->pfaces[i];
		auto it = std::find_if(paramReport.configManager->recepteur_s_List.begin(), paramReport.configManager->recepteur_s_List.end(), [face](const r_Surf* surfRec) {return surfRec->idRecepteurS == face.idRecepteurS; });
		if (it != paramReport.configManager->recepteur_s_List.end() && (*it)->recordAngle)
		{
			int key = (*it)->idRecepteurS;
			CoreString name = (*it)->name;
			int idx_surf = std::distance(paramReport.configManager->recepteur_s_List.begin(), it);
			int idx_angle = surfReceiverIDIdxMap.size();
			if(surfReceiverIDIdxMap.insert(std::make_pair(key, std::make_pair(idx_surf, idx_angle) )).second)
				surfIncidenceAngleEnergy[idx_angle].receiverName = name;
		}
		if (surfReceiverIDIdxMap.size() == surfIncidenceAngleEnergy.size())
			break;
	}
}

ReportManagerAGH::~ReportManagerAGH()
{
	//this->SaveAndCloseParticleFile();
	//+delete[] tabEnergyByTimeStep;
}

void ReportManagerAGH::ParticuleFreeTranslation(CONF_PARTICULE_AGH& particleInfos, const vec3& nextPosition)
{
	vec3 direction(nextPosition - particleInfos.position);

	Core_ConfigurationAGH* configManager = static_cast<Core_ConfigurationAGH*>(this->paramReport.configManager);
	if (particleInfos.currentTetra->linkedCutMap && particleInfos.reflectionOrder >= *(configManager->FastGetConfigValue(Core_ConfigurationAGH::IPROP_ANGLE_STATS_MIN_REFL)))
	{
		for (std::vector<r_SurfCut*>::iterator itrs = particleInfos.currentTetra->linkedCutMap->begin(); itrs != particleInfos.currentTetra->linkedCutMap->end(); itrs++)
		{
			float t, u, v;
			if (collision_manager::intersect_parallelogram(particleInfos.position, direction, (*itrs)->Bvert, (*itrs)->Cvert, (*itrs)->Avert, &t, &u, &v) && t >= 0 && t <= 1)
			{
				uentier CellRow = (uentier)floorf(u*(*itrs)->NbCellU);
				uentier CellCol = (uentier)floorf(v*(*itrs)->NbCellV);
				vec3 normal = (*itrs)->planeNormal;
				if (particleInfos.direction.dot(normal) < 0)
					normal *= -1;
				if (*(this->paramReport.configManager->FastGetConfigValue(Core_Configuration::I_PROP_SURFACE_RECEIVER_MODE)) == 0)
					(*itrs)->data[particleInfos.frequenceIndex][CellRow][CellCol][particleInfos.pasCourant] += particleInfos.energie*cosf(normal.angle(particleInfos.direction));
				else
					(*itrs)->data[particleInfos.frequenceIndex][CellRow][CellCol][particleInfos.pasCourant] += particleInfos.energie;
			}
		}
	}
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

void ReportManagerAGH::ParticuleCollideWithSceneMesh(CONF_PARTICULE & particleInfos)
{
	if (particleInfos.nextModelIntersection.idface == -1)
		return;
	t_Tetra_Faces* face = &particleInfos.currentTetra->faces[particleInfos.nextModelIntersection.idface];
	if (face->face_scene == NULL)
		return;

	if (particleInfos.outputToParticleFile && *(this->paramReport.configManager->FastGetConfigValue(Core_Configuration::I_PROP_SAVE_SURFACE_INTERSECTION)))
	{
		//Add intersection to history
		this->collisionHistory.push_back(t_collision_history(face->face_scene->normal, particleInfos.reflectionOrder, particleInfos.nextModelIntersection.collisionPosition, particleInfos.direction, particleInfos.energie));
	}

	Core_ConfigurationAGH* configManager = static_cast<Core_ConfigurationAGH*>(this->paramReport.configManager);
	if (face->recepteurS && particleInfos.reflectionOrder >= *(configManager->FastGetConfigValue(Core_ConfigurationAGH::IPROP_ANGLE_STATS_MIN_REFL)))
	{
		vec3& normal = face->face_scene->normal;
		if (particleInfos.direction.dot(normal)<0) //If the receiver surface is on the other side then revert the normal
			normal *= -1;
		//Add particle energy to receiver
		if (*(this->paramReport.configManager->FastGetConfigValue(Core_Configuration::I_PROP_SURFACE_RECEIVER_MODE)) == 0)
			face->recepteurS->energieRecu[particleInfos.frequenceIndex][particleInfos.pasCourant] += particleInfos.energie*cosf(normal.angle(particleInfos.direction));
		else
			face->recepteurS->energieRecu[particleInfos.frequenceIndex][particleInfos.pasCourant] += particleInfos.energie;

		//Calc angle stats
		std::pair<int,int>& indexes = surfReceiverIDIdxMap[face->face_scene->idRecepteurS];
		if (configManager->recepteur_s_List[indexes.first]->recordAngle)
		{
			this->surfIncidenceAngleEnergy[indexes.second].calc_angle(particleInfos, *face->face_scene);
			this->surfIncidenceAngleEnergy[indexes.second].add_energy(particleInfos);
		}
	}
	particleInfos.reflectionOrder++;
}

void ReportManagerAGH::SaveThreadsStats(const CoreString& filename,const CoreString& filenamedBLvl,std::vector<t_sppsThreadParamAGH>& cols,const t_ParamReport& params)
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

void ReportManagerAGH::GetAngleStats(t_sppsThreadParamAGH& data, bool NormalizeAngleStats)
{
	if (surfIncidenceAngleEnergy.size() == 0) 
		return;

	using namespace formatGABE;
	data.GabeAngleData.reserve(surfIncidenceAngleEnergy.size());

	for (short i = 0; i < surfIncidenceAngleEnergy.size(); i++) {
		surfIncidenceAngleEnergy[i].calc_energy_density();
		if (NormalizeAngleStats) {
			surfIncidenceAngleEnergy[i].normalize_energy_density();
		}

		GABE_Data_Float* statValues = new GABE_Data_Float(surfIncidenceAngleEnergy[i].energy.size());
		statValues->headerData.numOfDigits = 22;
		statValues->SetLabel((CoreString::FromInt(paramReport.freqValue) + " Hz, " + surfIncidenceAngleEnergy[i].receiverName).c_str());

		for (int j = 0; j < surfIncidenceAngleEnergy[i].energy.size(); j++) {
			statValues->Set(j, surfIncidenceAngleEnergy[i].energy[j]);
		}

		data.GabeAngleData.push_back(statValues);
	}
}

void ReportManagerAGH::SaveAngleStats(const CoreString& filename, const CoreString& filenamedBLvl, std::vector<t_sppsThreadParamAGH>& cols, const t_ParamReport& params, bool extended)
{
	/////////////////////////////////////
	// 1: Sauvegarde des statistiques des états finaux des particules
	using namespace formatGABE;
	int size;
	if (!extended) {
		size = 90;
	}
	else {
		size = 90 * 360;
	}

	GABE_Data_ShortString* statLbl = new GABE_Data_ShortString(size);
	statLbl->SetLabel("Angle");

	if (!extended) {
		for (int i = 0; i<90; i++)
		{
			statLbl->SetString(i, (CoreString::FromInt(i + 1)).c_str());
		}
	}
	else {
		int licz = 0;
		for (int phi = -180; phi<180; phi++) {
			for (int theta = 0; theta<90; theta++) {
				statLbl->SetString(licz, ("phi=" + CoreString::FromInt(phi + 1) + ", theta=" + CoreString::FromInt(theta + 1)).c_str());
				licz++;
			}
		}
	}

	uentier nbfreqUsed = 0;
	uentier nbgroupsUsed = 0;
	for (std::size_t idfreq = 0; idfreq<cols.size(); idfreq++)
	{
		if (!cols[idfreq].GabeAngleData.empty()) {
			nbfreqUsed++;
			nbgroupsUsed = cols[idfreq].GabeAngleData.size();
		}
	}


	GABE exportTab(nbfreqUsed*nbgroupsUsed + 1);
	exportTab.LockData();
	exportTab.SetCol(0, statLbl);

	uentier currentIndex = 1;

	for (std::size_t idfreq = 0; idfreq<cols.size(); idfreq++)
	{
		for (int id_gr = 0; id_gr<nbgroupsUsed; id_gr++)
		{
			if (!cols[idfreq].GabeAngleData.empty())
			{
				exportTab.SetCol(currentIndex, *cols[idfreq].GabeAngleData[id_gr]);
				currentIndex++;
			}
		}
	}

	if (nbgroupsUsed != 0) {
		exportTab.Save(filename.c_str());
	}
}
