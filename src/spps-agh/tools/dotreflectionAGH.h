//#include "coreTypes.h"
#include "tools/dotreflection.h"


/**
 * @brief Cette classe regroupe les méthodes de réfléxion de particules sur les surfaces
 *
 * Les vecteurs de réflexion retournés sont normalisés car une perte de précision se fait ressentir quand à la norme de ces vecteurs
 */
class ReflectionLawsAGH : public  ReflectionLaws
{
public:
	/**
	 * En fonction du matériau appel la méthode de réflexion associé. Si la méthode est inconnue la réflexion est spéculaire.
	 * @param vectorDirection Vecteur incident
	 * @param materialInfo Matériau de la face
	 * @param faceInfo Informations de la face en collision
	 * @param AB Vecteur perpendiculaire à la normal de la face
	 * @return Vecteur réflexion normalisé
	 */
	static vec3 SolveDiffusePart(vec3 &vectorDirection,t_Material_BFreq &materialInfo,vec3& faceNormal)
	{
		switch(materialInfo.reflectionLaw)
		{
			case REFLECTION_LAW_SPECULAR:
				return SpecularReflection(vectorDirection,faceNormal);
			case REFLECTION_LAW_LAMBERT:
				return BaseWnReflection(vectorDirection,faceNormal,1);
			case REFLECTION_LAW_UNIFORM:
				return BaseWnReflection(vectorDirection,faceNormal,0);
			case REFLECTION_LAW_W2:
				return BaseWnReflection(vectorDirection,faceNormal,2);
			case REFLECTION_LAW_W3:
				return BaseWnReflection(vectorDirection,faceNormal,3);
			case REFLECTION_LAW_W4:
				return BaseWnReflection(vectorDirection,faceNormal,4);
			case REFLECTION_LAW_PHONG:
				return BaseWnReflection(vectorDirection, faceNormal, 1);
			default:
				return SpecularReflection(vectorDirection,faceNormal);
		};
	}

	static vec3 SolveSpecularPart(vec3 &vectorDirection, t_Material_BFreq &materialInfo, vec3& faceNormal)
	{
		switch (materialInfo.reflectionLaw)
		{
		case REFLECTION_LAW_PHONG:
			return PhongSpecularPart(vectorDirection, faceNormal, materialInfo);
		default:
			return SpecularReflection(vectorDirection, faceNormal);
		};
	}


private:

	static vec3 PhongSpecularPart(vec3 &vectorDirection, vec3 &faceNormal, t_Material_BFreq &material)
	{
		float n = powf(10, -1.7234170470604733*material.diffusion + 2.6245274102195886);
		vec3 specularDir = SpecularReflection(vectorDirection, faceNormal);

		Matrix3 rotationMatrix;
		rotationMatrix.calculateRotationMatrix(faceNormal, specularDir);

		vec3 target = BaseWnReflection(vectorDirection, faceNormal, n);
		target = rotationMatrix*target;
		target.normalize();

		double test = target.dot(faceNormal);

		//test if reflected ray is pointing into ground
		if(target.dot(faceNormal) <= 0)
		{
			//if it is get new random direction
			target = PhongSpecularPart(vectorDirection, faceNormal, material);
		}

		return target;
	}
};


