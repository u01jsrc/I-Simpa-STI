# -*- coding: cp1252 -*-
import math
import uictrl as ui
from libsimpa import *


def GetMixedLevel(folderwxid,param,enable):
    """
     Retourne un tableau contenant le niveau sonore global et toute bande des récepteurs ponctuels d'un dossier
     folderwxid identifiant wxid de l'élément dossier contenant les récepteurs ponctuels.
    """
    #cols=[]
    cols2=[]
    cols3=[]
    licznik=[]
    avr=[]
    global_Std=[]
    avr_std=[]
    #folder devient l'objet dossier
    folder=ui.element(folderwxid)
    #dans un tableau on place les indices des fichiers de données des récepteurs ponctuels
    recplist=folder.getallelementbytype(ui.element_type.ELEMENT_TYPE_REPORT_GABE_RECP)
    #Pour chaque récepteur on demande à l'application les données traitées du fichier (niveau sonore et cumuls)
    for idrecp in recplist:
        #recp devient l'objet ayant comme indice idrecp (entier)
        recp=ui.element(idrecp)
        if recp.getinfos()["name"]=="soundpressure":
            #on demande le calcul des paramètres sonores
            if enable==1: 
				ui.application.sendevent(recp,ui.idevent.IDEVENT_RECP_COMPUTE_ACOUSTIC_PARAMETERS,{"TR":"20;30", "EDT":"", "D":"50"})
            #on recupere l'element parent (le dossier de récepteur ponctuel)
            pere=ui.element(recp.getinfos()["parentid"])
            #application.sendevent(pere,idevent.IDEVENT_RELOAD_FOLDER)
            nomrecp=pere.getinfos()["label"]
            #on recupere les données calculées
            params=ui.element(pere.getelementbylibelle('acoustic_param'))
            #on stocke dans gridspl le tableau des niveaux de pression
            gridparam=ui.application.getdataarray(params)
            #on ajoute la colonne
            #if len(cols)==0: #si le tableau de sortie est vide alors on ajoute les libellés des lignes
            #    cols.append(list(zip(*gridparam)[0])) #libellé Freq et Global
            if len(cols2)==0: #si le tableau de sortie est vide alors on ajoute les libellés des lignes
                cols2.append(list(zip(*gridparam)[0])) #libellé Freq et Global
            if len(cols3)==0: #si le tableau de sortie est vide alors on ajoute les libellés des lignes
                cols3.append(list(zip(*gridparam)[0])) #libellé Freq et Global
            idcol1=gridparam[0].index(param) #Changer le paramètre par celui pour lequel on veut la fusion
            #cols.append([nomrecp]+list(zip(*gridparam)[idcol1][1:])) #1ere colonne, (0 etant le libellé des lignes) et [1:] pour sauter la premiere ligne						
			
            if len(avr)==0: #si le tableau de sortie est vide alors on ajoute les libellés des lignes
				avr=(['global']+list(zip(*gridparam)[idcol1][1:])) #libellé Freq et Global
            else:
				avr[1:]=[x + y for x, y in zip(avr[1:], list(zip(*gridparam)[idcol1][1:]))]
			
            if len(global_Std)==0: #si le tableau de sortie est vide alors on ajoute les libellés des lignes
				global_Std=(['global']+[x*x for x in list(zip(*gridparam)[idcol1][1:])]) #libellé Freq et Global
            else:
				global_Std[1:]=[x + y for x, y in zip(global_Std[1:],[z*z  for z in list(zip(*gridparam)[idcol1][1:])])]
			
            try:
				nomrecpint=int(nomrecp)
            except ValueError:
				nomrecpint=int(nomrecp[9:])
			
            if len(cols2)>nomrecpint:
				cols2[nomrecpint][1:]=[x + y for x, y in zip(cols2[nomrecpint][1:], list(zip(*gridparam)[idcol1][1:]))]		
				cols3[nomrecpint][1:]=[x + y for x, y in zip(cols3[nomrecpint][1:],[z*z  for z in list(zip(*gridparam)[idcol1][1:])])]
				licznik[nomrecpint-1]=licznik[nomrecpint-1]+1;
            else:
				cols2.append([nomrecp]+list(zip(*gridparam)[idcol1][1:]))
				cols3.append([nomrecp]+[x*x for x in list(zip(*gridparam)[idcol1][1:])])
				licznik.append(1)

		
	
    avr_std=(['average']+[0 for x in avr[1:]])
	
    for wek1, wek2 in zip(cols2[1:], cols3[1:]):
        wek1[1:] = [x / y  for x,y in zip(wek1[1:],licznik)]
        wek2[1:] = [math.sqrt(math.fabs(x1/n-x2*x2)) for n,x1,x2 in zip(licznik,wek2[1:],wek1[1:])]
        avr_std[1:]= [x+y for x,y in zip(wek2[1:],avr_std[1:])]
	
    suma=sum(licznik)
    avr[1:]=[x / suma for x in avr[1:]]
	
    avr_std[1:]= [x/len(licznik) for x in avr_std[1:]]
    cols2.append(avr)
	
    global_Std[1:]=[math.sqrt(math.fabs(x/suma - x2*x2)) for x,x2 in zip(global_Std[1:],avr[1:])]
    cols3.append(global_Std)
    cols3.append(avr_std)
	
	
    return [cols2,cols3]

