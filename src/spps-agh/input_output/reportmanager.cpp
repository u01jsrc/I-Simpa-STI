#include "reportmanager.h"
#include "tools/collision.h"
#include <iostream>
#include <fstream>

const l_decimal p_0=1/pow((float)(20*pow(10.f,(int)-6)),(int)2);

l_decimal to_deciBel(const l_decimal& wjVal,const l_decimal&  Rho)
{
	return 10*log10(Rho*wjVal*p_0);
}


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
	int RaySphere(const vec3& p1,const vec3& p2,const vec3& sc,double r,double *mu1,double *mu2)
	{
	   double a,b,c;
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

ReportManager::ReportManager(t_ParamReport& _paramReport)
:BaseReportManager()
{
	paramReport=_paramReport;
	nbSource = paramReport.configManager->srcList.size();
	timeStepInSourceOutput = *paramReport.configManager->FastGetConfigValue(Core_Configuration::I_PROP_OUTPUT_RECEIVER_BY_SOURCE);
	tabEnergyByTimeStep=new l_decimal[_paramReport.nbTimeStep];
	memset(tabEnergyByTimeStep,0,sizeof(l_decimal)*_paramReport.nbTimeStep);
	lst_rp_lef.insert(lst_rp_lef.begin(),_paramReport.configManager->recepteur_p_List.size(),t_rp_lef());
	for(uentier idrp=0;idrp<lst_rp_lef.size();idrp++)
	{
		lst_rp_lef[idrp].Init(_paramReport.nbTimeStep,nbSource, timeStepInSourceOutput);
	}

	particleFile = NULL;
	particleSurfaceCSVFile = NULL;
	particleReceiverCSVFile = NULL;
	lastParticuleFileHeaderInfo=0;

	if(paramReport.nbParticles!=0)
		writeParticleFile();
	
}

void ReportManager::writeParticleFile()
{
	if(particleFile)
		delete particleFile;
	if(particleSurfaceCSVFile)
		delete particleSurfaceCSVFile;
	if (particleReceiverCSVFile)
		delete particleReceiverCSVFile;
	//Création du dossier de particule
	//_mkdir(paramReport._particlePath.c_str());
	st_mkdir(paramReport._particlePath.c_str());
	stringClass freqFolder;
	freqFolder=paramReport._particlePath+stringClass::FromInt(paramReport.freqValue)+stringClass("\\");
	//Création du dossier de fréquence
	//_mkdir(freqFolder.c_str());
	st_mkdir(freqFolder.c_str());
	stringClass fileNamePath=freqFolder+paramReport._particleFileName;
	particleFile = new fstream(fileNamePath.c_str() , ios::out | ios::binary);
	stringClass fileCSVNamePath=freqFolder+"particle_surface_collision_statistics.csv";
	stringClass fileReceiversCSVNamePath = freqFolder + "particle_receivers_collision_statistics.csv";
	if(*(this->paramReport.configManager->FastGetConfigValue(Core_Configuration::I_PROP_SAVE_SURFACE_INTERSECTION))) {
		particleSurfaceCSVFile = new fstream(fileCSVNamePath.c_str() , ios::out);
	}
	if (*(this->paramReport.configManager->FastGetConfigValue(Core_Configuration::I_PROP_SAVE_RECEIVER_INTERSECTION))) {
		particleReceiverCSVFile = new fstream(fileReceiversCSVNamePath.c_str(), ios::out);
	}

	enteteSortie.nbParticles=paramReport.nbParticles;
	enteteSortie.nbTimeStepMax=paramReport.nbTimeStep;
	nbPasDeTempsMax=paramReport.nbTimeStep;
	enteteSortie.particleInfoLength=sizeof(binaryPTimeStep);
	enteteSortie.particleHeaderInfoLength=sizeof(binaryPHeader);
	enteteSortie.fileInfoLength=sizeof(binaryFHeader);
	enteteSortie.formatVersion=PARTICLE_BINARY_VERSION_INFORMATION;
	enteteSortie.timeStep=paramReport.timeStep;
	lastParticuleFileHeaderInfo=particleFile->tellp();
	particleFile->write((char*)&enteteSortie,sizeof(binaryFHeader));
	*particleSurfaceCSVFile<<"id,collision coordinate,face normal,reflection order,incident vector,energy"<<std::endl;
	*particleReceiverCSVFile << "time(s),receiver name,incident vector x,incident vector y,incident vector z,energy * dist" << std::endl;
	realNbParticle=0;

}


ReportManager::~ReportManager()
{
	this->SaveAndCloseParticleFile();
	delete[] tabEnergyByTimeStep;
}

void ReportManager::RecordTimeStep(CONF_PARTICULE& particleInfos)
{
	if(particleInfos.outputToParticleFile )
	{
		if(firstTimeStep==-1)
			firstTimeStep=particleInfos.pasCourant;
		positionsCurrentParticule.push_back(binaryPTimeStep(particleInfos.position,particleInfos.energie));
	}
	if(particleInfos.pasCourant<paramReport.nbTimeStep)
	{
		tabEnergyByTimeStep[particleInfos.pasCourant]+=particleInfos.energie;
	}
}

void ReportManager::ParticuleFreeTranslation(CONF_PARTICULE& particleInfos, const vec3& nextPosition)
{
	vec3 direction(nextPosition-particleInfos.position);
	//Si dans le modèle un récepteur plan a été déclaré


	if(particleInfos.currentTetra->linkedCutMap)
	{
		for(std::vector<r_SurfCut*>::iterator itrs=particleInfos.currentTetra->linkedCutMap->begin();itrs!=particleInfos.currentTetra->linkedCutMap->end();itrs++)
		{
			float t,u,v;
			if(collision_manager::intersect_parallelogram(particleInfos.position,direction,(*itrs)->Bvert,(*itrs)->Cvert,(*itrs)->Avert,&t,&u,&v) && t>=0 && t<=1)
			{
				uentier CellRow=(uentier)floorf(u*(*itrs)->NbCellU);
				uentier CellCol=(uentier)floorf(v*(*itrs)->NbCellV);
				vec3 normal=(*itrs)->planeNormal;
				if(particleInfos.direction.dot(normal)<0) //si la face du recepteur est orientée dans l'autre direction on inverse la normal
					normal*=-1;
				if(*(this->paramReport.configManager->FastGetConfigValue(Core_Configuration::I_PROP_SURFACE_RECEIVER_MODE))==0)
					(*itrs)->data[particleInfos.frequenceIndex][CellRow][CellCol][particleInfos.pasCourant]+=particleInfos.energie*cosf(normal.angle(particleInfos.direction));
				else
					(*itrs)->data[particleInfos.frequenceIndex][CellRow][CellCol][particleInfos.pasCourant]+=particleInfos.energie;
			}
		}
	}

	//Si le prochain tetrahèdre contient un ou des recepteurs ponctuel
	if(particleInfos.currentTetra->linkedRecepteurP)
	{

		for(std::size_t idrecp=0;idrecp<particleInfos.currentTetra->linkedRecepteurP->size();idrecp++)
		{
			t_Recepteur_P* currentRecp=particleInfos.currentTetra->linkedRecepteurP->at(idrecp);
			vec3 closestPointDuringPropagation=currentRecp->position.closestPointOnSegment(particleInfos.position,nextPosition);
			if(closestPointDuringPropagation.distance(currentRecp->position)<*paramReport.configManager->FastGetConfigValue(Core_Configuration::FPROP_RAYON_RECEPTEURP))
			{
				//Calcul de la longueur de croisement
				l_decimal mu1,mu2;

				if(RaySphere(particleInfos.position,nextPosition,currentRecp->position,*paramReport.configManager->FastGetConfigValue(Core_Configuration::FPROP_RAYON_RECEPTEURP),&mu1,&mu2))
				{
					if(mu2<0)
						mu2=0;
					else if(mu2>1)
						mu2=1;
					if(mu1<0)
						mu1=0;
					else if(mu1>1)
						mu1=1;
					float norm_dir=(direction).length();
					l_decimal Lintersect=abs(mu2-mu1)*norm_dir;
					if(Lintersect>0)
					{
						l_decimal cosphi=cos(M_PIDIV2-direction.angle(currentRecp->orientation));
						const l_decimal energy=particleInfos.energie*Lintersect;
						currentRecp->energy_sum[particleInfos.frequenceIndex][particleInfos.pasCourant]+=energy;

						lst_rp_lef[currentRecp->idrp].Lf[particleInfos.pasCourant]+=energy*pow(cosphi,2);
						lst_rp_lef[currentRecp->idrp].Lfc[particleInfos.pasCourant]+=energy*fabs(cosphi);
						
						
						//Calc receiver incidence angle for auralization
						if (*(this->paramReport.configManager->FastGetConfigValue(Core_Configuration::IPROP_EXPORT_RECEIVER_INCIDENCE_ANGLE)))
						{
							double phi = (atan2(-particleInfos.direction.y, -particleInfos.direction.x) - currentRecp->orientation_sph.z) * 180 / M_PI;
							double theta = (asin(particleInfos.direction.z / particleInfos.direction.length()) - currentRecp->orientation_sph.y) * 180 / M_PI;

							if (phi > 180) {
								phi = -(360 - phi);
							}
							else if (phi < -180) {
								phi = 360 + phi;
							}

							if (theta > 90) {
								phi = (180 - phi);
							}
							else if (theta < -90) {
								phi = -180 - phi;
							}
							lst_rp_lef[currentRecp->idrp].theta[particleInfos.pasCourant] += theta*energy;
							lst_rp_lef[currentRecp->idrp].phi[particleInfos.pasCourant] += phi*energy;
							lst_rp_lef[currentRecp->idrp].en[particleInfos.pasCourant] += energy;
						}

						vec3 particleIntensity((particleInfos.direction/particleInfos.direction.length())*energy);
						lst_rp_lef[currentRecp->idrp].intensity[particleInfos.pasCourant]+=veci_t(particleIntensity.x,particleIntensity.y,particleIntensity.z);
						if(timeStepInSourceOutput) {
							lst_rp_lef[currentRecp->idrp].SrcContrib[particleInfos.pasCourant*nbSource + particleInfos.sourceid]+=energy;
						} else {
							lst_rp_lef[currentRecp->idrp].SrcContrib[particleInfos.sourceid]+=energy;
						}
						if (particleInfos.outputToParticleFile && *(this->paramReport.configManager->FastGetConfigValue(Core_Configuration::I_PROP_SAVE_RECEIVER_INTERSECTION)))
						{
							//Add intersection to history
							decimal time = particleInfos.pasCourant * *this->paramReport.configManager->FastGetConfigValue(Base_Core_Configuration::FPROP_TIME_STEP) + particleInfos.elapsedTime;
							this->receiverCollisionHistory.push_back(t_receiver_collision_history(time, particleInfos.direction, energy * currentRecp->cdt_vol, currentRecp->idrp));
						}
					}
				}
			}
		}
	}
}
void ReportManager::ParticuleGoToNextTetrahedra(CONF_PARTICULE& particleInfos,t_Tetra* nextTetra)
{

}


void ReportManager::ParticuleCollideWithSceneMesh(CONF_PARTICULE& particleInfos)
{
	if(particleInfos.nextModelIntersection.idface==-1)
		return;


	//Retrieve face data ptr
	#ifdef UTILISER_MAILLAGE_OPTIMISATION
	t_Tetra_Faces* face=&particleInfos.currentTetra->faces[particleInfos.nextModelIntersection.idface];
	//Si la face est associée à un récepteur surfacique
	vec3& normal=face->face_scene->normal;
	if(particleInfos.outputToParticleFile && *(this->paramReport.configManager->FastGetConfigValue(Core_Configuration::I_PROP_SAVE_SURFACE_INTERSECTION)) && face->face_scene!=NULL)
	{
		//particleInfos.reflectionOrder++;
		//Add intersection to history
		this->collisionHistory.push_back(t_collision_history(face->face_scene->normal,particleInfos.reflectionOrder,particleInfos.nextModelIntersection.collisionPosition,particleInfos.direction, particleInfos.energie));
	}else if(face->face_scene!=NULL){
		particleInfos.reflectionOrder++;
	}

	#else
	t_cFace* face=&paramReport.sceneModel->pfaces[particleInfos.nextModelIntersection.idface];
	vec3& normal=face->normal;
	#endif

	if(face->face_scene!=NULL && face->face_scene->Rec_angle==true)
	{	
		if(this->angle_energy.size() < face->face_scene->angle_group){
			int tmp=angle_energy.size();
			this->angle_energy.resize(face->face_scene->angle_group);
			for(int i=tmp;i<face->face_scene->angle_group;i++)
				this->angle_energy[i].fill_empty_data(*(this->paramReport.configManager->FastGetConfigValue(Core_Configuration::IPROP_EXTENDED_ANGLE_STATS)));
		}	
		this->angle_energy[face->face_scene->angle_group-1].calc_angle(particleInfos,*face->face_scene);
		this->angle_energy[face->face_scene->angle_group-1].add_energy(particleInfos);
	}

	if(face->recepteurS)
	{
		if(particleInfos.direction.dot(normal)<0) //si la face du recepteur est orienté dans l'autre direction on inverse la normal
			normal*=-1;
		//Ajout de l'energie à la face
		if(*(this->paramReport.configManager->FastGetConfigValue(Core_Configuration::I_PROP_SURFACE_RECEIVER_MODE))==0)
			face->recepteurS->energieRecu[particleInfos.frequenceIndex][particleInfos.pasCourant]+=particleInfos.energie*cosf(normal.angle(particleInfos.direction));
		else
			face->recepteurS->energieRecu[particleInfos.frequenceIndex][particleInfos.pasCourant]+=particleInfos.energie;
	}
}

void ReportManager::SaveParticule()
{
	CloseLastParticleHeader();
}
void ReportManager::CloseLastParticleHeader()
{
	if(!positionsCurrentParticule.empty())
	{
		realNbParticle++;
		binaryPHeader pHeaderInfo(positionsCurrentParticule.size(),firstTimeStep);
		particleFile->write((char*)&pHeaderInfo,sizeof(binaryPHeader));
		for(uentier idtime=0;idtime<pHeaderInfo.nbTimeStep;idtime++)
			particleFile->write( (char*)&positionsCurrentParticule[idtime],sizeof(binaryPTimeStep));
		positionsCurrentParticule.clear();
	}
	if(!this->collisionHistory.empty())
	{
		//Update CSV file
		if(this->particleSurfaceCSVFile!=NULL)
		{
			while(!this->collisionHistory.empty())
			{
				t_collision_history& part_event=this->collisionHistory.front();
				*this->particleSurfaceCSVFile<<this->realNbParticle<<","
					<<part_event.collisionCoordinate.x<<" "<<part_event.collisionCoordinate.y<<" "<<part_event.collisionCoordinate.z<<","
					<<part_event.faceNormal.x<<" "<<part_event.faceNormal.y<<" "<<part_event.faceNormal.z<<","
					<<part_event.reflexionOrder<<","
					<<part_event.incidentVector.x<<" "<<part_event.incidentVector.y<<" "<<part_event.incidentVector.z<<","<<part_event.energy
					<<std::endl;
				this->collisionHistory.pop_front();
			}
		}
	}
	if (!this->receiverCollisionHistory.empty())
	{
		//Update CSV file
		if (this->particleReceiverCSVFile != NULL)
		{
			while (!this->receiverCollisionHistory.empty())
			{
				t_receiver_collision_history& part_event = this->receiverCollisionHistory.front();
				*this->particleReceiverCSVFile << part_event.time << "," << this->paramReport.configManager->recepteur_p_List.at(part_event.idrp)->lblRp << "," << part_event.incidentVector.x << "," << part_event.incidentVector.y << "," << part_event.incidentVector.z << "," << part_event.energy
					<< std::endl;
				this->receiverCollisionHistory.pop_front();
			}
		}
	}
}

void ReportManager::CloseLastParticleFileHeader()
{
	particleFile->seekp(lastParticuleFileHeaderInfo);
	enteteSortie.nbParticles=realNbParticle;
	particleFile->write((char*)&enteteSortie,sizeof(binaryFHeader));
}

formatGABE::GABE_Object* ReportManager::GetColStats()
{
	using namespace formatGABE;
	GABE_Data_Float* statValues=new GABE_Data_Float(8);
	statValues->headerData.numOfDigits=2;
	statValues->SetLabel((CoreString::FromInt(paramReport.freqValue)+" Hz").c_str());
	statValues->Set(0,statReport.partAbsAtmo);
	statValues->Set(1,statReport.partAbsSurf);
	statValues->Set(2,statReport.partAbsEncombrement);
	statValues->Set(3,statReport.partLoop);
	statValues->Set(4,statReport.partLost);
	statValues->Set(5,statReport.partAlive);
	statValues->Set(6,statReport.partTotal);
	statValues->Set(7,statReport.sumaric_time*paramReport.timeStep*343/(statReport.sumaric_reflections));

	return statValues;
}

bool ReportManager::GetAngleStats(t_sppsThreadParam& data, bool NormalizeAngleStats)
{
	if(angle_energy.size()==0){
		data.AngleGroupNum=0;
		return 1;
	}

	else{
		using namespace formatGABE;
		GABE_Data_Float** statValues=new GABE_Data_Float*[angle_energy.size()];

		for(short i=0;i<angle_energy.size();i++){
			angle_energy[i].calc_energy_density();
			if(NormalizeAngleStats){
				angle_energy[i].normalize_energy_density();
			}

			statValues[i]=new GABE_Data_Float(angle_energy[i].energy.size());
			statValues[i]->headerData.numOfDigits=22;
			statValues[i]->SetLabel((CoreString::FromInt(paramReport.freqValue)+" Hz, group "+CoreString::FromInt(i+1)).c_str());

			for(int j=0;j<angle_energy[i].energy.size();j++){
				statValues[i]->Set(j,angle_energy[i].energy[j]);
			}
		}
		data.GabeAngleData=statValues;
		data.AngleGroupNum=angle_energy.size();
	}
	return 0;
}

	
void ReportManager::FillWithLefData(t_sppsThreadParam& data)
{
	using namespace formatGABE;
	//Pour chaque récepteur ponctuel rempli les données récoltés au cours de la propagation
	data.GabeSumEnergyCosPhi.reserve(lst_rp_lef.size());
	data.GabeSumEnergyCosSqrtPhi.reserve(lst_rp_lef.size());

	for(uentier idrecp=0;idrecp<lst_rp_lef.size();idrecp++)
	{
		float volRp=(pow(*this->paramReport.configManager->FastGetConfigValue(Core_Configuration::FPROP_RAYON_RECEPTEURP),3)*M_PI*4.)/3.;
		t_rp_lef* currentRp=&lst_rp_lef[idrecp];
		t_Recepteur_P* currentConfigRp=paramReport.configManager->recepteur_p_List[idrecp];
		GABE_Data_Float* energyCosSqrtPhi=new GABE_Data_Float(paramReport.nbTimeStep);
		GABE_Data_Float* energyCosPhi=new GABE_Data_Float(paramReport.nbTimeStep);
		GABE_Data_Float* GabeIncidence[2];
		GABE_Data_Float* GabeIntensity[3];
		GABE_Data_Float* energyBySrc=new GABE_Data_Float(paramReport.configManager->srcList.size());
		for(short dim=0;dim<2;dim++)
		{
			GabeIncidence[dim]=new GABE_Data_Float(paramReport.nbTimeStep);
		}

		for(short dim=0;dim<3;dim++)
		{
			GabeIntensity[dim]=new GABE_Data_Float(paramReport.nbTimeStep+1); //+1 pour le cumul
		}

		CoreString lblcol=(CoreString::FromInt(paramReport.freqValue));
		GabeIntensity[0]->SetLabel((lblcol+" Hz\nx").c_str());
		GabeIntensity[1]->SetLabel((lblcol+" Hz\ny").c_str());
		GabeIntensity[2]->SetLabel((lblcol+" Hz\nz").c_str());

		GabeIncidence[0]->SetLabel((lblcol+" theta").c_str());
		GabeIncidence[1]->SetLabel((lblcol+" phi").c_str());

		energyBySrc->SetLabel((lblcol+" Hz").c_str());

		dvec3 cumulIntensity;
		for(uentier idstep=0;idstep<paramReport.nbTimeStep;idstep++)
		{
			energyCosSqrtPhi->Set(idstep,currentRp->Lfc[idstep]*currentConfigRp->cdt_vol);
			energyCosPhi->Set(idstep,currentRp->Lf[idstep]*currentConfigRp->cdt_vol);

			GabeIncidence[0]->Set(idstep,currentRp->theta[idstep]/currentRp->en[idstep]);
			GabeIncidence[1]->Set(idstep,currentRp->phi[idstep]/currentRp->en[idstep]);

			GabeIntensity[0]->Set(idstep,currentRp->intensity[idstep].x/volRp);
			GabeIntensity[1]->Set(idstep,currentRp->intensity[idstep].y/volRp);
			GabeIntensity[2]->Set(idstep,currentRp->intensity[idstep].z/volRp);
			cumulIntensity+=currentRp->intensity[idstep]/volRp;
		}
		GabeIntensity[0]->Set(paramReport.nbTimeStep,cumulIntensity[0]);
		GabeIntensity[1]->Set(paramReport.nbTimeStep,cumulIntensity[1]);
		GabeIntensity[2]->Set(paramReport.nbTimeStep,cumulIntensity[2]);
		for(uentier idsrc=0;idsrc<energyBySrc->GetSize();idsrc++)
		{
			l_decimal sum = 0;
			// If the energy was stored by timestep, sum all results
			if(this->timeStepInSourceOutput) {
				for(uentier idstep=0;idstep<paramReport.nbTimeStep;idstep++)
				{
					sum += currentRp->SrcContrib[idstep * nbSource + idsrc]; 	
				}
			} else {
				sum = currentRp->SrcContrib[idsrc];
			}
			energyBySrc->Set(idsrc,sum*currentConfigRp->cdt_vol);
		}
		data.GabeSumEnergyCosPhi.push_back(energyCosPhi);
		data.GabeSumEnergyCosSqrtPhi.push_back(energyCosSqrtPhi);

		data.GabeAngleInc[0].push_back(GabeIncidence[0]);
		data.GabeAngleInc[1].push_back(GabeIncidence[1]);		

		data.GabeIntensity[0].push_back(GabeIntensity[0]);
		data.GabeIntensity[1].push_back(GabeIntensity[1]);
		data.GabeIntensity[2].push_back(GabeIntensity[2]);
		data.GabeSlPerSrc.push_back(energyBySrc);

		if(this->timeStepInSourceOutput) {
			l_decimal* srcContribCopy = new l_decimal[nbSource*paramReport.nbTimeStep];
			memcpy(srcContribCopy, currentRp->SrcContrib, sizeof(l_decimal) * nbSource * paramReport.nbTimeStep); 
			data.SrcContrib.push_back(srcContribCopy);
		}
	}
}

formatGABE::GABE_Data_Float* ReportManager::GetSumEnergy()
{
	l_decimal cdtRho=(*(paramReport.configManager->FastGetConfigValue(Base_Core_Configuration::FPROP_RHO)))*(*(paramReport.configManager->FastGetConfigValue(Base_Core_Configuration::FPROP_CELERITE)));
	using namespace formatGABE;
	GABE_Data_Float* statValues=new GABE_Data_Float(paramReport.nbTimeStep);
	statValues->SetLabel((CoreString::FromInt(paramReport.freqValue)+" Hz").c_str());

	for(uentier idstep=0;idstep<paramReport.nbTimeStep;idstep++)
	{
		statValues->Set(idstep,(Floatb)(tabEnergyByTimeStep[idstep]*cdtRho));
	}
	return statValues;
}
void ReportManager::SaveAndCloseParticleFile()
{
	//On complète l'entete du fichier
	if(particleFile!=NULL)
	{
		CloseLastParticleFileHeader();
		//Fermeture du fichier de particules
		if(particleFile)
		{
			particleFile->close();
			fstream* tmp=particleFile;
			particleFile=NULL;
			delete tmp;
		}
	}
	if(particleSurfaceCSVFile!=NULL)
	{
		particleSurfaceCSVFile->close();
		fstream* tmp=particleSurfaceCSVFile;
		particleSurfaceCSVFile=NULL;
		delete tmp;
	}
	if (particleReceiverCSVFile != NULL)
	{
		particleReceiverCSVFile->close();
		fstream* tmp = particleReceiverCSVFile;
		particleReceiverCSVFile = NULL;
		delete tmp;
	}
}

void ReportManager::NewParticule(CONF_PARTICULE& particleInfos)
{
	if(particleFile!=NULL && particleInfos.outputToParticleFile)
	{
		//Dump old data, save 
		positionsCurrentParticule.clear();
		positionsCurrentParticule.reserve(paramReport.nbTimeStep);
		firstTimeStep=-1;
	}
}

void ReportManager::SaveAngleStats(const CoreString& filename,const CoreString& filenamedBLvl,std::vector<t_sppsThreadParam>& cols,const t_ParamReport& params, bool extended)
{
	/////////////////////////////////////
	// 1: Sauvegarde des statistiques des états finaux des particules
	using namespace formatGABE;
	int size;
	if(!extended){
		size=90;
	}else{
		size=90*360;
	}

	GABE_Data_ShortString* statLbl=new GABE_Data_ShortString(size);
	statLbl->SetLabel("Angle");

	if(!extended){
		for(int i=0; i<90; i++)
		{
			statLbl->SetString(i,(CoreString::FromInt(i+1)).c_str());
		}
	}else{
		int licz=0;
		for(int phi=-180; phi<180; phi++){
			for(int theta=0; theta<90; theta++){
				statLbl->SetString(licz,("phi="+CoreString::FromInt(phi+1)+", theta="+CoreString::FromInt(theta+1)).c_str());
				licz++;
			}
		}
	}

	uentier nbfreqUsed=0;
	uentier nbgroupsUsed=0;
	for(std::size_t idfreq=0;idfreq<cols.size();idfreq++)
	{
		if(cols[idfreq].GabeAngleData){
			nbfreqUsed++;
			nbgroupsUsed=cols[idfreq].AngleGroupNum;
		}
	}


	GABE exportTab(nbfreqUsed*nbgroupsUsed+1);
	exportTab.LockData();
	exportTab.SetCol(0,statLbl);

	uentier currentIndex=1;

	for(std::size_t idfreq=0;idfreq<cols.size();idfreq++)
	{
		for(int id_gr=0;id_gr<nbgroupsUsed;id_gr++)
		{
			if(cols[idfreq].GabeAngleData)
			{
				exportTab.SetCol(currentIndex,cols[idfreq].GabeAngleData[id_gr]);
				currentIndex++;
			}
		}
	}

	if(nbgroupsUsed!=0){
		exportTab.Save(filename.c_str());
	}
}


void ReportManager::SaveThreadsStats(const CoreString& filename,const CoreString& filenamedBLvl,std::vector<t_sppsThreadParam>& cols,const t_ParamReport& params)
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
	statLbl->SetString(6,"Total");
	statLbl->SetString(7,"Mean free path");
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
void ReportManager::SetPostProcessSurfaceReceiver(Core_Configuration& coreConfig ,const std::size_t& freqIndex,std::vector<r_Surf*>& surface_receiver_list,const float& timeStep)
{
	if(*coreConfig.FastGetConfigValue(Core_Configuration::I_PROP_SURFACE_RECEIVER_MODE)==1)
	{
		float i0_div_p0sqr=pow(10.f,-12.f)/pow((float)(20*pow(10.f,(int)-6)),(int)2)*(*coreConfig.FastGetConfigValue(Core_Configuration::FPROP_RHO))*(*coreConfig.FastGetConfigValue(Core_Configuration::FPROP_CELERITE));
		for(std::vector<r_Surf*>::iterator itrs=surface_receiver_list.begin();itrs!=surface_receiver_list.end();itrs++)
		{
			r_Surf& currentRs=**itrs;
			for(unsigned int idFace=0;idFace<currentRs.nbFaces;idFace++)
			{
				r_Surf_Face& curFace(currentRs.faces[idFace]);
				for(std::size_t idTimeStep=0;idTimeStep<curFace.nbtimestep;idTimeStep++)
					curFace.energieRecu[freqIndex][idTimeStep]*=i0_div_p0sqr;
			}
		}
	}
}
void ReportManager::SetPostProcessCutSurfaceReceiver(Core_Configuration& coreConfig ,const std::size_t& freqIndex,std::vector<r_SurfCut*>& surface_receiver_list,const float& timeStep)
{
	if(*coreConfig.FastGetConfigValue(Core_Configuration::I_PROP_SURFACE_RECEIVER_MODE)==1)
	{
		float i0_div_p0sqr(pow(10.f,-12.f)/pow((float)(20*pow(10.f,(int)-6)),(int)2)*(*coreConfig.FastGetConfigValue(Core_Configuration::FPROP_RHO))*(*coreConfig.FastGetConfigValue(Core_Configuration::FPROP_CELERITE)));
		for(std::vector<r_SurfCut*>::iterator itrs=surface_receiver_list.begin();itrs!=surface_receiver_list.end();itrs++)
		{
			r_SurfCut& currentRs(**itrs);
			//Ajout des noeuds
			for(uentier idrow=0;idrow<currentRs.nbrows;idrow++)
			{
				for(uentier idcol=0;idcol<currentRs.nbcols;idcol++)
				{
					for(unsigned int idTimeStep=0;idTimeStep<currentRs.nbtimestep;idTimeStep++)
					{
						currentRs.data[freqIndex][idrow][idcol][idTimeStep]*=i0_div_p0sqr;
					}
				}
			}
		}
	}
}


void ReportManager::SaveSoundLevelBySource(const CoreString& filename,std::vector<t_sppsThreadParam>& cols,const t_ParamReport& params)
{
	using namespace formatGABE;
	uentier nbfreqUsed=0;
	uentier nbfreqMax=params.configManager->freqList.size();
	for(std::size_t idfreq=0;idfreq<cols.size();idfreq++)
	{
		if(cols[idfreq].GabeColData)
			nbfreqUsed++;
	}

	////////////////////////////////////////////
	// Série Libellé des sources
	////////////////////////////////////////////
	GABE_Data_ShortString collbl(params.configManager->srcList.size());
	for(uentier idsrc=0;idsrc<collbl.GetSize();idsrc++)
		collbl.SetString(idsrc,params.configManager->srcList[idsrc]->sourceName.c_str());
	collbl.SetLabel("SPL");

	//Pour chaque récepteur ponctuel
	for(uentier idrecp=0;idrecp<params.configManager->recepteur_p_List.size();idrecp++)
	{
		t_Recepteur_P* currentRP=params.configManager->recepteur_p_List[idrecp];
		GABE recepteurPonctData(nbfreqUsed+1);
		recepteurPonctData.SetCol(0,collbl);
		int idcol=1;
		for(uentier idfreq=0;idfreq<nbfreqMax;idfreq++)
		{
			if(params.configManager->freqList[idfreq]->doCalculation)
			{
				recepteurPonctData.SetCol(idcol,*(cols[idfreq].GabeSlPerSrc[idrecp]));
				idcol++;
			}
		}
		recepteurPonctData.LockData();
		recepteurPonctData.Save((currentRP->pathRp+filename).c_str());
	}

	/////////////////////////////////////
	// Save one impulse response by source
	/////////////////////////////////////
	if(*params.configManager->FastGetConfigValue(Core_Configuration::I_PROP_OUTPUT_RECEIVER_BY_SOURCE) != 0) {
		int srcCount = params.configManager->srcList.size();
		//Instanciation du tableau des libellé des champs de fréquences
		std::vector<CoreString> reportFreqLbl;
		//Instanciation du tableau des libellé des pas de temps
		std::vector<CoreString> reportStepLbl;
		BaseReportManager::InitHeaderArrays(*params.configManager, reportFreqLbl, reportStepLbl);
		CoreString workingDir = *params.configManager->FastGetConfigValue(Core_Configuration::SPROP_CORE_WORKING_DIRECTORY);
		for(int idsource=0; idsource < srcCount; idsource++) {
			//Pour chaque récepteur ponctuel
			for(uentier idrecp=0;idrecp<params.configManager->recepteur_p_List.size();idrecp++)
			{
				CoreString rootRp=workingDir+CoreString(*params.configManager->FastGetConfigValue(Base_Core_Configuration::SPROP_PONCTUAL_RECEIVER_FOLDER_PATH)+"/");
				t_Recepteur_P* currentRP=params.configManager->recepteur_p_List[idrecp];
				t_Recepteur_P receiverData(nbfreqMax,params.nbTimeStep);
				// Create the content
				// Sound level by timestep and frequency
				for(uentier idfreq=0;idfreq<nbfreqMax;idfreq++)
				{
					if(params.configManager->freqList[idfreq]->doCalculation)
					{
						receiverData.InitFreq(idfreq);
						for(uentier idStep = 0; idStep < params.nbTimeStep; idStep ++) {
							double energy = cols[idfreq].SrcContrib[idrecp][idStep * srcCount + idsource];
							receiverData.energy_sum[idfreq][idStep] = energy;
						}
					}
				}
				receiverData.lblRp = currentRP->lblRp;
				receiverData.cdt_vol = currentRP->cdt_vol;
				CoreString file = rootRp + receiverData.lblRp + "/" + params.configManager->srcList[idsource]->sourceName + "/";
				st_mkdir(file.c_str());
				file += *params.configManager->FastGetConfigValue(Base_Core_Configuration::SPROP_PONCTUAL_RECEIVER_FILE_PATH);
				BaseReportManager::SauveRecepteurPonctuel(file, reportFreqLbl, reportStepLbl, &receiverData); 
			}
		}
	}
}


void ReportManager::SaveRecpIntensity(const CoreString& filename,std::vector<t_sppsThreadParam>& cols,const t_ParamReport& params)
{
	using namespace formatGABE;

	// Dans le format GABE on doit préciser le nombre de colonnes à la construction
	// et le nombre de colonne correspond au nombre de bande de fréquence*3+1
	//
	uentier nbfreqUsed=0;
	uentier nbfreqMax=params.configManager->freqList.size();
	for(std::size_t idfreq=0;idfreq<cols.size();idfreq++)
	{
		if(cols[idfreq].GabeColData)
			nbfreqUsed++;
	}

	////////////////////////////////////////////
	// Série Libellé des pas de temps
	////////////////////////////////////////////
	GABE_Data_ShortString collbl(params.nbTimeStep+1); //+1 pour le cumul
	GABE_Data_ShortString* statdBLbl=&collbl;
	for(uentier idstep=0;idstep<params.nbTimeStep;idstep++)
		statdBLbl->SetString(idstep,(CoreString::FromInt((int)(params.timeStep*(idstep+1)*1000))+" ms").c_str());
	statdBLbl->SetString(params.nbTimeStep,"Sum");
	statdBLbl->SetLabel("Intensity");

	for(uentier idrecp=0;idrecp<params.configManager->recepteur_p_List.size();idrecp++)
	{
		t_Recepteur_P* currentRP=params.configManager->recepteur_p_List[idrecp];
		GABE recepteurPonctData(nbfreqUsed*3+1);

		recepteurPonctData.SetCol(0,collbl);

		int idcol=1;
		for(std::size_t idfreq=0;idfreq<params.configManager->freqList.size();idfreq++)
		{
			if(params.configManager->freqList[idfreq]->doCalculation)
			{
				recepteurPonctData.SetCol(idcol,*(cols[idfreq].GabeIntensity[0][idrecp])); //x
				idcol++;
				recepteurPonctData.SetCol(idcol,*(cols[idfreq].GabeIntensity[1][idrecp])); //y
				idcol++;
				recepteurPonctData.SetCol(idcol,*(cols[idfreq].GabeIntensity[2][idrecp])); //z
				idcol++;
			}
		}
		recepteurPonctData.LockData();
		recepteurPonctData.Save((currentRP->pathRp+filename).c_str());
	}
	//////////////////////////////////////////////////
	// Enregistrement des animations
	std::vector<GABE_Object*> gabe_cols;
	GABE_Data_Integer serie_int_parameter(4);
	GABE_Data_Float serie_float_parameter(1);
	gabe_cols.push_back(&serie_int_parameter);
	gabe_cols.push_back(&serie_float_parameter);
	serie_int_parameter.Set(0,params.configManager->recepteur_p_List.size());	//Nombre de récepteur ponctuel
	serie_int_parameter.Set(1,params.nbTimeStep);								//Nombre de pas de temps
	serie_int_parameter.Set(2,3);												//Nombre de colonne définissant un récepteur ponctuel
	serie_int_parameter.Set(3,gabe_cols.size());						    	//Numéro de la colonne du premier récepteur ponctuel
	serie_float_parameter.Set(0,params.timeStep);								//Pas de temps (s)

	CoreString workpath=params.working_Path+"IntensityAnimation\\";
	//mkdir(workpath.c_str());
	st_mkdir(workpath.c_str());
	//Pour chaque bande de fréquence

	for(std::size_t idfreq=0;idfreq<cols.size();idfreq++)
	{
		if(params.configManager->freqList[idfreq]->doCalculation)
		{
			std::vector<GABE_Object*> freq_gabe_cols;
			CoreString freqdir=workpath+CoreString::FromInt(params.configManager->freqList[idfreq]->freqValue)+" Hz\\";
			//mkdir(freqdir.c_str());
			st_mkdir(freqdir.c_str());
			for(uentier idrecp=0;idrecp<params.configManager->recepteur_p_List.size();idrecp++)
			{
				t_Recepteur_P* currentRP=params.configManager->recepteur_p_List[idrecp];
				for(short dim=0;dim<3;dim++)
				{
					GABE_Data_Float* rp_data=cols[idfreq].GabeIntensity[dim][idrecp];
					unsigned int nbrecords=rp_data->GetSize();
					GABE_Data_Float* serie_pos=new GABE_Data_Float(nbrecords+1);
					freq_gabe_cols.push_back(serie_pos);
					//Position du récepteur
					serie_pos->Set(0,currentRP->position[dim]);
					for(unsigned int idstep=0;idstep<nbrecords;idstep++)
						serie_pos->Set(idstep+1,rp_data->GetValue(idstep));

				}
			}
			//Enregistrement du fichier
			std::size_t nbheaders_cols=gabe_cols.size();
			GABE recorder(nbheaders_cols+freq_gabe_cols.size());
			for(std::size_t idcol=0;idcol<nbheaders_cols;idcol++)
				recorder.SetCol(idcol,*gabe_cols[idcol]); //créé une copie
			for(std::size_t idcol=0;idcol<freq_gabe_cols.size();idcol++)
				recorder.SetCol(nbheaders_cols+idcol,freq_gabe_cols[idcol]);
			recorder.LockData();
			recorder.Save(CoreString(freqdir+"Intensity.rpi").c_str());
		}
	}

}
void ReportManager::SaveRecpAcousticParamsAdvance(const CoreString& filename,std::vector<t_sppsThreadParam>& cols,const t_ParamReport& params)
{
	using namespace formatGABE;
	uentier nbfreqUsed=0;
	uentier nbfreqMax=params.configManager->freqList.size();
	for(std::size_t idfreq=0;idfreq<cols.size();idfreq++)
	{
		if(cols[idfreq].GabeColData)
			nbfreqUsed++;
	}

	const uentier nbFixedCols=4;		//Colonne Index,Paramètres,Source,Fréquences
	const uentier nbRecpHeaderCols=1; //Colonne bruit
	const uentier nbColsByFreq=3;		//Colonne energie,LF,LFC
	const uentier nbCols=nbFixedCols+nbRecpHeaderCols+nbfreqUsed*nbColsByFreq; //Nombre de colonne dans chaque fichier gabe

	////////////////////////////////////////////
	// Série Index et paramètres entier [Col 0]
	////////////////////////////////////////////
	GABE_Data_Integer indexCol(8);
	indexCol.Set(0,1);								//Numéro de la colonne de la série des paramètres à décimal
	indexCol.Set(1,2);								//Numéro de la colonne de la série source
	indexCol.Set(2,3);								//Numéro de la colonne de la série valeurs de fréquences
	indexCol.Set(3,nbFixedCols);					//Numéro de la colonne du bruit
	indexCol.Set(4,nbFixedCols+nbRecpHeaderCols);	//Numéro de la colonne de la série de la première bande de fréquences
	indexCol.Set(5,nbfreqUsed);						//Nombre de bande de fréquences [1-27]
	indexCol.Set(6,nbColsByFreq);					//Nombre de colonne composant une bande de fréquence
	indexCol.Set(7,params.nbTimeStep);				//Nombre de pas de temps

	////////////////////////////////////////////
	// Série paramètres à décimal [Col 1]
	////////////////////////////////////////////
	GABE_Data_Float decimalParam(2);
	decimalParam.Set(0,params.timeStep);					//Pas de temps ms
	decimalParam.Set(1,params.nbTimeStep*params.timeStep);	//Durée

	////////////////////////////////////////////
	// Série source [Col 2]
	////////////////////////////////////////////v
	GABE_Data_Float sourceCol(nbfreqUsed);
	////////////////////////////////////////////
	// Série valeurs de fréquences [Col 3]
	////////////////////////////////////////////
	GABE_Data_Integer freqCol(nbfreqUsed);
	uentier idfreqligne=0;
	for(std::size_t idfreq=0;idfreq<params.configManager->freqList.size();idfreq++)
	{
		if(params.configManager->freqList[idfreq]->doCalculation)
		{
			//Pour cette bande de fréquence cumul de l'énérgie de toute les sources actives
			decimal cumulFreqSources=0;
			for(uentier idsource=0;idsource<params.configManager->srcList.size();idsource++)
			{
				cumulFreqSources+=params.configManager->srcList[idsource]->bandeFreqSource[idfreq].w_j;
			}
			sourceCol.Set(idfreqligne,cumulFreqSources*(*params.configManager->FastGetConfigValue(Core_Configuration::FPROP_RHO))*(*params.configManager->FastGetConfigValue(Core_Configuration::FPROP_CELERITE)));
			freqCol.Set(idfreqligne,(Intb)params.configManager->freqList[idfreq]->freqValue);
			idfreqligne++;
		}
	}
	for(uentier idrecp=0;idrecp<params.configManager->recepteur_p_List.size();idrecp++)
	{
		t_Recepteur_P* currentRP=params.configManager->recepteur_p_List[idrecp];
		GABE recepteurPonctData(nbCols);
		//Copie colonnes 0,1,2,3
		recepteurPonctData.SetCol(0,indexCol);
		recepteurPonctData.SetCol(1,decimalParam);
		recepteurPonctData.SetCol(2,sourceCol);
		recepteurPonctData.SetCol(3,freqCol);

		////////////////////////////////////////////
		// Série bruit  [Col nbFixedCols]
		////////////////////////////////////////////
		GABE_Data_Float* bruitCol=new GABE_Data_Float(nbfreqUsed);
		idfreqligne=0;
		for(std::size_t idfreq=0;idfreq<params.configManager->freqList.size();idfreq++)
		{
			if(params.configManager->freqList[idfreq]->doCalculation)
			{
				bruitCol->Set(idfreqligne,currentRP->bruit_spectre[idfreq].db);
				idfreqligne++;
			}
		}
		recepteurPonctData.SetCol(nbFixedCols,bruitCol);
		uentier idfreqcol=0;
		for(std::size_t idfreq=0;idfreq<cols.size();idfreq++)
		{
			if(currentRP->energy_sum[idfreq])
			{
				////////////////////////////////////////////
				// Série énérgie recu   [Col nbFixedCols+nbRecpHeaderCols+(idfreqcol*nbColsByFreq)]
				////////////////////////////////////////////
				GABE_Data_Float* e_col=new GABE_Data_Float(params.nbTimeStep);
				for(uentier idstep=0;idstep<params.nbTimeStep;idstep++)
				{
					e_col->Set(idstep,currentRP->energy_sum[idfreq][idstep]*currentRP->cdt_vol);
				}
				recepteurPonctData.SetCol(nbFixedCols+nbRecpHeaderCols+(idfreqcol*nbColsByFreq),e_col);
				////////////////////////////////////////////
				// Série énérgie recu LF  [Col nbFixedCols+nbRecpHeaderCols+(idfreqcol*nbColsByFreq)+1]
				////////////////////////////////////////////
				// Copie de la colonne
				recepteurPonctData.SetCol(nbFixedCols+nbRecpHeaderCols+(idfreqcol*nbColsByFreq)+1,*cols[idfreq].GabeSumEnergyCosPhi[idrecp]);
				////////////////////////////////////////////
				// Série énérgie recu LFC  [Col nbFixedCols+nbRecpHeaderCols+(idfreqcol*nbColsByFreq)+2]
				////////////////////////////////////////////
				// Copie de la colonne
				recepteurPonctData.SetCol(nbFixedCols+nbRecpHeaderCols+(idfreqcol*nbColsByFreq)+2,*cols[idfreq].GabeSumEnergyCosSqrtPhi[idrecp]);
				idfreqcol++;
			}
		}
		recepteurPonctData.Save((currentRP->pathRp+filename).c_str());
	}
}


void ReportManager::SaveReceiverIncidenceAngle(const CoreString& filename,std::vector<t_sppsThreadParam>& cols,const t_ParamReport& params)
{
	using namespace formatGABE;

	// Dans le format GABE on doit préciser le nombre de colonnes à la construction
	// et le nombre de colonne correspond au nombre de bande de fréquence*3+1
	//
	uentier nbfreqUsed=0;
	uentier nbfreqMax=params.configManager->freqList.size();
	for(std::size_t idfreq=0;idfreq<cols.size();idfreq++)
	{
		if(cols[idfreq].GabeColData)
			nbfreqUsed++;
	}

	////////////////////////////////////////////
	// Série Libellé des pas de temps
	////////////////////////////////////////////
	GABE_Data_ShortString collbl(params.nbTimeStep); //+1 pour le cumul
	GABE_Data_ShortString* statdBLbl=&collbl;
	for(uentier idstep=0;idstep<params.nbTimeStep;idstep++)
		statdBLbl->SetString(idstep,(CoreString::FromFloat((float)(params.timeStep*(idstep+1)*1000))+" ms").c_str());

	statdBLbl->SetLabel("Intensity + Angle of Incidence");

	for(uentier idrecp=0;idrecp<params.configManager->recepteur_p_List.size();idrecp++)
	{
		t_Recepteur_P* currentRP=params.configManager->recepteur_p_List[idrecp];
		GABE recepteurPonctData(nbfreqUsed*3+1);

		recepteurPonctData.SetCol(0,collbl);

		int idcol=1;
		for(std::size_t idfreq=0;idfreq<params.configManager->freqList.size();idfreq++)
		{
			if(params.configManager->freqList[idfreq]->doCalculation)
			{
				GABE_Data_Float* e_col=new GABE_Data_Float(params.nbTimeStep);
				e_col->SetLabel("SPL");
				for(uentier idstep=0;idstep<params.nbTimeStep;idstep++)
				{
					e_col->Set(idstep,10*log10((currentRP->energy_sum[idfreq][idstep]*currentRP->cdt_vol)*p_0));
				}

				recepteurPonctData.SetCol(idcol,*(e_col));
				idcol++;
				recepteurPonctData.SetCol(idcol,*(cols[idfreq].GabeAngleInc[0][idrecp])); //theta
				idcol++;
				recepteurPonctData.SetCol(idcol,*(cols[idfreq].GabeAngleInc[1][idrecp])); //phi
				idcol++;
			}
		}
		recepteurPonctData.LockData();
		recepteurPonctData.Save((currentRP->pathRp+filename).c_str());
	}
}

void ReportManager::ExportAsCSV(const CoreString& filename,std::vector<t_sppsThreadParam>& cols,const t_ParamReport& params)
{
	uentier nbfreqUsed=0;
	uentier nbfreqMax=params.configManager->freqList.size();
	for(std::size_t idfreq=0;idfreq<cols.size();idfreq++)
	{
		if(cols[idfreq].GabeColData)
			nbfreqUsed++;
	}

	for(uentier idrecp=0;idrecp<params.configManager->recepteur_p_List.size();idrecp++)
	{
		t_Recepteur_P* currentRP=params.configManager->recepteur_p_List[idrecp];

		ofstream resultsCSVFile;

		resultsCSVFile.open((currentRP->pathRp+filename).c_str());
		resultsCSVFile<<"freq,";
		for(uentier idstep=0;idstep<params.nbTimeStep;idstep++)
			resultsCSVFile<<(CoreString::FromFloat((float)(params.timeStep*(idstep+1)*1000))+" ms").c_str()<<",";
		resultsCSVFile<<"\n";
		for(std::size_t idfreq=0;idfreq<params.configManager->freqList.size();idfreq++)
		{
			if(params.configManager->freqList[idfreq]->doCalculation)
			{	
				resultsCSVFile<<params.configManager->freqList[idfreq]->freqValue<<"Hz,";
				for(uentier idstep=0;idstep<params.nbTimeStep;idstep++)
				{
					resultsCSVFile<<10*log10((currentRP->energy_sum[idfreq][idstep]*currentRP->cdt_vol)*p_0)<<",";
				}
				resultsCSVFile<<"\n";

				if (*(params.configManager->FastGetConfigValue(Core_Configuration::IPROP_EXPORT_RECEIVER_INCIDENCE_ANGLE))) {
					resultsCSVFile << params.configManager->freqList[idfreq]->freqValue << "Hz theta,";
					for (uentier idstep = 0; idstep < params.nbTimeStep; idstep++)
					{
						resultsCSVFile << cols[idfreq].GabeAngleInc[0][idrecp]->GetValue(idstep) << ",";
					}
					resultsCSVFile << "\n";
					resultsCSVFile << params.configManager->freqList[idfreq]->freqValue << "Hz phi,";
					for (uentier idstep = 0; idstep < params.nbTimeStep; idstep++)
					{
						resultsCSVFile << cols[idfreq].GabeAngleInc[1][idrecp]->GetValue(idstep) << ",";
					}
				}
				resultsCSVFile<<"\n";
			}
		}

		resultsCSVFile<<"\n";
		resultsCSVFile.close();
	}
}