/**
* @brief Cette classe regroupe les méthodes de réfléxion de particules sur les surfaces
*
* Les vecteurs de réflexion retournés sont normalisés car une perte de précision se fait ressentir quand à la norme de ces vecteurs
*/
class ReflectionLawsMLT : public  ReflectionLaws
{
public:
	/**
	* En fonction du matériau appel la méthode de réflexion associé. Si la méthode est inconnue la réflexion est spéculaire.
	* @param vectorDirection Vecteur incident
	* @param materialInfo Matériau de la face
	* @param faceInfo Informations de la face en collision
	* @param AB Vecteur perpendiculaire à la normal de la face
	* @return Vecteur réflexion normalisé
	*/
	static vec3 SolveDiffusePart(vec3 &vectorDirection, t_Material_BFreq &materialInfo, vec3& faceNormal, CONF_PARTICULE_AGH& particuleInfo,double &rnd1, double &rnd2, double &probability)
	{
		switch (materialInfo.reflectionLaw)
		{
		case REFLECTION_LAW_SPECULAR:
			probability = (1 - materialInfo.diffusion);
			return SpecularReflection(vectorDirection, faceNormal);
		case REFLECTION_LAW_LAMBERT:
			return BaseWnReflection(vectorDirection, faceNormal, 1, rnd1, rnd2, probability);
		case REFLECTION_LAW_UNIFORM:
			return BaseWnReflection(vectorDirection, faceNormal, 0, rnd1, rnd2, probability);
		case REFLECTION_LAW_W2:
			return BaseWnReflection(vectorDirection, faceNormal, 2, rnd1, rnd2, probability);
		case REFLECTION_LAW_W3:
			return BaseWnReflection(vectorDirection, faceNormal, 3, rnd1, rnd2, probability);
		case REFLECTION_LAW_W4:
			return BaseWnReflection(vectorDirection, faceNormal, 4, rnd1, rnd2, probability);
		case REFLECTION_LAW_PHONG:
			return BaseWnReflection(vectorDirection, faceNormal, 1, rnd1, rnd2, probability);
		default:
			return SpecularReflection(vectorDirection, faceNormal);
		};
	}

	static vec3 SolveSpecularPart(vec3 &vectorDirection, t_Material_BFreq &materialInfo, vec3& faceNormal, CONF_PARTICULE_AGH& particuleInfo, double &rnd1, double &rnd2, double &probability)
	{
		switch (materialInfo.reflectionLaw)
		{
		case REFLECTION_LAW_PHONG:
			return PhongSpecularPart(vectorDirection, faceNormal, particuleInfo, materialInfo, rnd1, rnd2, probability);
		default:
			probability = (1 - materialInfo.diffusion);
			return SpecularReflection(vectorDirection, faceNormal);
		};
	}


private:

	static vec3 PhongSpecularPart(vec3 &vectorDirection, vec3 &faceNormal, CONF_PARTICULE_AGH& particuleInfo, t_Material_BFreq &material, double &rnd1, double &rnd2, double &probability)
	{
		float n = powf(10, powf(-0.82662*material.diffusion, 3) + 1.5228);
		vec3 specularDir = SpecularReflection(vectorDirection, faceNormal);

		Matrix3 rotationMatrix;
		rotationMatrix.calculateRotationMatrix(faceNormal, specularDir);

		vec3 target = BaseWnReflection(vectorDirection, faceNormal, n, rnd1, rnd2, probability);
		target = rotationMatrix*target;

		//test if reflected ray is pointing into ground
		if (target.dot(faceNormal) <= 0)
		{
			//if it is get new random direction
			target = PhongSpecularPart(vectorDirection, faceNormal, particuleInfo, material, rnd1, rnd2, probability);
		}

		return target / target.length();
	}


	static vec3 BaseWnReflection(vec3 &vecteurVitesse, vec3 &faceNormal, decimal expo, double &rnd1, double &rnd2, double &probability)
	{
		decimal theta = rnd1 * M_2PI;
		decimal phi = acos(pow((float)1 - rnd2, (float)(1. / (expo + 1.))));//pow((float)acos(1-GetRandValue()),(float)(1./(expo+1.)));

		probability = ((expo + 2) / (2 * M_PI)) * powf(cos(phi),expo);

		return BaseUniformReflection(vecteurVitesse, faceNormal, theta, phi);
	}


	static vec3 SpecularReflection(vec3 &vecteurVitesse, vec3 &faceNormal)
	{
		vec3 retVal = (vecteurVitesse - (faceNormal*(vecteurVitesse.dot(faceNormal)) * 2));
		return retVal / retVal.length();
	}
};