def SaveLevel(tab,path):
    #Creation de l'objet qui lit et ecrit les fichiers gabe
    gabewriter=Gabe_rw(len(tab))
    labelcol=stringarray()
    for cell in tab[0][1:]:
        labelcol.append(cell.encode('cp1252'))
    gabewriter.AppendStrCol(labelcol,"RT-20")
    for col in tab[1:]:
        datacol=floatarray()
        for cell in col[1:]:
            datacol.append(float(cell))
        gabewriter.AppendFloatCol(datacol,str(col[0]))
    gabewriter.Save(path.encode('cp1252'))
    
def dofusion(folderwxid, path,param,enable):
    arraydata=GetMixedLevel(folderwxid,param,enable)
    SaveLevel(zip(*arraydata[0]),path+param+".gabe")
    SaveLevel(zip(*arraydata[1]),path+param+"_std.gabe")
    #raffraichie l'arbre complet
    ui.application.sendevent(ui.element(ui.element(ui.application.getrootreport()).childs()[0][0]),ui.idevent.IDEVENT_RELOAD_FOLDER)

class manager:
    def __init__(self):
        self.GetMixedLevelid_all=ui.application.register_event(self.OnFusion_all)
        self.GetMixedLevelid_t20=ui.application.register_event(self.OnFusion_t20)
        self.GetMixedLevelid_t30=ui.application.register_event(self.OnFusion_t30)
        self.GetMixedLevelid_edt=ui.application.register_event(self.OnFusion_edt)
        self.GetMixedLevelid_c50=ui.application.register_event(self.OnFusion_c50)
        self.GetMixedLevelid_c80=ui.application.register_event(self.OnFusion_c80)
        self.GetMixedLevelid_d50=ui.application.register_event(self.OnFusion_d50)
    def getmenu(self,typeel,idel,menu):
        el=ui.element(idel)
        infos=el.getinfos()
        if infos["name"]==u"SPPS":
			submenu=[]
			submenu.append(((u"Calculate All"),self.GetMixedLevelid_all))
			submenu.append(())
			submenu.append(((u"Calculate T20"),self.GetMixedLevelid_t20))
			submenu.append(((u"Calculate T30"),self.GetMixedLevelid_t30))
			submenu.append(((u"Calculate EDT"),self.GetMixedLevelid_edt))
			submenu.append(())
			submenu.append(((u"Calculate C50"),self.GetMixedLevelid_c50))
			submenu.append(((u"Calculate C80"),self.GetMixedLevelid_c80))
			submenu.append(((u"Calculate D50"),self.GetMixedLevelid_d50))
			menu.insert(0,())
			menu.insert(0,(u"Combine receivers",submenu))
			return True
        else:
            return False
    def OnFusion_all(self,idel):
        grp=ui.e_file(idel)
        dofusion(idel,grp.buildfullpath()+"fusion_","RT-20 (s)",1)
        dofusion(idel,grp.buildfullpath()+"fusion_","RT-30 (s)",0)
        dofusion(idel,grp.buildfullpath()+"fusion_","EDT (s)",0)
        dofusion(idel,grp.buildfullpath()+"fusion_","C-50 (dB)",0)
        dofusion(idel,grp.buildfullpath()+"fusion_","C-80 (dB)",0)
        dofusion(idel,grp.buildfullpath()+"fusion_","D-50 (%)",0)
    def OnFusion_t20(self,idel):
        grp=ui.e_file(idel)
        dofusion(idel,grp.buildfullpath()+"fusion_","RT-20 (s)",1)
    def OnFusion_t30(self,idel):
        grp=ui.e_file(idel)
        dofusion(idel,grp.buildfullpath()+"fusion_","RT-30 (s)",1)
    def OnFusion_edt(self,idel):
        grp=ui.e_file(idel)
        dofusion(idel,grp.buildfullpath()+"fusion_","EDT (s)",1)
    def OnFusion_c50(self,idel):
        grp=ui.e_file(idel)
        dofusion(idel,grp.buildfullpath()+"fusion_","C-50 (dB)",1)
    def OnFusion_c80(self,idel):
        grp=ui.e_file(idel)
        dofusion(idel,grp.buildfullpath()+"fusion_","C-80 (dB)",1)
    def OnFusion_d50(self,idel):
        grp=ui.e_file(idel)
        dofusion(idel,grp.buildfullpath()+"fusion_","D-50 (%)",1)
		
ui.application.register_menu_manager(ui.element_type.ELEMENT_TYPE_REPORT_FOLDER, manager())
